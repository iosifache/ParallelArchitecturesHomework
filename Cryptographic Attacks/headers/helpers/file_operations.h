#ifndef _FILE_OPERATIONS_H

#define _FILE_OPERATIONS_H

#include "utarray.h"

int read_file_content(const char *filename, char **content, int *content_size);
int load_int_array_from_file(const char *filename, int **array, int *array_size);
int load_string_array_from_file(const char *filename, int max_string_size, UT_array **array, int *array_size);
int write_content_to_file(const char *filename, char *content, int content_size);

#endif