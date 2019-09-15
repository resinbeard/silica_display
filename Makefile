all:
	gcc -o silica_display src/c/silica_display.c src/c/silica_displaylib.c src/c/silica_displaylib_handlers.c src/c/rtqueue.c -lpthread
