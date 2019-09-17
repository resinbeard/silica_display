

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
void silica_display_handler_send_blank_screen();
void silica_display_handler_send_status_cpu(float total, float cpu0, float cpu1, float cpu2, float cpu3);
void silica_display_handler_send_status_mem(float mem, float total);
void silica_display_handler_send_status_disk(float disk, float total);
#endif
