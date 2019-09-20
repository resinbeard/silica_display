#ifndef silica_displaylib_h__
#define silica_displaylib_h__

#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "rtqueue.h"
#include "silica_displaylib_handlers.h"

union {
  unsigned char i;
  char c[2];
} binary_integer;

typedef struct silica_display_message_response {
  unsigned char *id;
  rtqueue_t *response_queue;
  struct silica_display_message_response *next;
  struct silica_display_message_response *prev;
} silica_display_message_response_t;

unsigned char *silica_display_send_message(unsigned char id, unsigned char command, unsigned char *data, size_t size);
unsigned int silica_display_get_id();
void silica_display_thread(void *arg);
int silica_display_setup(char *portname);
int silica_display_teardown();

#endif
