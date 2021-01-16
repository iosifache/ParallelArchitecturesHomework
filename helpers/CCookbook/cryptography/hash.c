#include <openssl/md5.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"

int md5(const char *string, uint length, char *hash){

    MD5_CTX context;
    unsigned char digest[MD5_LENGTH_IN_BYTES];

    // Initialize context and create MD5 hash
    MD5_Init(&context);
    MD5_Update(&context, string, length);
    MD5_Final(digest, &context);

    // Dump the hash into the buffer
    for(int i = 0; i < 16; i ++)
        sprintf(hash + i * 2, "%02x", (unsigned int)digest[i]);

    return 0;

}