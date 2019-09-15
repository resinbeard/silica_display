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

unsigned char *silica_display_send_message(unsigned char id, unsigned char command, unsigned char *data) {
  unsigned char message[12];
  unsigned char *message_in = NULL;
  silica_display_message_response_t *message_response = NULL;
  silica_display_message_response_t *temp_message_response = NULL;
  
  binary_integer.i = id;
  message[0] = (unsigned)id & 0xff;
  message[1] = (unsigned)id >> 8;
  
  binary_integer.i = command;
  message[2] = (unsigned)command & 0xff;
  message[3] = (unsigned)command >> 8;
  
  /* should check that data is less than
     our maximum data size */

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
  int n;
  int response_id = 0;
  silica_display_message_response_t *temp_message_response = NULL;
  
  while(global_exit == 0) {
    if( rtqueue_isempty(global_display_queue_out) == 0) {;
      temp_message = rtqueue_deq(global_display_queue_out);
      printf("deq'd\n");

      printf("sizeof(temp_message): %d\n", sizeof(temp_message));
      for(n=0; n<12; n++)
	printf("temp_message[%d]: %c\n", n, temp_message[n]);
      
      write(fd, temp_message, 12);
      printf("written\n");
    }

    temp_message = malloc(sizeof(unsigned char) * 12);
    n = read(fd, temp_message, sizeof(unsigned char) * 12);
    if (n > 0) {
      printf("reading\n");
      int i;
      for (i = 0; i < 12; i++) {
	printf("%c",temp_message[i]);
      }

      binary_integer.c[0] = temp_message[0];
      binary_integer.c[1] = temp_message[1];
      response_id = binary_integer.i;

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
