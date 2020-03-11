#include "dict.h"

void reset_tbl() {
	totalBlobs = 0;
}

Blob *get_blob_from_key(char *key) {
	for (int i = 0; i < totalBlobs; i++ ) {
		if (strcmp(key, keys[i]) == 0) {
			return tbl[i];
		}
	}
	return NULL;
}

void write_to_dict(char *root_inode, Blob *b) {
    strcpy(keys[totalBlobs], root_inode);
	tbl[totalBlobs] = b;
	totalBlobs++;
}

void printTBL() {
	fprintf(stderr, "\nThe tbl is: \n");
	for(int i = 0; i < totalBlobs; i++) {
		Blob *tblob = tbl[i];
		print_blob(tblob);
	}
}

Blob *get_root_inode() {
    return tbl[0];

}
