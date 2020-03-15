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


void test_insert(const char *k, char * v);
void test_iterator();
void test_read();
void test_api();
// testing CRUD
void test_api() {
    set_up_ssd();
    char val[4096];
    char key[MAX_INODEID];
    strcpy(key, "00000hello1");
    fprintf(stderr, "==================================\n");
    strcpy(val, "value1");
    test_insert(key, val);
    test_iterator();
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
	root2 -> is_dir = false;
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
void test_write_dir() {
  Blob *root2 = malloc(sizeof(Blob));
	const char root_inode[MAX_INODEID] = "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002";
	strcpy(root2 -> inodeid, root_inode);
	root2 -> is_dir = true;
  strcpy(root2 -> sub_items[0].item_path, "item path");
  strcpy(root2 -> sub_items[0].inodeid, "0923457890239870598743528743aaaaaaa");
  root2 -> sub_items[0].is_dir = false;
  strcpy(root2 -> sub_items[1].item_path, "item path2");
  strcpy(root2 -> sub_items[1].inodeid, "0923457892222222298743528743aaaaaaa");
  root2 -> sub_items[1].is_dir = false;
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


int main() {
    test_api();
    // test_write_file();
    // test_write_dir();
    return 0;
}



