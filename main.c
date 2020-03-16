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

#include <stdbool.h>
#include <kvs_api.h>


#include "blob.h"
#include "rand.h"
#include "dict.h"



#include "blob.c"
#include "rand.c"
#include "dict.c"


struct ssd_handle shand;


static struct options {
	const char *filename;
	const char *contents;
	const int use_dpdk;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--use_dpdk", use_dpdk),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void *hello_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	memset((void *) &shand, 0, sizeof(shand));
	set_up_ssd(); 
	(void) conn;
	cfg->kernel_cache = 1;
	
	// put in random seed every time that the file system comes up.
	set_time_srand_seed(); 
	write_root();
	return NULL;
}

static int hello_getattr_wrapped(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;
	char rand[MAX_INODEID];
	char file_name[MAX_NAME], dir_name[MAX_NAME];
	if (strcmp("/HEAD", path) == 0 || strcmp("/", path) == 0) {
		Blob *root = get_root_inode();
		stbuf->st_size = root -> size;
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return res;
	}

	get_fn_dir(path, dir_name, file_name);
	// fprintf(stderr, "dir_name %s\n", dir_name);
	// fprintf(stderr, "file_name %s\n", file_name);
	Blob *cur_blob = go_through_inodes(dir_name);
	if (cur_blob == NULL) {
		fprintf(stderr, "nulled out cur_blob in go thorugh inodes");
		return -ENOENT;
	}
	// fprintf(stderr, "path was - %s\n", dir_name);
	// fprintf(stderr, "actual name - %s\n", file_name); 
	// fprintf(stderr, "cur_blob inode - %s\n", cur_blob -> inodeid); 
	
	if (has_path(cur_blob, file_name) == CONTAINS) {
		if (cur_blob -> is_dir == ISDIR) {
			get_inode_from_path(cur_blob, file_name, rand);
			Blob *b = get_blob_from_key(rand);
			if (b -> is_dir == ISDIR) {
				stbuf->st_size = b -> size;
				stbuf->st_mode = S_IFDIR | 0755;
				stbuf->st_nlink = 2;
			}
			else {
				stbuf->st_size = b -> size;
				stbuf->st_mode = (S_IFREG | 0444);
				stbuf->st_nlink = 1;
			}
		}
		else {
			res = -ENOENT;
		}
	}
	else {
		res = -ENOENT;
	}
	return res;
}

static int hello_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	int ret_val = hello_getattr_wrapped(path, stbuf, fi);
	// fprintf(stderr, "returning %d\n", ret_val);
	// fprintf(stderr, "size that it returns was %d\n", stbuf -> st_size);
	return ret_val;

}

