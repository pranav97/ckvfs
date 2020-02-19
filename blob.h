#ifndef BLOB_H
#define BLOB_H

#include <stdio.h>
#include <string.h>


#define MAX_PATH 100

// todo find out what this should be
#define MAX_KEY 101


#define NUM_BLOBS 100

#define MAX_SHA 100

#define MAX_BLOCK 1000

typedef struct blob {
	char sha[MAX_PATH];
	char *data;
	size_t size;
} Blob;

Blob *getEntry(const char *path);
#endif

