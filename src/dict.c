#include "dict.h"


#define iter_buff (32*1024)
extern struct ssd_handle shand;


void set_up_ssd(const char *config_file) {
    kvs_init_env_opts(&shand.options);
    strcpy(shand.cont_name, "test");
    strcpy(shand.dev_path, "/dev/kvemul");
    strcpy(shand.configfile,  config_file);
    shand.options.memory.use_dpdk = 0;
    shand.options.emul_config_file = shand.configfile;

    if(_env_init() != SUCCESS) {
        fprintf(stderr, "KVAPI: not able to initialize\n");
        exit(1);
    }
        
    
}

int _env_init() {
    //   
  // initialize the environment
  kvs_init_env(&shand.options);
  kvs_result ret = kvs_open_device(shand.dev_path, &shand.dev);
  if(ret != KVS_SUCCESS) {
    fprintf(stderr, "KVAPI: Device open failed\n");
    return FAILED;
  }

  //container list before create "test"
  uint32_t valid_cnt = 0;
  const uint32_t retrieve_cnt = 2;
  kvs_container_name names[retrieve_cnt];
  for(uint8_t idx = 0; idx < retrieve_cnt; idx++) {
    names[idx].name_len = MAX_CONT_PATH_LEN;
    names[idx].name = (char*)malloc(MAX_CONT_PATH_LEN);
  }
  ret = kvs_list_containers(shand.dev, 1, retrieve_cnt*sizeof(kvs_container_name),
    names, &valid_cnt);
  if(ret != KVS_SUCCESS) {
    fprintf(stderr, "KVAPI: List current containers failed. error:0x%x.\n", ret);
    kvs_close_device(shand.dev);
    return FAILED;
  }
  for (uint8_t idx = 0; idx < valid_cnt; idx++) {
    kvs_delete_container(shand.dev, names[idx].name);
  }

  kvs_container_context ctx;
  // Initialize key order to KVS_KEY_ORDER_NONE
  ctx.option.ordering = KVS_KEY_ORDER_NONE;
  ret = kvs_create_container(shand.dev, shand.cont_name, 0, &ctx);
  if (ret != KVS_SUCCESS) {
    fprintf(stderr, "KVAPI: Create containers failed. error:0x%x.\n", ret);
    kvs_close_device(shand.dev);
    return FAILED;
  }


  ret = kvs_open_container(shand.dev, shand.cont_name, &shand.cont_handle);
  if(ret != KVS_SUCCESS) {
    fprintf(stderr, "KVAPI: Open containers %s failed. error:0x%x.\n", shand.cont_name, ret);
    kvs_delete_container(shand.dev, shand.cont_name);
    kvs_close_device(shand.dev);
    return FAILED;
  }

  char *name_buff = (char*)malloc(MAX_CONT_PATH_LEN);
  kvs_container_name name = {MAX_CONT_PATH_LEN, name_buff};
  kvs_container cont = {false, 0, 0, 0, 0, &name};
  ret = kvs_get_container_info(shand.cont_handle, &cont);
  if(ret != KVS_SUCCESS) {
    fprintf(stderr, "KVAPI: Get info of containers %s failed. error:0x%x.\n", name.name, ret);
    _env_exit(shand.dev, shand.cont_name, shand.cont_handle);
    return FAILED;
  }

  fprintf(stderr, "KVAPI: Container information get name: %s\n", cont.name->name);
  fprintf(stderr, "KVAPI: open:%d, scale:%d, capacity:%ld, free_size:%ld count:%ld.\n", 
    cont.opened, cont.scale, cont.capacity, cont.free_size, cont.count);
  free(name_buff);

  int32_t dev_util = 0;
  int64_t dev_capa = 0;
  kvs_get_device_utilization(shand.dev, &dev_util);

  float waf = 0.0;
  kvs_get_device_waf(shand.dev, &waf);
  kvs_get_device_capacity(shand.dev, &dev_capa);
  fprintf(stderr, "KVAPI: Before: Total size is %ld bytes, used is %d, waf is %.2f\n", dev_capa, dev_util, waf);
  
  kvs_device *dev_info = (kvs_device*)malloc(sizeof(kvs_device));
  if(dev_info){
    kvs_get_device_info(shand.dev, dev_info);
    fprintf(stderr, "KVAPI: Total size: %.2f GB\nMax value size: %d\nMax key size: %d\n"
      "Optimal value size: %d\n", (float)dev_info->capacity/1000/1000/1000,
    dev_info->max_value_len, dev_info->max_key_len, dev_info->optimal_value_len);
    free(dev_info);
  }else{
    fprintf(stderr, "KVAPI: dev_info malloc failed\n");
    _env_exit(shand.dev, shand.cont_name, shand.cont_handle);
    return FAILED;
  }

  return SUCCESS;
}