void get_list(Blob * b, void *buf, fuse_fill_dir_t filler) {
	int i = 0;
	while (i < b -> num_items) {
		// fprintf(stderr, "filler with name %s\n", &b -> sub_items[i].item_path);
		filler(buf, b -> sub_items[i].item_path, NULL, 0, 0);
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
  
  
    this file system uses the system call *readdir* to get a list of entries and then calls filler to fill in the stuff into the buffer
    the buffer is the output of this file readdir call.
    de = readdir(dp);
    if (de == 0)
    return -errno;

    This will copy the entire directory into the buffer.  The loop exits
    when either the system readdir() returns NULL, or filler()
    returns something non-zero.  The first case just means I've
    read the whole directory; the second means the buffer is full.
    do {
    log_msg("calling filler with name %s\n", de->d_name);
    if (filler(buf, de->d_name, NULL, 0) != 0)
    return -ENOMEM;
    } while ((de = readdir(dp)) != NULL);
    system call readir uses the same error codes as this filessytem call

    EBADF  Invalid file descriptor fd.

    EFAULT Argument points outside the calling process's address space.

    EINVAL Result buffer is too small.
    ENOENT No such directory.

    ENOTDIR
    File descriptor does not refer to a directory.
	*/
	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	
	int retstat = 0;
	char rand[MAX_INODEID];
	(void) offset;
	(void) fi;
	(void) flags;

	char file_name[MAX_NAME], dir_name[MAX_NAME];

	
	if (strcmp(path, "/") == 0) {
		Blob * root = get_root_inode();
		get_list(root, buf, filler); 
		return 0;
	}
	else {
		get_fn_dir(path, dir_name, file_name);
		Blob *cur_blob = go_through_inodes(dir_name);
		if (cur_blob == NULL) {
			return -ENOENT;
		}
		if (cur_blob -> is_dir == NOTDIR) {
			return -ENOTDIR;
		}
		get_inode_from_path(cur_blob, file_name, rand);
		cur_blob = get_blob_from_key(rand);
		get_list(cur_blob, buf, filler); 
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
	char file_name[MAX_NAME], dir_name[MAX_NAME];
	get_fn_dir(path, dir_name, file_name);
	Blob *cur_blob = go_through_inodes(dir_name);
	if (cur_blob == NULL) {
		return 0;
	}
	get_inode_from_path(cur_blob, file_name, rand);
	if (strcmp(rand, "") == 0)  {
		return -ENOENT;
	}
	Blob *b = get_blob_from_key(rand);
	if (b == NULL)  {
		return -ENOENT;
	}
	memcpy(buf, b -> data, b -> size);
	return b -> size;
}


static int hello_write(const char *path, const char *buf, size_t size,
		      off_t offset, struct fuse_file_info *fi)
{
	(void) buf;
	(void) offset;
	(void) fi;
	char file_name[MAX_NAME], dir_name[MAX_NAME];
	char rand[MAX_INODEID];

	char data[MAX_BLOCK] = "";
	if ((strlen(buf) + strlen(path)) > MAX_BLOCK) {
		 // todo - find a way to split the data and put it back together. 
		return 0;
	}
	strcat(data, buf);
	
	get_fn_dir(path, dir_name, file_name);
	Blob *cur_blob = go_through_inodes(dir_name);
	if (cur_blob == NULL) {
		fprintf(stderr, "nulled out cur_blob in go thorugh inodes");
		return -ENOENT;
	}
	get_inode_from_path(cur_blob, file_name, rand);
	Blob *b = get_blob_from_key(rand);
	memcpy(b -> data, buf, size);
	b->size = size;
	printTBL();
	write_to_dict(b -> inodeid, b);
	return size;
}


static int hello_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	// todo make this go ahead and get all the nodes down the tree
	int res = 0;
	char file_name[MAX_NAME], dir_name[MAX_NAME];
	char inodeid[MAX_INODEID], rand[MAX_INODEID];
	get_fn_dir(path, dir_name, file_name);
	Blob *cur_blob  = go_through_inodes(dir_name);
	if (cur_blob == NULL) {
		return 0;
	}
	else if (cur_blob -> is_dir) {
		if (has_path(cur_blob, file_name) == NOTCONTAIN) {
			insert_item_into_blob(cur_blob, file_name, false, inodeid);
			// print_blob(cur_blob);
			Blob *new_blob = (Blob *) malloc(sizeof(Blob));
			new_blob -> num_items = 0;
			char * new_bl = malloc(MAX_BLOCK * sizeof(char));
			strcpy(new_bl, ""); 
			new_blob -> data = new_bl;
			strcpy(new_blob -> inodeid, inodeid);
			new_blob -> size = 0;
			new_blob -> is_dir = NOTDIR;
			write_to_dict(new_blob -> inodeid, new_blob);
		}
	}
	printTBL();
	return res;
}
	

static int hello_mkdir(const char *path, mode_t mode)
{
	// todo potentially have to refactor mkdir and create to be similar. 
	// The only difference is 
	// 1) the insert iterm into blob arguments 
	// 2) new blob types
	int res = 0;
	char rand[MAX_INODEID];
	char new_dir_name[MAX_NAME], dir_name[MAX_NAME];
	char inodeid[MAX_INODEID];
	
	get_fn_dir(path, dir_name, new_dir_name);
	Blob *cur_blob = go_through_inodes(dir_name);
	if (cur_blob == NULL) {
		fprintf(stderr, "nulled out cur_blob in go thorugh inodes");
		return -ENOTDIR;
	}
	if (has_path(cur_blob, new_dir_name) == NOTCONTAIN) {
		if (cur_blob -> is_dir == ISDIR) {
			insert_item_into_blob(cur_blob, new_dir_name, true, inodeid);
			Blob *new_blob = (Blob *) malloc(sizeof(Blob));
			// fprintf(stderr, "created entry with inode id %s", inodeid);
			
			strcpy(new_blob -> inodeid, inodeid);
			new_blob -> num_items = 0;
			new_blob -> data = NULL;
			new_blob -> size = 0;
			new_blob -> is_dir = ISDIR;
			write_to_dict(inodeid, new_blob);
		}
	}
	printTBL();
	return res;
}

 int hello_unlink(const char *path) {
	int res = 0;
	fprintf(stderr, "Path to be removed is: %s\n", path);
	char rand[MAX_INODEID];
	char file_name[MAX_NAME], dir_name[MAX_NAME];
	get_fn_dir(path, dir_name, file_name);
	Blob *cur_blob = go_through_inodes(dir_name);
	if (cur_blob == NULL) {
		return res;
	}
	get_inode_from_path(cur_blob, file_name, rand);
	if (strcmp(rand, "") == 0)  {
		return res;
	}
	perform_delete(rand);
	remove_inode_from_path(cur_blob, file_name);
	write_to_dict(cur_blob -> inodeid, cur_blob);
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
	.mkdir 		= hello_mkdir,
	.unlink 	= hello_unlink
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
	if (options.use_dpdk == 1) {
		#define USE_DPDK 1
	}

	ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}
