#ifndef BLOB_H
#define BLOB_H

#include <stdio.h>
#include <string.h>


#define MAX_PATH 100

#define MAX_KEY 1024

#define NUM_BLOBS 100

#define MAX_SHA 100

#define MAX_BLOCK 4096

typedef struct blob {
	char path[MAX_PATH];
	char *data;
	size_t size;
} Blob;

Blob *getEntry(const char *path);
#endif

