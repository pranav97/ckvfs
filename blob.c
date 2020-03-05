#include "blob.h"
#include "rand.h"
#include "dict.h"

void print_paths(Blob *b) {
    fprintf(stderr, "\nThe paths are: \n");
    for(int i = 0; i < b -> num_items; i++)
        fprintf(stderr, "path %s\n", b -> sub_items[i] -> item_path);
}

int has_path(Blob *b, const char *path) {
	// todo - this funciton should be recursive, looking at even the dirs
	// that are in here as opposed to just the files
    // todo - make this return a blob upon recursion
	for(int i = 0; i < b -> num_items; i++) {
		if (strcmp(b -> sub_items[i] -> item_path, path) == 0) {
            // contains
			return 0;
		}
	}
    // does not contain
	return 1;
}

void insert_item_into_blob(Blob *b, const char *name) {
    // todo take in information about whether a dir or not 
    if (has_path(b, name) == 0) {
        return;
    }
    else {
        char *new_inodeid;
        new_inodeid = (char *) malloc(MAX_INODEID * sizeof(char));
        get_random_num(new_inodeid);


        char *new_name;
        new_name = (char *) malloc(MAX_PATH * sizeof(char));
        strcpy(new_name, name);
        
        Item *new_item = (Item *) malloc(sizeof(Item));
        new_item -> item_path = new_name;
        new_item -> inodeid = new_inodeid;
        
        b -> sub_items[b -> num_items] = new_item;
        b -> num_items ++;

    }
}

void get_inode_from_path(Blob *b, const char *path, char *rand) {
	// puts the inode id of the blob that the user is looking for
	for(int i = 0; i < b -> num_items; i++) {
		if (strcmp(b -> sub_items[i] -> item_path, path) == 0) {
			fprintf(stderr, "rand %s\n", b -> sub_items[i] -> inodeid);
			fprintf(stderr, "path is %s\n", b -> sub_items[i] -> item_path);
			strcpy(rand, b -> sub_items[i] -> inodeid);
			return;
		}
	}
	strcpy(rand, "");
}

