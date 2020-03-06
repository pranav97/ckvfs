#include <stdio.h>
#include <string.h>

void get_first_name(char *path, char * first_name) {
  int i = 0;
  while (path[i] != 0 && path[i] != '/') {
    first_name[i] = path[i];
    i++;
  }
  first_name[i] = 0;
  if (path[i]  == 0) {
    strcpy(path, "");
  }
  else {
    i++;
  }
  int j = 0;
  while (path[i] != 0) {
    path[j++] = path[i++];
  }
  path[j] = 0;

}

int main() {
  char first_name[100] = "";
  char path[100] = "hello/sir/my/name/is/pranav";
  strcpy(path, "ello/mate");
  get_first_name(path, first_name);
  printf("%s\n", path);
  printf("%s\n", first_name);
  return 0;
  int i = 0;
  while (1) {
    printf("\n");
    get_first_name(path, first_name);
    printf("%s\n", first_name);
    printf("%s\n", path);
    if (strcmp(path, "") == 0) {
      break;
    }
    strcpy(first_name, "");
  }
}
