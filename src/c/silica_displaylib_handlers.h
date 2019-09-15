

#ifndef silica_displaylib_handlers_h__
#define silica_displaylib_handlers_h__

#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "rtqueue.h"

void silica_display_handler_send_linux_started();
void silica_display_handler_send_jackd_started();
void silica_display_handler_send_cyperus_started();

#endif
