#define MAX_PATH 1024
typedef struct blob {
	char path[MAX_PATH];
	char *data;
	size_t size;
} Blob;

static Blob tbl [100];
static int totalBlobs = 0;

Blob *getEntry(const char *path) {
	fprintf(stderr, "this is the path in entry %s", path);
	for(int i = 0; i < totalBlobs; i++) {
		if (strcmp(tbl[i].path, path) == 0) {
			return &tbl[i];
		}
	}

	return NULL;
}

