#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "rtqueue.h"
#include "silica_displaylib.h"

void silica_display_handler_send_linux_started() {
  unsigned int id = silica_display_get_id();
  silica_display_send_message(id, 0, "        ");
} /* silica_display_handler_send_linux_started */

void silica_display_handler_send_jackd_started() {
  unsigned int id = silica_display_get_id();
  silica_display_send_message(id, (unsigned int)1, "        ");
} /* silica_display_handler_send_jackd_started */

