all: main
CC=gcc
CXX=g++
CXXFLAGS=-I/src/KVSSD/PDK/core/include/ -D SAMSUNG_API -Wall
CFLAGS=-I/usr/local/include/fuse3 -I /src/KVSSD/PDK/core/include/ -D SAMSUNG_API -Wall
LDFLAGS=-L/src/KVSSD/PDK/core/build/ -L/usr/local/lib/x86_64-linux-gnu -lfuse3 -lpthread

main: main.o
	$(CC) -g main.o $(LDFLAGS) -lkvapi -o main

use_emulator: use_emulator.o
	$(CC) -g use_emulator.o $(LDFLAGS) -lkvapi  -o use_emulator

test_sync: test_sync.o
	$(CXX) -g test_sync.o $(LDFLAGS) -lkvapi  -o test_sync

clean: 
	rm -f main main.o test_sync test_sync.o use_emulator use_emulator.o
