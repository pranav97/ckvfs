#ifndef BLOB_H
#define BLOB_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_NAME 20

#define MAX_PATH 100

#define NUM_BLOBS 100

#define MAX_INODEID 100

#define MAX_BLOCK 10000

#define MAX_ITEMS 40

#define MISSING_FILE 1

#define NO_SUCH_DIR 2

#define CONTAINS 0


#define FAILED 1
#define SUCCESS 0
#define SMALL_STR 15

#define ISDIR 100
#define NOTDIR 50


#define NOTCONTAIN  1
typedef struct item {
	char item_path[MAX_PATH];
	char inodeid[MAX_INODEID];
	int is_dir;
} Item;

typedef struct write_blob {
	int is_dir;  // common
	int num_items; // dir
	size_t size; // file
	char inodeid[MAX_INODEID]; // common
	// this data will just have the raw bytes of either sub_items or data
	unsigned char data[MAX_BLOCK]; // common
} WriteBlob; 

typedef struct blob {
	Item sub_items [MAX_ITEMS]; // dir
	char inodeid[MAX_INODEID];
	char *data; // file
	size_t size;
	int num_items; 
	int is_dir;
} Blob;

extern void print_paths(Blob *b);
	
extern int has_path(Blob *b, const char *path);

extern void insert_item_into_blob(Blob *b, const char *name, int is_dir, char *inodeid);

extern void get_inode_from_path(Blob *b, const char *path, char *rand);

extern void get_first_name(char *path, char * first_name);

extern Blob* go_through_inodes(const char *path);

extern void get_fn_dir(const char *path, char*dir, char*fn);

extern void remove_inode_from_path(Blob *cur_blob,const char* file_name);

#endif

