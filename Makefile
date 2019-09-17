all:
	mkdir -p bin/
	gcc -o bin/silica_display src/c/osc.c src/c/osc_handlers.c src/c/silica_display.c src/c/silica_displaylib.c src/c/silica_displaylib_handlers.c src/c/rtqueue.c -lpthread -llo;
