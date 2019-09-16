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
  char *procline = malloc(sizeof(char) * 252);
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
  silica_display_send_message(id, (unsigned int)cmd_id, procline);

  free(procline);
} /* silica_display_handler_startup_system_process */