int perform_insertion(const char *k, const char *val) {
    kvs_key_t klen = MAX_INODEID;
    size_t val_size = sizeof(WriteBlob);
    char *key   = (char*)kvs_malloc(klen, 4096);
    char *value = (char*)kvs_malloc(val_size, 4096);
    strcpy(key, k);
    memcpy(value, val, val_size);
    if(key == NULL || value == NULL) {
        fprintf(stderr, "KVAPI: failed to allocate\n");
        return FAILED;
    }
    kvs_store_option option;
    option.st_type = KVS_STORE_POST;
    option.kvs_store_compress = false;    
    const kvs_store_context put_ctx = { option, 0, 0 };
    const kvs_key kvskey = { key, klen };
    const kvs_value kvsvalue = { value, val_size, 0, 0};
    // print is broken here because it's not a char, it's unsigned char 
    // fprintf(stderr, "KVAPI: Inserted kv pair: { %s : %s }\n", key, value);
    int ret = kvs_store_tuple(shand.cont_handle, &kvskey, &kvsvalue, &put_ctx);    
    if(ret != KVS_SUCCESS ) {
        fprintf(stderr, "KVAPI: store tuple failed with error 0x%x - %s\n", ret, kvs_errstr(ret));
        kvs_free(key);
        kvs_free(value);
        return FAILED;
    }

    if(key) kvs_free(key);
    if(value) kvs_free(value);

    return SUCCESS;
}

int perform_read(const char *k, char *buff) {
    kvs_key_t klen = MAX_INODEID;
    size_t val_size = sizeof(WriteBlob);

    int ret = SUCCESS;
    char *key   = (char*)kvs_malloc(klen, 4096);
    char *value = (char*)kvs_malloc(val_size, 4096);
    strcpy(key, k);
    // strcpy(value, val);
    if(key == NULL || value == NULL) {
        fprintf(stderr, "KVAPI: failed to allocate\n");
        return FAILED;
    }

    memset(value, 0, val_size);
    kvs_retrieve_option option;
    memset(&option, 0, sizeof(kvs_retrieve_option));
    option.kvs_retrieve_decompress = false;
    option.kvs_retrieve_delete = false;

    const kvs_retrieve_context ret_ctx = {option, 0, 0};
    const kvs_key  kvskey = {key, klen };
    kvs_value kvsvalue = { value, val_size , 0, 0 /*offset */};
    ret = kvs_retrieve_tuple(shand.cont_handle, &kvskey, &kvsvalue, &ret_ctx);
    if(ret != KVS_SUCCESS) {
        fprintf(stderr, "KVAPI: retrieve tuple %s failed with error 0x%x - %s\n", key, ret, kvs_errstr(ret));
        ret = FAILED;
        //exit(1);
    } else {
        // fprintf(stderr, "KVAPI: retrieve tuple %s with value = %s, vlen = %d, actual vlen = %d \n", key, value, kvsvalue.length, kvsvalue.actual_value_size);
        memcpy(buff, value, sizeof(WriteBlob));
    }

    if(key) kvs_free(key);
    if(value) kvs_free(value);

    return ret;
}

void print_iterator_keyvals(kvs_iterator_list *iter_list, kvs_iterator_option g_iter_mode){
  uint8_t *it_buffer = (uint8_t *) iter_list->it_list;
  uint32_t i;

  if(g_iter_mode.iter_type) {
    // key and value iterator (KVS_ITERATOR_KEY_VALUE)
    uint32_t vlen = sizeof(kvs_value_t);
    uint32_t vlen_value = 0;

    for(i = 0;i < iter_list->num_entries; i++) {
      fprintf(stdout, "Iterator get %dth key: %s\n", i, it_buffer);
      it_buffer += 16;

      uint8_t *addr = (uint8_t *)&vlen_value;
      for (unsigned int i = 0; i < vlen; i++) {
	*(addr + i) = *(it_buffer + i);
      }

      it_buffer += vlen;
      it_buffer += vlen_value;
      // for ETA50K24 firmware
      //fprintf(stderr, "error: iterator(KVS_ITERATOR_KEY_VALUE) is not supported.\n");
    }
  } else {

    // For fixed key length
    /*
    
    for(i = 0; i < iter_list->num_entries; i++)
      fprintf(stdout, "Iterator get key %s\n",  it_buffer + i * 16);
    */
    
    // ETA50K24 firmware
    // key only iterator (KVS_ITERATOR_KEY)
    uint32_t key_size = 0;
    char key[256];
    char value[4096];
    for(i = 0;i < iter_list->num_entries; i++) {
       // get key size
       key_size = *((unsigned int*)it_buffer);
       it_buffer += sizeof(unsigned int);

       // print key
       memcpy(key, it_buffer, key_size);
       key[key_size] = 0;

       perform_read(key, value);
       fprintf(stdout, "%dth key --> %s\n", i, key);
       fprintf(stdout, "val ---> %s\n", value);


       it_buffer += key_size;
    }
      
  }
}

