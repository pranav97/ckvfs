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
#include "rand.h"
#include "dict.h"


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


static void *hello_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	// put in random seed every time that the file system comes up.
	set_time_srand_seed(); 

	Blob *root = malloc(sizeof(Blob));
	const char root_inode[MAX_INODEID] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002";
	strcpy(root -> inodeid, root_inode);
	root -> is_dir = true;
	root -> num_items = 0;
	write_to_dict(root -> inodeid, root);
	return NULL;
}

static int hello_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;
	char rand[MAX_INODEID];
	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return res;
	}
	else {
		Blob *root = get_root_inode();	
		int contains = has_path(root, path);
		fprintf(stderr, "contains is %d", contains);
		if (contains == 1) {
			return -ENOENT;
		}
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;

		Blob *target_blob = tbl[0];
		get_inode_from_path(target_blob, path, &rand[0]);
		Blob *b = get_blob_from_key(rand);
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
	char rand[MAX_INODEID];
	Blob *root = get_root_inode();	

	// todo: iterate though the inodes to find the right one 
	get_inode_from_path(root, path, rand);
	fprintf(stderr, "rand from read => %s\n", rand);

	if (strcmp(rand, "") == 0)  {
		return -ENOENT;
	}
	Blob *b = get_blob_from_key(rand);
	if (b == NULL)  {
		return -ENOENT;
	}
	fprintf(stderr, "file contents read => %s\n", b -> data);
	memcpy(buf, b -> data, b -> size);
	fprintf(stderr, "buf => %s\n", buf);
	fprintf(stderr, "size => %ld\n", b -> size);
	return b -> size;
}


static int hello_write(const char *path, const char *buf, size_t size,
		      off_t offset, struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;
	
	fprintf(stderr, "write \n");
	char data[MAX_BLOCK] = "";
	if ((strlen(buf) + strlen(path)) > MAX_BLOCK) {
		 // todo - find a way to split the data and put it back together. 
		return 0;
	}
	strcat(data, buf);
	char found_rand[MAX_INODEID];
	Blob *root = get_root_inode();
	get_inode_from_path(root, path, found_rand);
	fprintf(stderr, "the inode id is %s \n", found_rand);
	
	Blob *b = get_blob_from_key(found_rand);
	fprintf(stderr, "The new blob is %s\n", b -> inodeid);
	memcpy(b -> data, buf, size);
	fprintf(stderr, "memcopied the data\n");
	fprintf(stderr, "data is %s\n", b -> data);
	fprintf(stderr, "%s\n", b -> data);
	b->size = size;
	printTBL();
	return size;
}


static int hello_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	// todo make this go ahead and get all the nodes down the tree

	Blob *root = get_root_inode();
	int contains = has_path(root, path);
	if (contains == 1) {
		insert_item_into_blob(root, path);
		fprintf(stderr, "num_items %d\n", root -> num_items);
		fprintf(stderr, "path is %s\n",root -> sub_items[0] -> item_path);
		fprintf(stderr, "inode id %s\n", root -> sub_items[0] -> inodeid);
		fprintf(stderr, "isdir %d\n", root -> sub_items[0] -> is_dir);

		char inodeid[MAX_INODEID];
		get_inode_from_path(root, path, inodeid);
		Blob *new_blob = (Blob *) malloc(sizeof(Blob));
        new_blob -> num_items = 0;
        strcpy(new_blob -> inodeid, inodeid);
        char * new_bl = malloc(MAX_BLOCK * sizeof(char));
        strcpy(new_bl, "");
		new_blob -> data = new_bl;
        new_blob -> size = 0;
		tbl[totalBlobs] = new_blob;
		strcpy(keys[totalBlobs], inodeid);
        totalBlobs ++;
	}
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
