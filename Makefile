CC=gcc 
CFLAGS=-std=c99 -Wall -Werror
unpak: unpak.o
bsp2json: bsp2json.o

clean:
	rm -f unpak unpak.o bsp2json bsp2json.o

