#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include "../../headers/helpers/crypto_operations.h"

int md5(const char *string, char *hash){

    unsigned char digest[MD5_HASH_LENGTH];

    // Create MD5 hash
    MD5_CTX context;
    MD5_Init(&context);
    MD5_Update(&context, string, strlen(string));
    MD5_Final(digest, &context);

    // Dump the hash into the buffer
    for(int i = 0; i < 16; i ++)
        sprintf(hash + i * 2, "%02x", (unsigned int)digest[i]);

    return 0;

}