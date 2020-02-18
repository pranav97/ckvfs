#include <stdio.h>
#include <string.h>


#define MAX_PATH 100

#define MAX_KEY 1024

typedef struct blob {
	char path[MAX_KEY];
	char *data;
	size_t size;
} Blob;

#define NUM_BLOBS
