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
void print_items(Item * items[], int num_items) {
	for (int i = 0; i < num_items; i++) {
		fprintf(stderr, "\tPath: %s\n", items[i] -> item_path);
		fprintf(stderr, "\tinodeid: %s\n", items[i] -> inodeid);
		fprintf(stderr, "\t is bool %d",items[i] -> is_dir);
	}
}

void print_blob(Blob *theBlob) {
	fprintf(stderr, "blob id %s \n ->", theBlob -> inodeid);
	fprintf(stderr, "data: %s\n", theBlob -> data);
	fprintf(stderr, "Items are \n");
	print_items(theBlob -> sub_items, theBlob -> num_items);
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
