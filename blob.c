#include "blob.h"
#include "rand.h"
#include "dict.h"


Blob* go_through_inodes(const char *path) {
	Blob *cur_blob = get_root_inode();
  	fprintf(stderr, "==================================\n");
	char rest[MAX_PATH], rand[MAX_INODEID];
	char *rest_ptr, *token;
	const char sep[2] = "/";
    rest_ptr = strdup(path);
	token = strtok(rest_ptr, sep);	
   	while( token != NULL ) {
		fprintf( stderr, "next item %s\n", token );
		if (has_path(cur_blob, token) == CONTAINS) {
			get_inode_from_path(cur_blob, token, rand);
			fprintf(stderr, "the inode number in go through - %s\n", rand);
			cur_blob = get_blob_from_key(rand);
			fprintf(stderr, "blob in go through - %s\n", cur_blob -> inodeid);
		}
		else {
			cur_blob = NULL;
			break;
		}
      	token = strtok(NULL, sep);
   	}

	fprintf(stderr, "==================================\n");
	return cur_blob;
}

void get_fn_dir(const char *path, char*dir, char*fn) {
    int i = 0;
	strcpy(dir, "/");

    for (i = strlen(path); i >= 0; i--) {
        if (path[i] == '/') {
          strcpy(fn, &path[i+1]);
          strncpy(dir, path, i);
          dir[i] = 0;
          break;
        }

    }
}


void print_paths(Blob *b) {
    fprintf(stderr, "\nThe paths are: \n");
    for(int i = 0; i < b -> num_items; i++)
        fprintf(stderr, "path %s\n", b -> sub_items[i].item_path);
}

int has_path(Blob *b, const char *path) {
	// todo - this funciton should be recursive, looking at even the dirs
	// that are in here as opposed to just the files
    // todo - make this return a blob upon recursion
	for(int i = 0; i < b -> num_items; i++) {
		// fprintf(stderr, "%s\n", b -> sub_items[i].item_path);
		// fprintf(stderr, "%s\n", path);
		if (strcmp(b -> sub_items[i].item_path, path) == 0) {
      		// contains
			return CONTAINS;
		}
	}
    // does not contain
	return NOTCONTAIN;
}

void insert_item_into_blob(Blob *b, const char *name, bool is_dir, char *inodeid) {
    // todo take in information about whether a dir or not 
    if (has_path(b, name) == CONTAINS) {
        return;
    }
    else {
        char *new_inodeid;
        new_inodeid = (char *) malloc(MAX_INODEID * sizeof(char));
        get_random_num(new_inodeid);
        strcpy(inodeid, new_inodeid);
        
        strcpy(b -> sub_items[b -> num_items].item_path, name);
        strcpy(b -> sub_items[b -> num_items].inodeid, new_inodeid);
        b -> sub_items[b -> num_items].is_dir = is_dir;
    
        b -> num_items ++;
		WriteBlob *wb = convert_to_write_blob(b);
		perform_insertion(new_inodeid, (char *) wb);
    }
}

void get_first_name(char *path, char * first_name) {
	if(path[0] == '/')   {
		strcpy(path, &path[1]);
		strcpy(first_name, "/");
		return;
	}
	int i = 0;
	while (path[i] != 0 && path[i] != '/') {
		first_name[i] = path[i];
		i++;
	}
	first_name[i] = 0;
	if (path[i]  == 0) {
		strcpy(path, "");
	}
	else {
		i++;
	}
	int j = 0;
	while (path[i] != 0) {
		path[j++] = path[i++];
	}
	path[j] = 0;

}

void walk_subitems(Blob *b, const char *path, char *rand) {
    for(int i = 0; i < b -> num_items; i++) {
		if (strcmp(b -> sub_items[i].item_path, path) == 0) {
			// fprintf(stderr, "rand %s\n", b -> sub_items[i].inodeid);
			// fprintf(stderr, "path is %s\n", b -> sub_items[i].item_path);
			strcpy(rand, b -> sub_items[i].inodeid);
			return;
		}
	}
    // not found 
    strcpy(rand, "");
}


void get_inode_from_path(Blob *b, const char *path, char *rand) {
    // fprintf(stderr, "IN THE GET INODE FROM PATH FUN\n");
    // fprintf(stderr, "the blob is -> %s\n", &b -> inodeid[90]);
    // fprintf(stderr, "path is -> %s\n", path);
	walk_subitems(b, path, rand);
}

void print_items(Item items[], int num_items) {
	for (int i = 0; i < num_items; i++) {
		fprintf(stderr, "\tPath: %s\n", items[i].item_path);
		fprintf(stderr, "\tinodeid: %s\n", items[i].inodeid);
		fprintf(stderr, "\tis dir %d\n",items[i].is_dir);
	}
}

void print_blob(Blob *theBlob) {
	int l = strlen(theBlob -> inodeid);
	if (l > 10) {
		fprintf(stderr, "theBlob -> nodeid: ... %s\n", &theBlob -> inodeid[l - 5]);
	}
	else {
		fprintf(stderr, "theBlob -> nodeid: %s\n", theBlob -> inodeid);
	}	
	if (theBlob -> is_dir) {
		fprintf(stderr, "theBlob -> subitems is\n");
		print_items(theBlob -> sub_items, theBlob -> num_items);
	}
	else {
		fprintf(stderr, "theBlob -> data: \n");
		int len = strlen(theBlob -> data);
		if (len > 10) {
			fprintf(stderr, "... %s\n", &theBlob -> data[len - 10]);
		}
		else {
			fprintf(stderr, "%s\n", theBlob -> data);
		}
	}
	fprintf(stderr, "\n\n");
}