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
		
		Blob *target_blob = tbl[0];
		get_inode_from_path(target_blob, path, &rand[0]);
		Blob *b = get_blob_from_key(rand);
		stbuf->st_size = b -> size;
		stbuf->st_mode = b -> is_dir ?  (S_IFDIR | 0755) : (S_IFREG | 0444);
		stbuf->st_nlink = 1;
	}

	return res;
}
void get_list(Blob * b, void *buf, fuse_fill_dir_t filler) {
	int i = 0;
	while (i < b -> num_items) {
		fprintf(stderr, "filler with name %s\n", &b -> sub_items[i] -> item_path[1]);
		filler(buf, & b -> sub_items[i] -> item_path[1], NULL, 0, 0);
		i++;
	}

}
static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
  /*
    filler()'s prototype looks like this:

    int fuse_fill_dir_t(void *buf, const char *name,
    const struct stat *stbuf, off_t off);
  */
  /*
    // this file system uses the system call *readdir* to get a list of entries and then calls filler to fill in the stuff into the buffer
    // the buffer is the output of this file readdir call.
    de = readdir(dp);
    if (de == 0)
    return -errno;

    // This will copy the entire directory into the buffer.  The loop exits
    // when either the system readdir() returns NULL, or filler()
    // returns something non-zero.  The first case just means I've
    // read the whole directory; the second means the buffer is full.
    do {
    log_msg("calling filler with name %s\n", de->d_name);
    if (filler(buf, de->d_name, NULL, 0) != 0)
    return -ENOMEM;
    } while ((de = readdir(dp)) != NULL);
  */
  /*
    system call readir uses the same error codes as this filessytem call

    EBADF  Invalid file descriptor fd.

    EFAULT Argument points outside the calling process's address space.

    EINVAL Result buffer is too small.
    ENOENT No such directory.

    ENOTDIR
    File descriptor does not refer to a directory.
  */

	fprintf(stderr, "Got into readir\n");
	fprintf(stderr, "the path is: %s\n", path);
	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	
	int retstat = 0;
	char rand[MAX_INODEID];
	(void) offset;
	(void) fi;
	(void) flags;
	Blob *root = get_root_inode();
	if (strcmp(path, "/") == 0) {
		get_list(root, buf, filler); 
		return 0;
	}
	else {
		/* all this doesn't matter till you get makedir to work */   
		get_inode_from_path(root, path, rand);

		if (strcmp(rand, "") == 0)  {
		/* cant find the root node this is a problem */
			fprintf(stderr, "NOT FOUND\n");

			return -ENOENT;
		}
		Blob *b = get_blob_from_key(rand);
		if (b == NULL) {
			/* cant find the node that they are looking for in the table of keys and their values */
			return -ENOENT;
		}
		if (b ->  is_dir == false) {
			return -ENOTDIR;
		}
	}
	return retstat;
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
	Blob *b = get_blob_from_key(found_rand);
	memcpy(b -> data, buf, size);
	b->size = size;
	printTBL();
	return size;
}


static int hello_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	// todo make this go ahead and get all the nodes down the tree

	Blob *root = get_root_inode();
	int contains = has_path(root, path);
	if (contains == 1) {
		insert_item_into_blob(root, path, false);
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

static int hello_mkdir(const char *path, mode_t mode)
{
	int res = 0;
	Blob *root = get_root_inode();
	int contains = has_path(root, path);
	if (contains == 1) {
		insert_item_into_blob(root, path, true);
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
		new_blob -> is_dir = true;
		tbl[totalBlobs] = new_blob;
		strcpy(keys[totalBlobs], inodeid);
        totalBlobs ++;
	}

	return res;
}

static const struct fuse_operations hello_oper = {
	.init     = hello_init,
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	.open		  = hello_open,
	.read		  = hello_read,
	.write		= hello_write,
	.create 	= hello_create,
	.mkdir 		= hello_mkdir
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