int perform_iterator()
{
  kvs_iterator_type iter_type=KVS_ITERATOR_KEY;
  struct iterator_info *iter_info = (struct iterator_info *)malloc(sizeof(struct iterator_info));
  iter_info->g_iter_mode.iter_type = iter_type;

  int ret;
  static int total_entries = 0;

  /* Open iterator */

  kvs_iterator_context iter_ctx_open;

  iter_ctx_open.bitmask = 0xffff0000;
  char prefix_str[5] = "0000";
  unsigned int PREFIX_KV = 0;
  for (int i = 0; i < 4; i++){
    PREFIX_KV |= (prefix_str[i] << (3-i)*8);
  }

  iter_ctx_open.bit_pattern = PREFIX_KV;
  iter_ctx_open.private1 = NULL;
  iter_ctx_open.private2 = NULL;

  iter_ctx_open.option.iter_type = iter_type;
  
  ret = kvs_open_iterator(shand.cont_handle, &iter_ctx_open, &iter_info->iter_handle);
  if(ret != KVS_SUCCESS) {
    fprintf(stderr, "iterator open fails with error 0x%x - %s\n", ret, kvs_errstr(ret));
    free(iter_info);
    return FAILED;
  }
  /* Do iteration */
  iter_info->iter_list.size = iter_buff;
  uint8_t *buffer;
  buffer =(uint8_t*) kvs_malloc(iter_buff, 4096);
  iter_info->iter_list.it_list = (uint8_t*) buffer;

  kvs_iterator_context iter_ctx_next;
  iter_ctx_next.private1 = iter_info;
  iter_ctx_next.private2 = NULL;

  iter_info->iter_list.end = 0;
  iter_info->iter_list.num_entries = 0;

  struct timespec t1, t2;
  clock_gettime(CLOCK_REALTIME, &t1);

  while(1) {
    iter_info->iter_list.size = iter_buff;
    memset(iter_info->iter_list.it_list, 0, iter_buff);
    ret = kvs_iterator_next(shand.cont_handle, iter_info->iter_handle, &iter_info->iter_list, &iter_ctx_next);
    if(ret != KVS_SUCCESS) {
      fprintf(stderr, "iterator next fails with error 0x%x - %s\n", ret, kvs_errstr(ret));
      free(iter_info);
      kvs_free(buffer);
      return FAILED;
    }
        
    total_entries += iter_info->iter_list.num_entries;
    print_iterator_keyvals(&iter_info->iter_list, iter_info->g_iter_mode);
        
    if(iter_info->iter_list.end) {
      fprintf(stdout, "Done with all keys. Total: %d\n", total_entries);
      break;
    } else {
      fprintf(stdout, "More keys available, do another iteration\n");
      memset(iter_info->iter_list.it_list, 0,  iter_buff);
    }
  }
  printf("Finished doing the iteration");
  exit(1);

  clock_gettime(CLOCK_REALTIME, &t2);
  unsigned long long start, end;
  start = t1.tv_sec * 1000000000L + t1.tv_nsec;
  end = t2.tv_sec * 1000000000L + t2.tv_nsec;
  double sec = (double)(end - start) / 1000000000L;
  fprintf(stdout, "Total time %.2f sec; Throughput %.2f keys/sec\n", sec, (double)total_entries /sec );

  /* Close iterator */
  kvs_iterator_context iter_ctx_close;
  iter_ctx_close.private1 = NULL;
  iter_ctx_close.private2 = NULL;

  ret = kvs_close_iterator(shand.cont_handle, iter_info->iter_handle, &iter_ctx_close);
  if(ret != KVS_SUCCESS) {
    fprintf(stderr, "Failed to close iterator with error 0x%x - %s\n", ret,
      kvs_errstr(ret));
    kvs_free(buffer);
    free(iter_info);
    return FAILED;
  }
  
  if(buffer) kvs_free(buffer);
  if(iter_info) free(iter_info);

  return SUCCESS;
}

