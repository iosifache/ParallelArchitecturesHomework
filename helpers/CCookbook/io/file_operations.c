#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_operations.h"
#include "../miscellaneous/conditon_checkers.h"

int read_file_content(const char *filename, char **content, uint *content_length){

    FILE *file = NULL;
    uint readed_chars_count;
    int is_error = 0;

    // Open file for read
    GOTO_CONDITION_CHECKER(filename == NULL, is_error, EXIT_READ_FILE_CONTENT_1);
    file = fopen(filename, "r");
    GOTO_CONDITION_CHECKER(file == NULL, is_error, EXIT_READ_FILE_CONTENT_1);

    // Get file size
    fseek(file, 0, SEEK_END);
    readed_chars_count = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read the content into an allocated buffer
    *content = (char *)malloc(readed_chars_count * sizeof(char) + 1);
    GOTO_CONDITION_CHECKER(*content == NULL, is_error, EXIT_READ_FILE_CONTENT_2);
    fread(*content, readed_chars_count, 1, file);
    (*content)[readed_chars_count] = '\0';
    *content_length = readed_chars_count;

    EXIT_READ_FILE_CONTENT_2:
        fclose(file);
    EXIT_READ_FILE_CONTENT_1:
        return is_error;

}

int load_int_array_from_file(const char *filename, int **array, int *array_size){

    char *content = NULL, *token = NULL, *delims = " ";
    uint content_length;
    int spaces_count = 0, is_error = 0, i, ret_val;

    // Read file content
    ret_val = read_file_content(filename, &content, &content_length);
    GOTO_CONDITION_CHECKER(ret_val != 0, is_error, EXIT_LOAD_INT_ARRAY_FROM_FILE_1);

    // Allocate array
    for (i = 0; i < content_length; i++)
        if (content[i] == ' ')
            spaces_count++;
    *array = (int *)malloc((spaces_count + 1) * sizeof(int));
    GOTO_CONDITION_CHECKER(*array == NULL, is_error, EXIT_LOAD_INT_ARRAY_FROM_FILE_2);

    // Parse string
    token = strtok(content, delims);
    i = 0;
    while (token != NULL){
        (*array)[i] = atoi(token);
        i++;
        token = strtok(NULL, delims);
    }
    *array_size = i;

    EXIT_LOAD_INT_ARRAY_FROM_FILE_2:
        free(content);
    EXIT_LOAD_INT_ARRAY_FROM_FILE_1:
        return is_error;

}

int load_string_array_from_file(const char *filename, int max_string_size, UT_array **array, int *array_size){

    char *content = NULL, *token = NULL, *delims = "\n";
    uint content_length;
    int elements_count = 0, is_error = 0, i, ret_val;

    // Read file content
    ret_val = read_file_content(filename, &content, &content_length);
    GOTO_CONDITION_CHECKER(ret_val != 0, is_error, EXIT_LOAD_STRING_ARRAY_FROM_FILE_1);

    // Parse string and insert elements into array
    i = 0;
    token = strtok(content, delims);
    while (token != NULL){

        if (i == 0){

            elements_count = atoi(token);

            utarray_new(*array, &ut_str_icd);
            utarray_reserve(*array, elements_count);

        }
        else{

            utarray_push_back(*array, &token);

        }

        // Go to next element
        i++;
        token = strtok(NULL, delims);

    }
    *array_size = elements_count;

    free(content);
    EXIT_LOAD_STRING_ARRAY_FROM_FILE_1:
        return is_error;

}

int write_content_to_file(const char *filename, char *content, uint content_length){

    FILE *file = NULL;
    int is_error = 0;

    // Open file for read
    GOTO_CONDITION_CHECKER(filename == NULL, is_error, EXIT_WRITE_CONTENT_TO_FILE_1);
    file = fopen(filename, "w");
    GOTO_CONDITION_CHECKER(file == NULL, is_error, EXIT_WRITE_CONTENT_TO_FILE_2);

    // Write to file
    fwrite(content, content_length, 1, file);

    EXIT_WRITE_CONTENT_TO_FILE_2:
        fclose(file);
    EXIT_WRITE_CONTENT_TO_FILE_1:
        return is_error;

}