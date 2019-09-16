

#ifndef silica_displaylib_handlers_h__
#define silica_displaylib_handlers_h__

#include <termios.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "rtqueue.h"

void silica_display_handler_send_startup_system_process(char *procname, int status);

#endif
