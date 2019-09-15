#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "silica_displaylib.h"
#include "silica_displaylib_handlers.h"

int main(void) {
  silica_display_setup("/dev/ttyACM0");

  sleep(1);

  silica_display_handler_send_linux_started();

  sleep(1);
  
  silica_display_handler_send_jackd_started();

  sleep(1);

  silica_display_handler_send_cyperus_started();

  while(1) {} ;
  
}
