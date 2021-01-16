 //to be installed using sudo apt-get install libssl-dev
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "../../helpers/CCookbook/io/file_operations.h"
#include "../../helpers/CCookbook/cryptography/hash.h"
#include "../../helpers/CCookbook/miscellaneous/math_operations.h"
#include "../../helpers/CCookbook/miscellaneous/conditon_checkers.h"
#include "../../helpers/CCookbook/miscellaneous/data_types.h"

#define MAX_WORD_SIZE 255
#define MAX_WORD_IN_A_PHASE 7

typedef struct{
    int thread_id;
    UT_array *word_array;
    int base;
    int full_interval_length;
    int skip_to_value;
    char *searched_hash;
    int *found_hash_flag;
} thread_parameters;

int threads_number;
char *dictionary_filename = NULL;
char *hash_filename = NULL;
char *output_filename = NULL;

void read_cmd_args(int argc, char **argv){

    if (argc < 5){
        printf("How to run the program: ./md5 threads_number DICT_FILE_NAME MD5_HASH_FILENAME OUTPUT_FILENAME\n");
        exit(1);
    }

    threads_number = atoi(argv[1]);
    dictionary_filename = argv[2];
    hash_filename = argv[3];
    output_filename = argv[4];

}

void *thread_function(void *var){

    thread_parameters *parameters;
    char word_indexes[MAX_WORD_IN_A_PHASE], phase_buffer[MAX_WORD_IN_A_PHASE * (MAX_WORD_SIZE + 1)], md5_hash[2 * MD5_LENGTH_IN_BYTES + 1];
    char **found_element;
    int start_index, stop_index, i, j, index, divisor, reminder, temp_reminder, found_zero, digit_count;

    parameters = (thread_parameters *)var;

    // Compute indexes for buffer
    start_index = parameters->skip_to_value + parameters->thread_id * ceil(parameters->full_interval_length / threads_number);
    stop_index = parameters->skip_to_value + min(parameters->full_interval_length, (parameters->thread_id + 1) * ceil(parameters->full_interval_length / threads_number));

    // Iterate through buffer
    for (i = start_index; i < stop_index; i++){

        // Check if a hash was already found
        if (*(parameters->found_hash_flag) == 1)
            break;

        memset(word_indexes, 0, MAX_WORD_IN_A_PHASE);
        memset(phase_buffer, 0, MAX_WORD_IN_A_PHASE * (MAX_WORD_SIZE + 1));

        reminder = i;
        temp_reminder = reminder / parameters->base;
        divisor = 1;
        while (divisor <= temp_reminder)
            divisor *= parameters->base;

        found_zero = 0;
        digit_count = 0;
        while (divisor){

            index = reminder / divisor;
            word_indexes[digit_count] = index;

            if ((found_zero == 0 && index == 0) || (found_zero != 0 && index != 0))
                found_zero++;

            reminder %= divisor;
            divisor /= parameters->base;

            digit_count++;

        }

        // Skip numbers that were already processed
        if (found_zero > 1)
            continue;

        // Append the word from the computed index (digit on the given base) to buffer
        for (j = 0; j < MAX_WORD_IN_A_PHASE; j++){
            if (word_indexes[j] != 0){
                found_element = utarray_eltptr(parameters->word_array, word_indexes[j] - 1);
                if (j != 0)
                    strcat(phase_buffer, " ");
                strcat(phase_buffer, *found_element);
            }
        }

        // Compute phase hash and compare it with the wanted one
        md5(phase_buffer, strlen(phase_buffer), md5_hash);
        if (strcmp(parameters->searched_hash, md5_hash) == 0){
            strcat(phase_buffer, "\n");
            write_content_to_file(output_filename, phase_buffer, strlen(phase_buffer));
            *(parameters->found_hash_flag) = 1;
        }

    }

    return NULL;

}

int main(int argc, char **argv){

    pthread_t *tid = NULL;
    thread_parameters *parameters = NULL;
    UT_array *word_array = NULL;
    char *hash = NULL;
    uint hash_length;
    int found_hash_flag = 0, is_error = 0, word_array_size, base, full_interval_length, skip_to_value, ret_val, i;

    // Read arguments and the content of given files
    read_cmd_args(argc, argv);
    ret_val = load_string_array_from_file(dictionary_filename, MAX_WORD_SIZE, &word_array, &word_array_size);
    GOTO_CONDITION_CHECKER(ret_val != 0, is_error, EXIT_MAIN_1);
    ret_val = read_file_content(hash_filename, &hash, &hash_length);
    GOTO_CONDITION_CHECKER(ret_val != 0, is_error, EXIT_MAIN_2);

    // Allocate memory for threads and initialize their parameters
    tid = (pthread_t *)malloc(threads_number * sizeof(pthread_t));
    GOTO_CONDITION_CHECKER(tid == NULL, is_error, EXIT_MAIN_3);
    parameters = (thread_parameters *)malloc(threads_number * sizeof(thread_parameters));
    GOTO_CONDITION_CHECKER(parameters == NULL, is_error, EXIT_MAIN_4);
    base = word_array_size + 1;
    skip_to_value = pow(base, 6);
    full_interval_length = pow(base, 7) - skip_to_value + 1;
    for (i = 0; i < threads_number; i++){
        parameters[i].thread_id = i;
        parameters[i].word_array = word_array;
        parameters[i].base = base;
        parameters[i].full_interval_length = full_interval_length;
        parameters[i].searched_hash = hash;
        parameters[i].skip_to_value = skip_to_value;
        parameters[i].found_hash_flag = &found_hash_flag;
    }

    // Run threads for computing hashes from word combinations
    for (i = 0; i < threads_number; i++)
        pthread_create(&(tid[i]), NULL, thread_function, &(parameters[i]));
    for (i = 0; i < threads_number; i++)
        pthread_join(tid[i], NULL);

    free(parameters);
    EXIT_MAIN_4:
        free(tid);
    EXIT_MAIN_3:
        free(hash);
    EXIT_MAIN_2:
        utarray_free(word_array);
    EXIT_MAIN_1:
        return is_error;

}