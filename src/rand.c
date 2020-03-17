#include "rand.h"


/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}
/* itoa:  convert n to characters in s 
from Kernighan and Ritchie's The C Programming */ 
void itoa(unsigned int n, char s[], int offset)
{
    int i, dig;
    i = 0;
    /* generate digits in reverse order */
    do {       
        dig = n % offset;
        if (offset > 10) {
            if (dig >= 10) {
                dig = dig - 10;
                s[i++] = dig + 'a';
            }
            else {
                s[i++] = dig + '0';
            }
        }
        else {
            s[i++] = dig + '0';   /* get next digit */
        }
    } while ((n /= offset) > 0);     /* delete it */
    s[i] = '\0';
    reverse(s);
}  

void get_random_num(char *buffer) {
    strcpy(buffer, "");
    unsigned int i, t;
    char ret [150];
    strcpy(ret, "");

    char buffer2 [9];
    for (i = 0; i < 13; i++) {
        t = rand();
        itoa (t, buffer2, HEX);
        strcat(ret, buffer2);
    }
    for(i = 0; i < 96; i++) {
        buffer[i] = ret[i];
    }
    buffer[i] = 0;
}

void set_time_srand_seed() {
    time_t t;
    srand((unsigned) time(&t));
}

// void get_random_numbers() {
    
//     int i, n;
//     n = 5;
//     char buffer [97];
//     set_time_srand_seed();
    
   
//    /* Print 5 random numbers from 0 to 50 */
//    for( i = 0 ; i < n ; i++ ) {
//         get_random_num(buffer);
//         printf("%s\t %ld\n",buffer, strlen(buffer));
//    }
// }

// int main () {
//    get_random_numbers();
//    return(0);
// }