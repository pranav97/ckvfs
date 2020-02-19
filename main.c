/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall hello.c `pkg-config fuse3 --cflags --libs` -o hello
 *
 * ## Source code ##
 * \include hello.c
 */


#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "blob.h"

#include "blob.h"
#include "checksum.c"

static Blob tbl [100];
static char all_paths[NUM_BLOBS][MAX_PATH];
static char all_shas[NUM_BLOBS][MAX_SHA];
static int totalBlobs = 0;
static int totalPaths = 0;

static struct options {
	const char *filename;
	const char *contents;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

void get_sha_from_path(const char *path, char *sha) {
	for(int i = 0; i < totalPaths; i++) {
		if (strcmp(all_paths[i], path) == 0) {
			fprintf(stderr, "\nsha %s\n", all_shas[i]);
			fprintf(stderr, "path is %s\n", path);
			strcpy(sha, all_shas[i]);
			return;
		}
	}
	strcpy(sha, "");
}

int *get_path(const char *path) {
	for(int i = 0; i < totalPaths; i++) {
		if (strcmp(all_paths[i], path) == 0) {
			return 0;
		}
	}
	return 1;
}

Blob *get_blob(const char *path) {
	for(int i = 0; i < totalBlobs; i++) {
		if (strcmp(tbl[i].sha, path) == 0) {
			return &tbl[i];
		}
	}

	return NULL;
}

void printTBL() {
	fprintf(stderr, "\nThe tbl is: \n");
	for(int i = 0; i < totalBlobs; i++) {
		Blob *tblob = &tbl[i];
		fprintf(stderr, "sha %s\n", all_shas[i]);
		fprintf(stderr, "key %s\n", tblob -> sha);
		fprintf(stderr, "val %s\n", tblob -> data);
	}

	fprintf(stderr, "\nThe paths are: \n");
	for(int i = 0; i < totalPaths; i++)
		fprintf(stderr, "path %s\n", all_paths[i]);

}


static void *hello_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

static int hello_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;
	char sha[MAX_SHA];


	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return res;
	}
	// char *strip_path = strip(path);
	int contains = get_path(path);
	if (contains == 1) {
		return -ENOENT;
	}
	

	stbuf->st_mode = S_IFREG | 0444;
	stbuf->st_nlink = 1;

	get_sha_from_path(path, &sha[0]);

	if (strcmp(sha, "") == 0) {
		stbuf->st_size = 0;
	}
	else {
		Blob *b = get_blob(sha);
		stbuf->st_size = b -> size;
	}

	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	filler(buf, options.filename, NULL, 0, 0);

	return 0;
}


static int hello_open(const char *path, struct fuse_file_info *fi)
{
	// this funciton is supposed to check if the user has the privilates usnig fuse_file_info
	return 0;
}


static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	(void) fi;
	char sha[MAX_SHA];
	get_sha_from_path(path, &sha[0]);
	fprintf(stderr, "Sha from read => %s\n", sha);

	if (strcmp(sha, "") == 0)  {
		return -ENOENT;
	}
	
	Blob *b = get_blob(sha);
	if (b == NULL)  {
		return -ENOENT;
	}
	fprintf(stderr, "file contents read => %s\n", b -> data);
	memcpy(buf, b -> data, b -> size);
	fprintf(stderr, "buf => %s\n", buf);
	fprintf(stderr, "size => %d\n", b -> size);
	return b -> size;
}


static int hello_write(const char *path, const char *buf, size_t size,
		      off_t offset, struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;
	
	char sha_data[MAX_BLOCK] = "";
	if ((strlen(buf) + strlen(path)) > MAX_BLOCK) {
		 // todo - find a way to split the data and put it back together. 
		return 0;
	}
	strcat(sha_data, buf);
	strcat(sha_data, path);
	sha256_string(sha_data, all_shas[totalBlobs]);

	Blob *b = get_blob(path);
	b = &tbl[totalBlobs];
	b->data = malloc(size);
	memcpy(b -> data, buf, size);
	strcpy(b -> sha, all_shas[totalBlobs]);
	totalBlobs++;
	
	b->size = size;
	printTBL();
	return size;
}


static int hello_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	// Blob *b = &tbl[totalBlobs];
	// strcpy(all_paths[totalBlobs], path);
	// strcpy(all_shas[totalBlobs], "");
	// totalBlobs++;
	int contains = get_path(path);
	if (contains == 1) {
		strcpy(all_paths[totalPaths++], path);
	}

	// strcpy(b -> path, path);
	// b->size = 0;
	printTBL();
	return 0;
}



static const struct fuse_operations hello_oper = {
	.init           = hello_init,
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	.open		= hello_open,
	.read		= hello_read,
	.write		= hello_write,
	.create 	= hello_create,
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"hello\" file\n"
	       "                        (default: \"hello\")\n"
	       "    --contents=<s>      Contents \"hello\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");
}

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("hello");
	options.contents = strdup("Hello World!\n");

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}

	ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}
