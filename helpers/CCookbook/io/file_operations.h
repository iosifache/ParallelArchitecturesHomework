#ifndef _FILE_OPERATIONS_H

#define _FILE_OPERATIONS_H

#include "../miscellaneous/data_types.h"
#include "../extern_modules/uthash/include/utarray.h"

/*
 * Reads the content of a file and saves it into a dynamically allocated buffer
 */
int read_file_content(const char *filename, char **content, uint *content_size);

/*
 * Reads integers from file, separated by space, and saves them into a 
 * dynamically allocated array
 */
int load_int_array_from_file(const char *filename, int **array, int *array_size);

/*
 * Reads strings from file, separated by new line, and saves them into a 
 * dynamically alocated array
 */
int load_string_array_from_file(const char *filename, int max_string_size, UT_array **array, int *array_size);

/*
 * Writes content with a specific length into file
 */
int write_content_to_file(const char *filename, char *content, uint content_size);

#endif