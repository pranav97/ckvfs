#ifndef BLOB_H
#define BLOB_H

#include <stdio.h>
#include <string.h>


#define MAX_PATH 100

#define NUM_BLOBS 100

#define MAX_INODEID 100

#define MAX_BLOCK 1000

typedef struct blob {
	char inodeid[MAX_PATH];
	char *data;
	size_t size;
} Blob;

Blob *getEntry(const char *path);
#endif

