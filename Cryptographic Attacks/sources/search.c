#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "../headers/helpers/file_operations.h"
#include "../headers/helpers/math_operations.h"
#include "../headers/helpers/conditon_checkers.h"

typedef struct{
    int thread_id;
    char *buffer;
    int buffer_size;
    int *f_values;
    int *found_indexes;
    int *found_count;
    pthread_mutex_t *found_mutex;
} thread_parameters;

int threads_count;
char *input_val_filename = NULL;
char *input_text_filename = NULL;
char *output_filename = NULL;

inline int sequence_function(char *string){

    int c = 0, i;

    for (i = 0; i < string[2]; i++)
        c ^= i;

    return ((((string[0] - 1) * string[0]) / 2 + ((string[1] - 1) * string[1]) / 2) | c | string[3]) ^ string[4];

}

void read_cmd_args(int argc, char **argv){

    if (argc < 5) {
        printf("How to run the program: ./search THREADS_COUNT INPUT_VALS_FILE INPUT_TEXT_FILE OUTPUT_FILENAME\n");
        exit(1);
    }

    threads_count = atoi(argv[1]);
    input_val_filename = argv[2];
    input_text_filename = argv[3];
    output_filename = argv[4];

}

void *thread_function(void *var){

    thread_parameters *parameters;
    int start_index, stop_index, first_value, second_value, f, i;

    parameters = (thread_parameters *)var;
    first_value = parameters->f_values[0];
    second_value = parameters->f_values[1];

    // Compute indexes for buffer
    start_index = parameters->thread_id * ceil(parameters->buffer_size / threads_count);
    stop_index = min(parameters->buffer_size, (parameters->thread_id + 1) * ceil(parameters->buffer_size / threads_count));

    // Iterate through buffer
    for (i = start_index; i < stop_index; i++){

        // Check if the end of the buffer is reached
        if (i == parameters->buffer_size - 4)
            break;

        // Compute the value of the function f and check with target values
        f = sequence_function(parameters->buffer + i);
        if (f == first_value || f == second_value){
            pthread_mutex_lock(parameters->found_mutex);
            parameters->found_indexes[*parameters->found_count] = i;
            (*parameters->found_count)++;
            pthread_mutex_unlock(parameters->found_mutex);
        }

    }

    return NULL;

}

int main(int argc, char **argv){

    pthread_t *tid = NULL;
    pthread_mutex_t found_mutex;
    thread_parameters *parameters = NULL;
    char out_buffer[21] = {'\0'};
    char *input_text = NULL;
    int *array = NULL;
    int indexes[2] = {0};
    int found_count = 0, is_error = 0, input_text_length, array_size, temp_index, i, ret_val;

    pthread_mutex_init(&found_mutex, NULL);

    // Read arguments and the content of given files
    read_cmd_args(argc, argv);
    ret_val = load_int_array_from_file(input_val_filename, &array, &array_size);
    GOTO_CONDITION_CHECKER(ret_val != 0, is_error, EXIT_MAIN_1);
    GOTO_CONDITION_CHECKER(array_size != 2, is_error, EXIT_MAIN_2);
    ret_val = read_file_content(input_text_filename, &input_text, &input_text_length);
    GOTO_CONDITION_CHECKER(ret_val != 0, is_error, EXIT_MAIN_2);

    // Allocate memory for threads and initialize their parameters
    tid = (pthread_t *)malloc(threads_count * sizeof(pthread_t));
    GOTO_CONDITION_CHECKER(tid == NULL, is_error, EXIT_MAIN_3);
    parameters = (thread_parameters *)malloc(threads_count * sizeof(thread_parameters));
    GOTO_CONDITION_CHECKER(parameters == NULL, is_error, EXIT_MAIN_4);
    for (i = 0; i < threads_count; i++){
        parameters[i].thread_id = i;
        parameters[i].buffer = input_text;
        parameters[i].buffer_size = input_text_length;
        parameters[i].f_values = array;
        parameters[i].found_indexes = indexes;
        parameters[i].found_count = &found_count;
        parameters[i].found_mutex = &found_mutex;
    }

    // Run threads for searching the delimitator
    for (i = 0; i < threads_count; i++)
        pthread_create(&(tid[i]), NULL, thread_function, &(parameters[i]));
    for (i = 0; i < threads_count; i++)
        pthread_join(tid[i], NULL);

    // Write indexes to file
    if (indexes[0] > indexes[1]){
        temp_index = indexes[0];
        indexes[0] = indexes[1];
        indexes[1] = temp_index;
    }
    indexes[0] += 5;
    sprintf(out_buffer, "%d %d\n", indexes[0], indexes[1]);
    ret_val = write_content_to_file(output_filename, out_buffer, strlen(out_buffer));
    GOTO_CONDITION_CHECKER(ret_val != 0, is_error, EXIT_MAIN_5);
    EXIT_MAIN_5:
        free(parameters);
    EXIT_MAIN_4:
        free(tid);
    EXIT_MAIN_3:
        free(input_text);
    EXIT_MAIN_2:
        free(array);
    EXIT_MAIN_1:
        pthread_mutex_destroy(&found_mutex);
        return is_error;

}