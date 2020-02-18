all: main

main:
	gcc -Wall main.c `pkg-config fuse3 --cflags --libs` -o main