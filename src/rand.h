#ifndef RAND_H
#define RAND_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
# define DECIMAL  10
# define HEX 16
# define OCTAL 8



extern void reverse(char s[]);
extern void itoa(unsigned int n, char s[], int offset);
extern void get_random_num(char *buffer);
extern void set_time_srand_seed();
#endif
