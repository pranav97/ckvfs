#ifndef DICT_H
#define DICT_H
#include "blob.h"



extern void reset_tbl();

extern void printTBL();

extern Blob *get_root_inode();

extern void write_to_dict(char *root_inode, Blob *b);

extern Blob *get_blob_from_key(char *key);

struct ssd_handle {
    kvs_container_handle cont_handle;
    kvs_init_options options;
    char *configfile;
    char *dev_path;
    kvs_device_handle dev;
    char *cont_name;
};





struct iterator_info{
  kvs_iterator_handle iter_handle;
  kvs_iterator_list iter_list;
  int has_iter_finish;
  kvs_iterator_option g_iter_mode;
};

void set_up_ssd();
int _env_init();
int perform_insertion(const char *k, const char *val);
int perform_read(const char *k, char *buff);
void print_iterator_keyvals(kvs_iterator_list *iter_list, kvs_iterator_option g_iter_mode);
int perform_iterator();
int perform_delete(const char *k);
WriteBlob *convert_to_write_blob(Blob *b);
Blob *convert_to_blob(WriteBlob *b);
void print_writable_blob(WriteBlob *w);
int _env_exit();

#endif