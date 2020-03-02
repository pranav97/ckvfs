#ifndef BLOB_H
#define BLOB_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_PATH 100

#define NUM_BLOBS 100

#define MAX_INODEID 100

#define MAX_BLOCK 1000

#define MAX_ITEMS 100

typedef struct item {
	char *item_path;
	char *inodeid;
	bool is_dir;
} Item;


typedef struct blob {
	Item *sub_items [MAX_ITEMS];
	char inodeid[MAX_INODEID];
	char *data;
	size_t size;
	int num_items;
	bool is_dir;
} Blob;


extern void print_paths(Blob *b);
	
extern int has_path(Blob *b, const char *path);

extern void insert_item_into_blob(Blob *b, const char *name);

extern void get_inode_from_path(Blob *b, const char *path, char *rand);

#endif

