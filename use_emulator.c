#include <stdbool.h>
#include <stdlib.h>
#include <kvs_api.h>



#define KLEN 100
#define VLEN 4096

#define FAILED 1
#define SUCCESS 0
#define SMALL_STR 15


#define iter_buff (32*1024)

struct ssd_handle {
    kvs_container_handle cont_handle;
    kvs_init_options options;
    char *configfile;
    char *dev_path;
    kvs_device_handle dev;
    char *cont_name;
};

static struct ssd_handle shand;



struct iterator_info{
  kvs_iterator_handle iter_handle;
  kvs_iterator_list iter_list;
  int has_iter_finish;
  kvs_iterator_option g_iter_mode;
};

void set_up_ssd();
int _env_init();

int perform_insertion(const char *k, const char *value, size_t val_size);
int perform_read(const char *k, char *buff);


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


int perform_insertion(const char *k, const char *val, size_t val_size) {
    kvs_key_t klen = KLEN;

    char *key   = (char*)kvs_malloc(klen, 4096);
    char *value = (char*)kvs_malloc(VLEN, 4096);
    strcpy(key, k);
    strcpy(value, val);
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

    fprintf(stderr, "KVAPI: Inserted kv pair: { %s : %s }\n", key, value);
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
    kvs_key_t klen = KLEN;


    int ret;
    char *key   = (char*)kvs_malloc(klen, 4096);
    char *value = (char*)kvs_malloc(VLEN, 4096);
    strcpy(key, k);
    // strcpy(value, val);
    if(key == NULL || value == NULL) {
        fprintf(stderr, "KVAPI: failed to allocate\n");
        return FAILED;
    }

    memset(value, 0, VLEN);
    kvs_retrieve_option option;
    memset(&option, 0, sizeof(kvs_retrieve_option));
    option.kvs_retrieve_decompress = false;
    option.kvs_retrieve_delete = false;

    const kvs_retrieve_context ret_ctx = {option, 0, 0};
    const kvs_key  kvskey = {key, klen };
    kvs_value kvsvalue = { value, VLEN , 0, 0 /*offset */};

    ret = kvs_retrieve_tuple(shand.cont_handle, &kvskey, &kvsvalue, &ret_ctx);
    if(ret != KVS_SUCCESS) {
        fprintf(stderr, "KVAPI: retrieve tuple %s failed with error 0x%x - %s\n", key, ret, kvs_errstr(ret));
        //exit(1);
    } else {
        fprintf(stderr, "KVAPI: retrieve tuple %s with value = %s, vlen = %d, actual vlen = %d \n", key, value, kvsvalue.length, kvsvalue.actual_value_size);
        strcpy(buff, value);
    }

    if(key) kvs_free(key);
    if(value) kvs_free(value);

    return SUCCESS;
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
    fprintf(stderr, "KVAPI: iterator open fails with error 0x%x - %s\n", ret, kvs_errstr(ret));
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
      fprintf(stderr, "KVAPI: iterator next fails with error 0x%x - %s\n", ret, kvs_errstr(ret));
      free(iter_info);
      kvs_free(buffer);
      return FAILED;
    }
        
    total_entries += iter_info->iter_list.num_entries;
    fprintf(stderr, "KVAPI: total ent %d\n", total_entries);
    // print_iterator_keyvals(&iter_info->iter_list, iter_info->g_iter_mode);
        
    if(iter_info->iter_list.end) {
      fprintf(stderr, "KVAPI: Done with all keys. Total: %d\n", total_entries);
      break;
    } else {
      fprintf(stderr, "KVAPI: More keys available, do another iteration\n");
      memset(iter_info->iter_list.it_list, 0,  iter_buff);
    }
  }

  clock_gettime(CLOCK_REALTIME, &t2);
  unsigned long long start, end;
  start = t1.tv_sec * 1000000000L + t1.tv_nsec;
  end = t2.tv_sec * 1000000000L + t2.tv_nsec;
  double sec = (double)(end - start) / 1000000000L;
  fprintf(stderr, "KVAPI: Total time %.2f sec; Throughput %.2f keys/sec\n", sec, (double)total_entries /sec );

  /* Close iterator */
  kvs_iterator_context iter_ctx_close;
  iter_ctx_close.private1 = NULL;
  iter_ctx_close.private2 = NULL;

  ret = kvs_close_iterator(shand.cont_handle, iter_info->iter_handle, &iter_ctx_close);
  if(ret != KVS_SUCCESS) {
    fprintf(stderr, "KVAPI: Failed to close iterator with error 0x%x - %s\n", ret,
      kvs_errstr(ret));
    kvs_free(buffer);
    free(iter_info);
    return FAILED;
  }
  
  if(buffer) kvs_free(buffer);
  if(iter_info) free(iter_info);

  return SUCCESS;
}


void set_up_ssd() {
    char *conf = malloc(sizeof(char) * SMALL_STR);
    char *dev_path = malloc(sizeof(char) * SMALL_STR);
    char *cont_name = malloc(sizeof(char) * SMALL_STR);

    kvs_init_env_opts(&shand.options);
    strcpy(conf, "../kvssd_emul.conf");
    strcpy(dev_path, "/dev/kvemul");
    strcpy(cont_name, "test");
    shand.cont_name = cont_name;
    shand.dev_path = dev_path;
    shand.configfile = conf;   
    shand.options.memory.use_dpdk = 0;
    shand.options.emul_config_file = shand.configfile;

    if(_env_init() != SUCCESS)
        fprintf(stderr, "KVAPI: not able to initialize\n");
}

void insert_read_iterate() {
    set_up_ssd();
    char val[4096];
    strcpy(val, "world");
    perform_insertion("0000Hello", val, strlen(val));
    strcpy(val, "world234");
    perform_insertion("0000hola", val, strlen(val));
    strcpy(val, "world23433333");
    perform_insertion("000045dd3", val, strlen(val));
    char recvd[VLEN];
    perform_read("0000Hello", recvd);

    fprintf(stderr, "KVAPI: received %s\n",recvd);
    fprintf(stderr, "KVAPI: reading all keys\n");
    fprintf(stderr, "KVAPI: ----------------------------\n");
    perform_iterator();
    fprintf(stderr, "KVAPI: ----------------------------\n");

}

int main() {
    insert_read_iterate();
    return 0;
}



