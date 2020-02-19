#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "blob.h"
#include "openssl/sha.h"

void sha256_hash_string (unsigned char hash[SHA256_DIGEST_LENGTH], char outputBuffer[65])
{
    int i = 0;

    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }

    outputBuffer[64] = 0;
}


void sha256_string(char *string, char outputBuffer[MAX_SHA])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

// int main()
// {
// 	static unsigned char buffer[65];
// 	sha256_string("hello my name is pranav", buffer);
// 	printf("%s\n", buffer);
// }
