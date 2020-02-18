CC=gcc  #compiler
TARGET=main #target file name

all: main

main: clean
	gcc -Wall main.c `pkg-config fuse3 --cflags --libs` -o main

clean:
	rm -f *.o main