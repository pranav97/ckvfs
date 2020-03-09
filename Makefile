all: main
CC=gcc
CXXFLAGS=-I /usr/local/include/fuse3 -I /src/KVSSD/PDK/core/include/ -D SAMSUNG_API -Wall

CFLAGS=-I /usr/local/include/fuse3 -I /src/KVSSD/PDK/core/include/ -D SAMSUNG_API -Wall
LDFLAGS=-L /src/KVSSD/PDK/core/build/ -L /usr/local/lib/x86_64-linux-gnu -lfuse3 -lpthread

main: main.o
	gcc -g main.o -lkvapi $(LDFLAGS) -o main

use_emulator: use_emulator.o
	gcc -g use_emulator.o -lkvapi $(LDFLAGS) -o use_emulator

test_sync: test_sync.o
	g++ -g test_sync.o -lkvapi $(LDFLAGS) -o test_sync

clean: 
	rm -f main main.o test_sync test_sync.o
