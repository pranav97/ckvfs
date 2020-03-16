#include <stdbool.h>
#include <stdlib.h>

#include <kvs_api.h>

#include "blob.h"
#include "rand.h"
#include "dict.h"

#define VLEN 1000

#include "blob.c"
#include "rand.c"
#include "dict.c"

struct ssd_handle shand;

const char root_inode[MAX_INODEID] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002";

void test_insert(const char *k, char * v);
void test_iterator();
void test_read();
void test_api();
// testing CRUD
void test_api() {
    set_up_ssd();
    // char val[4096];
    // char key[MAX_INODEID];
    // strcpy(key, "00000hello1");
    // fprintf(stderr, "==================================\n");
    // strcpy(val, "value1");
    // test_insert(key, val);
    // test_iterator();
    // fprintf(stderr, "==================================\n");
    // strcpy(val, "value2");
    // strcpy(key, "00000hello2");
    // test_insert(key, val);
    // test_iterator();
    // fprintf(stderr, "==================================\n");
    // fprintf(stderr, "this is an overwrite of the value\n");
    // strcpy(val, "value3");
    // test_insert(key, val);
    // test_iterator();
    // fprintf(stderr, "==================================\n");
    // test_delete(key);
    // test_iterator();
    // fprintf(stderr, "==================================\n");
}
void test_read() {
  char recvd[VLEN];
  perform_read("0000Hello", recvd);
  fprintf(stderr, "KVAPI: received %s\n",recvd);
}

void test_insert(const char *k, char * v) {
  char val[4096];
  strcpy(val, v);
  perform_insertion(k, val);
}


void test_iterator() {
  fprintf(stderr, "KVAPI: reading all keys\n");
  fprintf(stderr, "----------------------------\n");
  perform_iterator();
  fprintf(stderr, "----------------------------\n");
}

void test_delete(const char *k) {
    perform_delete(k);
}

void test_write_file() {
  Blob *root2 = malloc(sizeof(Blob));
	const char root_inode[MAX_INODEID] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002";
	strcpy(root2 -> inodeid, root_inode);
	root2 -> is_dir = NOTDIR;
  char *d = malloc(sizeof(char) * MAX_BLOCK);
  strcpy(d, "this is some sample data to write into the file ");
  root2 -> data = d;
  root2 -> num_items = 0;


  fprintf(stderr, "==========================\n");
  fprintf(stderr, "reference \n");
  print_blob(root2);
  fprintf(stderr, "==========================\n");

  fprintf(stderr, "Converted write blob\n");
  WriteBlob *to_write = convert_to_write_blob(root2);
  print_writable_blob(to_write);
  fprintf(stderr, "==========================\n");

  fprintf(stderr, "Converted back to blob\n");
  Blob *cpy = convert_to_blob(to_write);
  print_blob(cpy);
  fprintf(stderr, "==========================\n");


}


void test_insert_item_into() {
  Blob *root2 = malloc(sizeof(Blob));
	strcpy(root2 -> inodeid, root_inode);
	root2 -> is_dir = ISDIR;
  root2 -> num_items = 0;
  root2 -> size = 0;
  root2 -> data = malloc(sizeof(Item) * MAX_ITEMS);
  char new_inode_id[MAX_INODEID];
  insert_item_into_blob(root2, "hello", false, new_inode_id);

  Blob *new_blob = malloc(sizeof(new_blob));
  strcpy(new_blob -> inodeid, new_inode_id);
  new_blob -> is_dir = NOTDIR;
  new_blob -> data = malloc(sizeof(MAX_BLOCK));
  new_blob -> size = 0;
  write_to_dict(new_inode_id, new_blob);



  WriteBlob *read = malloc(sizeof(WriteBlob));
  memset(read, 0 , sizeof(WriteBlob));
  perform_read(new_inode_id, (char *) read);
  Blob *bb = convert_to_blob(read);
  print_blob(bb);


  memset(read, 0 , sizeof(WriteBlob));
  perform_read(root_inode, (char *) read);
  free(bb);
  bb = convert_to_blob(read);
  print_blob(bb);


  free(bb);
  free(root2 -> data);
  free(root2);
}
void test_write_dir() {
  Blob *root2 = malloc(sizeof(Blob));
	strcpy(root2 -> inodeid, root_inode);
	root2 -> is_dir = ISDIR;
  strcpy(root2 -> sub_items[0].item_path, "item path");
  strcpy(root2 -> sub_items[0].inodeid, "0923457890239870598743528743aaaaaaa");
  root2 -> sub_items[0].is_dir = NOTDIR;
  strcpy(root2 -> sub_items[1].item_path, "item path2");
  strcpy(root2 -> sub_items[1].inodeid, "0923457892222222298743528743aaaaaaa");
  root2 -> sub_items[1].is_dir = NOTDIR;
	root2 -> num_items = 2;

  fprintf(stderr, "==========================\n");
  fprintf(stderr, "reference \n");
  print_blob(root2);
  fprintf(stderr, "==========================\n");

  fprintf(stderr, "Converted write blob\n");
  WriteBlob *to_write = convert_to_write_blob(root2);
  print_writable_blob(to_write);

  fprintf(stderr, "Converted back to blob\n");
  Blob *cpy = convert_to_blob(to_write);
  print_blob(cpy);
  
}

void test_root() {
  char buff[MAX_BLOCK];
  write_root();
  Blob *b = get_root_inode();
  fprintf(stderr, "Number of things in the dir is %d\n", b -> num_items);
  if (b -> is_dir == ISDIR) {
    fprintf(stderr, "is dir is set\n");
  }
  else {
    fprintf(stderr, "is dir is NOT set\n");
  }
  print_blob(b);
}

int main() {
    test_api();
    // test_write_file();
    // test_write_dir();
    // test_insert_item_into();
  	
    test_root();
    return 0;
}



