#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "osc.h"
#include "silica_displaylib.h"
#include "silica_displaylib_handlers.h"

int main(void) {
  silica_display_setup("/dev/ttyACM0");

  osc_setup("9456", "9458");
  while(1) {sleep(1);} ;
  
}
