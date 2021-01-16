#ifndef _HASH_H

#define _HASH_H

#include "../miscellaneous/data_types.h"

#define MD5_LENGTH_IN_BYTES 16

/*
 * Computes MD5 hash for a given string and places it into an already allocated
 * buffer
 */
int md5(const char *string, uint length, char *hash);

#endif