all: main

main: clean
	gcc -Wall main.c `pkg-config fuse3 --cflags --libs` -o main -lcrypto

clean: 
	rm -f main