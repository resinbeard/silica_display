all:
	gcc -o silica_display silica_display.c silica_displaylib.c silica_displaylib_handlers.c rtqueue.c -lpthread
