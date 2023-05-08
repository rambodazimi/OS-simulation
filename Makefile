mysh: shell.c interpreter.c shellmemory.c
	gcc -D FRAME_STORE_SIZE=$(framesize) -D VAR_STORE_SIZE=$(varmemsize) -c shell.c interpreter.c shellmemory.c
	gcc -o mysh shell.o interpreter.o shellmemory.o

clean: 
	rm mysh; rm *.o
