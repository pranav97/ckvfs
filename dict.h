#ifndef DICT_H
#define DICT_H
#include "blob.h"

Blob *tbl [100]; 
char keys[100][MAX_INODEID];
int totalBlobs;

extern void reset_tbl();

extern void printTBL();

extern Blob *get_root_inode();

extern void write_to_dict(char *root_inode, Blob *b);

extern Blob *get_blob_from_key(char *key);


#endif