int perform_delete(const char *k) {
  kvs_key_t klen = MAX_INODEID;
  char *key  = (char*)kvs_malloc(klen, 4096);
  strcpy(key, k);

  if(key == NULL) {
    fprintf(stderr, "failed to allocate\n");
    return FAILED;
  }
  const kvs_key  kvskey = { key, klen};
  
  const kvs_delete_context del_ctx = { {false}, 0, 0};
  int ret = kvs_delete_tuple(shand.cont_handle, &kvskey, &del_ctx);
  if(ret != KVS_SUCCESS) {
    fprintf(stderr, "delete tuple failed with error 0x%x - %s\n", ret, kvs_errstr(ret));
    kvs_free(key);
    return FAILED;
  } else {
    fprintf(stderr, "delete key %s done \n", key);
  }
  if(key) kvs_free(key);
  return SUCCESS;
}


// this is for write
WriteBlob *convert_to_write_blob(Blob *b) {
  WriteBlob *w = malloc(sizeof(WriteBlob));
  w -> is_dir = b -> is_dir;
  w -> size = b -> size;
  strcpy(w -> inodeid, b -> inodeid);
  w -> num_items = b -> num_items;
  if (b -> is_dir == NOTDIR) {
    memcpy(w -> data, b -> data, b -> size);
  }
  else {
    if (sizeof(Item) * MAX_ITEMS > MAX_BLOCK) {
      fprintf(stderr, "corruuption \n");
      // todo, is there a way to make this exit go away?
      exit(1);
    }
    memcpy(w -> data, b -> sub_items, sizeof(Item) * b -> num_items);
  }
  return w;
}

// this is for read
Blob *convert_to_blob(WriteBlob *b) {
  // malloc fails sometimes 
  Blob *translation = (Blob*) malloc(sizeof(Blob));
  if (translation == NULL) {
    return NULL;
  }
  translation -> size = b -> size;
  translation -> is_dir = b -> is_dir;
  translation -> num_items = b -> num_items;
  strcpy(translation -> inodeid, b -> inodeid);

  if (b -> is_dir == ISDIR) {
    memcpy(translation -> sub_items, b -> data, (sizeof(Item) * b -> num_items));
  }
  else {
    translation -> data = malloc(sizeof(char) * MAX_BLOCK);
    memcpy(translation -> data, b -> data, b -> size);
  }
  return translation;

}

void print_writable_blob(WriteBlob *w) {
  int l = strlen(w -> inodeid);
  if (l > 10) {
    fprintf(stderr, "w -> inodeid: ... %s\n", &w -> inodeid[l - 5]);
  }
  else {
    fprintf(stderr, "w -> inodeid: %s\n", w -> inodeid);
  }
  
  if (w -> is_dir == ISDIR) {
    fprintf(stderr, "w -> data: not printed because dir\n");
  }
  else {
    int len = w -> size;
		if (len > 10) {
			fprintf(stderr, "w -> data: ... %s\n", &w -> data[len - 10]);
		}
		else {
			fprintf(stderr, "w -> data: %s\n", w -> data);
		}
  }
  
  fprintf(stderr, "w -> num_items: %lld\n", w -> num_items);
}



int _env_exit() {
  int32_t dev_util = 0;
  kvs_get_device_utilization(shand.dev, &dev_util);
  fprintf(stderr, "KVAPI: After: Total used is %d\n", dev_util);
  kvs_close_container(shand.cont_handle);
  kvs_delete_container(shand.dev, shand.cont_name);
  kvs_close_device(shand.dev);
  kvs_result ret = kvs_exit_env();
  return ret;
}

void reset_tbl() {
	fprintf(stderr, "Oops can't reset this, please run system calls to remove everything.");
}

Blob *get_blob_from_key(char *key) {
  WriteBlob wb;
  char *cwb = (char *) &wb;
  perform_read(key, cwb);
  Blob *b = convert_to_blob(&wb);
  return b;
	
}

void write_to_dict(char *root_inode, Blob *b) {
  // print_blob(b);
  WriteBlob *wb = convert_to_write_blob(b);
  char *cwb = (char *) wb;
  perform_insertion(root_inode, cwb);
  free(wb);
  
}

void printTBL() {
  fprintf(stderr, "Not implmemented\n");
}

Blob *get_root_inode() {
  WriteBlob recvd;
  perform_read(root_id, (char *) &recvd);
  Blob *b =  convert_to_blob(&recvd);
  return b;

}

void write_root() {
	Blob *root = malloc(sizeof(Blob));
  memset(root, 0, sizeof(Blob));
	strcpy(root -> inodeid, root_id);
	root -> is_dir = ISDIR;
	root -> num_items = 0;
	write_to_dict(root -> inodeid, root);
	free(root->data);
	free(root);

}
