#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "rtqueue.h"
#include "silica_displaylib.h"
#include "silica_displaylib_handlers.h"

char *global_portname;
rtqueue_t *global_display_queue_out;
silica_display_message_response_t *global_message_response_list;
int global_exit;

int _set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // ignore break signal
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf ("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
} /* _set_interface_attribs */

void _set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            	// 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                printf ("error %d setting term attributes", errno);
} /* _set_blocking */

unsigned int silica_display_get_id() {
  return (unsigned char)rand();
} /* silica_display_get_id */

silica_display_message_response_t *silica_display_message_response_init(unsigned char id) {
  silica_display_message_response_t *message_response = malloc(sizeof(silica_display_message_response_t));
  message_response->id = id;
  message_response->response_queue = rtqueue_init(1);
  message_response->next = NULL;
  message_response->prev = NULL;
} /* silica_display_message_response_init */

unsigned char *silica_display_send_message(unsigned char id, unsigned char command, unsigned char *data, size_t size) {
  unsigned char message[256];
  unsigned char *message_in = NULL;
  silica_display_message_response_t *message_response = NULL;
  silica_display_message_response_t *temp_message_response = NULL;
  
  message[0] = 0x12;

  printf("0x12: %d, ", 0x12);
  printf("message[0]: %d\n", message[0]);
  
  message[1] = (unsigned)id & 0xff;
  message[2] = (unsigned)id >> 8;

  printf("sending id: %d\n", id);
  
  message[3] = (unsigned)command & 0xff;
  message[4] = (unsigned)command >> 8;

  memcpy(message+5, data, size);
  
  message[5+size+2] = 0x13;;
  
  /* should check that data is less than
     our maximum data size */

  printf("------------------------- sending id %d\n", id);
  
  message_response = silica_display_message_response_init(id);

  temp_message_response = global_message_response_list;
  if(temp_message_response == NULL)
    global_message_response_list = message_response;
  else {
    while(temp_message_response->next != NULL)
      temp_message_response = temp_message_response->next;
    temp_message_response->next = message_response;
  }

  printf("sizeof(message): %d\n", sizeof(message));
  rtqueue_enq(global_display_queue_out, message);
  printf("waiting\n");
  message_in = rtqueue_deq(message_response->response_queue);
  printf("got something\n");

  temp_message_response = global_message_response_list;
  while(temp_message_response == NULL) {
    if(temp_message_response->id == id) {
      temp_message_response->next->prev = temp_message_response->prev;
      temp_message_response->prev->next = temp_message_response->next;
      free(temp_message_response->response_queue);
      free(temp_message_response);
      break;
    }
    temp_message_response = temp_message_response->next;
  }
  printf("waited\n");
  return message_in;
} /* silica_display_send_message */

void silica_display_thread(void *arg) {
  global_exit = 0;
  global_display_queue_out = rtqueue_init(8192);

  int fd = open (global_portname, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0)
    {
      printf("error %d opening %s: %s", errno, global_portname, strerror (errno));
      return;
    }
  
  _set_interface_attribs (fd, B115200, 0); // set speed to 115,200 bps, 8n1 (no parity)
  _set_blocking (fd, 0);		   // set no blocking
  
  unsigned char *temp_message = NULL;
  unsigned char *msg_buffer = NULL;
  int n. o;
  unsigned int response_id = 0;
  silica_display_message_response_t *temp_message_response = NULL;
  
  while(global_exit == 0) {
    if( rtqueue_isempty(global_display_queue_out) == 0) {;
      temp_message = rtqueue_deq(global_display_queue_out);
      printf("deq'd\n");

      printf("sizeof(temp_message): %d\n", sizeof(temp_message));
      
      write(fd, temp_message, 29);
      printf("written\n");
    }

    temp_message = malloc(sizeof(unsigned char) * 256);
    n = read(fd, temp_message, sizeof(unsigned char) * 256);
    if (n > 0) {

      printf("reading %d bytes\n", n);

      response_id = temp_message[2];
      response_id = response_id << 8;
      response_id |= temp_message[1];

      printf("------------------------- response_id: %d\n", response_id);
      printf("temp_message[0]: %d\n", temp_message[0]);
      if(temp_message[0] == 0x12)
	printf("OMFG OMFG OMFG OMFG\n");
      
      temp_message_response = global_message_response_list;
      while(temp_message_response != NULL) {
	if(response_id == temp_message_response->id)
	  rtqueue_enq(temp_message_response->response_queue, temp_message);
	temp_message_response = temp_message_response->next;
      }
    }
    usleep(10);
  }
} /* silica_display_thread */

int silica_display_setup(char *portname) {
  srand(time(NULL));
  global_message_response_list = NULL;
  global_portname = portname;
  pthread_t display_thread_id;
  pthread_create(&display_thread_id, NULL, silica_display_thread, NULL);
  pthread_detach(display_thread_id);
} /* silica_display_setup */

int silica_display_teardown(char *portname) {
  global_exit = 1;
  free(global_display_queue_out);
} /* silica_display_teardown */
