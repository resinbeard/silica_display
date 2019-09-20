#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "rtqueue.h"
#include "silica_displaylib.h"

void silica_display_handler_send_startup_system_process(char *procname, int status) {
  int padding = 81 - strlen(procname) - 4;
  char *procline = malloc(sizeof(char) * 250);
  unsigned int id = silica_display_get_id();
  int i, cmd_id;
  
  for(i=0; i<80; i++)
    procline[i] = ' ';
  for(i=0; i<strlen(procname); i++)
    procline[i] = procname[i];
  if(status) {
    procline[76] = 'B';
    procline[77] = 'A';
    procline[78] = 'D';
  } else {
    procline[77] = 'O';
    procline[78] = 'K';
  }

  cmd_id = 0;

  printf("procline: %s\n", procline);
  silica_display_send_message(id, (unsigned int)cmd_id, procline, 80);

  free(procline);
} /* silica_display_handler_startup_system_process */

void silica_display_handler_send_blank_screen() {
  unsigned int id = silica_display_get_id();
  int cmd_id = 1;

  printf("sending message\n");
  silica_display_send_message(id, (unsigned int)cmd_id, "   ", 3);
} /* silica_display_handler_send_blank_screen */

void silica_display_handler_send_status_cpu(float total, float cpu0, float cpu1, float cpu2, float cpu3) {
  char *output = malloc(sizeof(char) * 250);
  unsigned int id = silica_display_get_id();
  int i, cmd_id;

  unsigned long tempval = *(unsigned long *)&total;
  output[0] = tempval & 0x00FF;
  output[1] = (tempval & 0xFF00) >> 8;
  output[2] = (tempval & 0xFF0000) >> 16;
  output[3] = (tempval & 0xFF000000) >> 24;

  tempval = *(unsigned long *)&cpu0;
  output[4] = tempval & 0x00FF;
  output[5] = (tempval & 0xFF00) >> 8;
  output[6] = (tempval & 0xFF0000) >> 16;
  output[7] = (tempval & 0xFF000000) >> 24;

  tempval = *(unsigned long *)&cpu1;
  output[8] = tempval & 0x00FF;
  output[9] = (tempval & 0xFF00) >> 8;
  output[10] = (tempval & 0xFF0000) >> 16;
  output[11] = (tempval & 0xFF000000) >> 24;

  tempval = *(unsigned long *)&cpu2;
  output[12] = tempval & 0x00FF;
  output[13] = (tempval & 0xFF00) >> 8;
  output[14] = (tempval & 0xFF0000) >> 16;
  output[15] = (tempval & 0xFF000000) >> 24;

  tempval = *(unsigned long *)&cpu3;
  output[16] = tempval & 0x00FF;
  output[17] = (tempval & 0xFF00) >> 8;
  output[18] = (tempval & 0xFF0000) >> 16;
  output[19] = (tempval & 0xFF000000) >> 24;
  
  cmd_id = 2;

  silica_display_send_message(id, (unsigned int)cmd_id, output, 20);

  free(output);
} /* silica_display_handler_send_status_cpu */

void silica_display_handler_send_status_mem(float mem, float total) {
  char *output = malloc(sizeof(char) * 250);
  unsigned int id = silica_display_get_id();
  int i, cmd_id;

  unsigned long tempval = *(unsigned long *)&total;
  output[0] = tempval & 0x00FF;
  output[1] = (tempval & 0xFF00) >> 8;
  output[2] = (tempval & 0xFF0000) >> 16;
  output[3] = (tempval & 0xFF000000) >> 24;

  tempval = *(unsigned long *)&mem;
  output[4] = tempval & 0x00FF;
  output[5] = (tempval & 0xFF00) >> 8;
  output[6] = (tempval & 0xFF0000) >> 16;
  output[7] = (tempval & 0xFF000000) >> 24;

  printf("mem: %f\n", mem);
  printf("totla: %f\n", total);
  
  cmd_id = 3;

  silica_display_send_message(id, (unsigned int)cmd_id, output, 8);

  free(output);
} /* silica_display_handler_send_status_mem */


void silica_display_handler_send_status_disk(float disk, float total) {
  char *output = malloc(sizeof(char) * 250);
  unsigned int id = silica_display_get_id();
  int i, cmd_id;

  unsigned long tempval = *(unsigned long *)&total;
  output[0] = tempval & 0x00FF;
  output[1] = (tempval & 0xFF00) >> 8;
  output[2] = (tempval & 0xFF0000) >> 16;
  output[3] = (tempval & 0xFF000000) >> 24;

  tempval = *(unsigned long *)&disk;
  output[4] = tempval & 0x00FF;
  output[5] = (tempval & 0xFF00) >> 8;
  output[6] = (tempval & 0xFF0000) >> 16;
  output[7] = (tempval & 0xFF000000) >> 24;

  printf("disk: %f\n", disk);
  printf("totla: %f\n", total);
  
  cmd_id = 4;

  silica_display_send_message(id, (unsigned int)cmd_id, output, 8);

  free(output);
} /* silica_display_handler_send_status_disk */

void silica_display_handler_send_status_network(int ip0, int ip1, int ip2, int ip3) {
  char *output = malloc(sizeof(char) * 250);
  unsigned int id = silica_display_get_id();
  int i, cmd_id;

  output[0] = ip0;
  output[1] = ip1;
  output[2] = ip2;
  output[3] = ip3;

  cmd_id = 5;

  silica_display_send_message(id, (unsigned int)cmd_id, output, 4);

  free(output);
} /* silica_display_handler_send_status_network */
