all: main

main:
	gcc -Wall main.c rand.c blob.c dict.c `pkg-config fuse3 --cflags --libs` -o main

clean: 
	rm -f main