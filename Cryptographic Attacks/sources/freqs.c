#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "../headers/helpers/utlist.h"
#include "../headers/helpers/file_operations.h"
#include "../headers/helpers/string_operations.h"
#include "../headers/helpers/char_operations.h"
#include "../headers/helpers/math_operations.h"
#include "../headers/helpers/conditon_checkers.h"

typedef enum operation_type{
    COMPUTE_FREQUENCIES,
    DECRYPT_WORDS
} operation_type;

typedef struct word_pointer{
    char *pointer;
    char length;
    unsigned long hash;
    struct word_pointer *next;
} word_pointer;

typedef struct thread_parameters{
    int thread_id;
    char *buffer;
    int buffer_size;
    operation_type operation;
    int *frequencies;
    char *decryption_key;
    word_pointer **word_list;
    pthread_mutex_t *insert_mutex;
    int *performing_write_operation;
} thread_parameters;

int threads_count;
char *input_index_filename = NULL;
char *input_text_filename = NULL;
char *char_order_filename = NULL;
char *output_filename = NULL;

void read_cmd_args(int argc, char **argv){

    if (argc < 5){
        printf("How to run the program: ./freqs THREADS_COUNT INPUT_INDEX_FILE INPUT_TEXT_FILE CHAR_ORDER_FILE OUTPUT_FILENAME\n");
        exit(1);
    }

    threads_count = atoi(argv[1]);
    input_index_filename = argv[2];
    input_text_filename = argv[3];
    char_order_filename = argv[4];
    output_filename = argv[5];

}

int word_compare(word_pointer *first, word_pointer *second){

    if (first->hash == second->hash)
        return 0;

    return memcmp(first->pointer, second->pointer, second->length);

}

void *thread_function(void *var){

    thread_parameters *parameters;
    word_pointer *current_word = NULL, *found_word;
    char *last_word_start = NULL;
    int start_index, stop_index, i, index;

    parameters = (thread_parameters *)var;

    // Compute indexes for buffer
    start_index = parameters->thread_id * ceil(parameters->buffer_size / threads_count);
    stop_index = min(parameters->buffer_size, (parameters->thread_id + 1) * ceil(parameters->buffer_size / threads_count));

    // Iterate through buffer
    for (i = start_index; i < stop_index; i++){

        if (parameters->operation == COMPUTE_FREQUENCIES){

            // If the current character is a letter, increment its frequency
            index = GET_LETTER_INDEX(parameters->buffer[i]);
            if (index != -1)
                (parameters->frequencies[index])++;

        }
        else{

            // Decrypt letter via decryption key
            parameters->buffer[i] = parameters->decryption_key[GET_LOWERCASE_LETTER_INDEX(parameters->buffer[i])];

            // Check if a new word was found
            if (CHECK_IF_LOWERCASE_LETTER(parameters->buffer[i]) || last_word_start == NULL){
                if (last_word_start == NULL)
                    last_word_start = parameters->buffer + i;
            }
            else{

                CHECK_WORD:

                    // Create a new element to compare with the elements from list
                    if (current_word == NULL)
                        current_word = malloc(sizeof(word_pointer));
                    current_word->pointer = last_word_start;
                    parameters->buffer[i] = '\0';
                    current_word->hash = djb2(last_word_start);
                    current_word->length = strlen(last_word_start);
                    last_word_start = NULL;

                    // If element not found in the list, insert it
                    found_word = NULL;
                    while (*parameters->performing_write_operation == 1){
                        ;
                    }
                    LL_SEARCH(*(parameters->word_list), found_word, current_word, word_compare);
                    if (found_word == NULL){
                        pthread_mutex_lock(parameters->insert_mutex);
                        *parameters->performing_write_operation = 1;
                        LL_APPEND(*(parameters->word_list), current_word);
                        *parameters->performing_write_operation = 0;
                        pthread_mutex_unlock(parameters->insert_mutex);
                        current_word = NULL;
                    }

                    // Check if there is an uncompleted word
                    if (last_word_start != NULL && i == stop_index - 1 && stop_index != parameters->buffer_size){

                        // Find first non-letter character and build a new word
                        while (1){
                            if (!CHECK_IF_LOWERCASE_LETTER(parameters->buffer[i]))
                                goto CHECK_WORD;
                            else
                                i++;

                        }
                    }

            }

        }

    }

    if (current_word != NULL)
        free(current_word);
    return NULL;

}

int main(int argc, char **argv){

    FILE *output_file = NULL;
    pthread_t *tid = NULL;
    pthread_mutex_t insert_mutex;
    thread_parameters *parameters = NULL;
    word_pointer *word_list = NULL, *current_word, *temp;
    char *input_text = NULL, *char_order = NULL, *decryption_key = NULL;
    int *indexes_array = NULL, *final_frequency_array;
    int elements_count = 0, performing_write_operation = 0, is_error = 0, indexes_array_size, input_text_length, char_order_size, max_index, max_value, i, j, allocated_limit, ret_val;

    // Read arguments and the content of given files
    read_cmd_args(argc, argv);
    ret_val = load_int_array_from_file(input_index_filename, &indexes_array, &indexes_array_size);
    GOTO_CONDITION_CHECKER(ret_val != 0, is_error, EXIT_MAIN_1);
    GOTO_CONDITION_CHECKER(indexes_array_size != 2, is_error, EXIT_MAIN_2);
    ret_val = read_file_content(input_text_filename, &input_text, &input_text_length);
    GOTO_CONDITION_CHECKER(ret_val != 0, is_error, EXIT_MAIN_2);
    ret_val = read_file_content(char_order_filename, &char_order, &char_order_size);
    GOTO_CONDITION_CHECKER(ret_val != 0, is_error, EXIT_MAIN_3);

    // Remove CRLF from characters buffer
    if (char_order[char_order_size - 2] == '\r'){
        char_order[char_order_size - 2] = '\0';
        char_order_size -= 2;
    }

    // Allocate memory for threads and initialize their parameters
    tid = (pthread_t *)malloc(threads_count * sizeof(pthread_t));
    GOTO_CONDITION_CHECKER(tid == NULL, is_error, EXIT_MAIN_4);
    parameters = (thread_parameters *)malloc(threads_count * sizeof(thread_parameters));
    GOTO_CONDITION_CHECKER(parameters == NULL, is_error, EXIT_MAIN_5);
    for (i = 0; i < threads_count; i++){
        parameters[i].thread_id = i;
        parameters[i].buffer = input_text + indexes_array[0];
        parameters[i].buffer_size = indexes_array[1] - indexes_array[0];
        parameters[i].operation = COMPUTE_FREQUENCIES;
        parameters[i].decryption_key = NULL;
        parameters[i].word_list = NULL;
        parameters[i].frequencies = (int *)calloc(char_order_size, sizeof(int));
        GOTO_CONDITION_CHECKER(parameters[i].frequencies == NULL, is_error, EXIT_MAIN_6);
        parameters[i].performing_write_operation = &performing_write_operation;

    }

    // Run threads for computing the frequency
    for (i = 0; i < threads_count; i++)
        pthread_create(&(tid[i]), NULL, thread_function, &(parameters[i]));
    for (i = 0; i < threads_count; i++)
        pthread_join(tid[i], NULL);

    // Store the sum of the frequences in the vector of the first thread
    final_frequency_array = parameters[0].frequencies;
    for (i = 0; i < threads_count; i++)
        for (j = 0; j < char_order_size; j++){
            final_frequency_array[j] += (parameters[i].frequencies)[j];
        }

    // Compute decryption key 
    decryption_key = (char *)malloc(char_order_size * sizeof(char));
    GOTO_CONDITION_CHECKER(decryption_key == NULL, is_error, EXIT_MAIN_7);
    ret_val = pthread_mutex_init(&insert_mutex, NULL);
    GOTO_CONDITION_CHECKER(ret_val != 0, is_error, EXIT_MAIN_8);
    for (i = 0; i < char_order_size; i++){

        // Get max frequency
        max_index = 0;
        max_value = 0;
        for (j = 0; j < char_order_size; j++)
            if (final_frequency_array[j] >= max_value){
                max_index = j;
                max_value = final_frequency_array[j];
            }

        // Append in the key the found letter
        decryption_key[max_index] = char_order[i];
        final_frequency_array[max_index] = -1;

    }

    // Initialize threads parameters
    for (i = 0; i < threads_count; i++){
        parameters[i].operation = DECRYPT_WORDS;
        parameters[i].decryption_key = decryption_key;
        parameters[i].word_list = &word_list;
        parameters[i].insert_mutex = &insert_mutex;
    }

    // Run threads for decryption
    for (i = 0; i < threads_count; i++)
        pthread_create(&(tid[i]), NULL, thread_function, &(parameters[i]));
    for (i = 0; i < threads_count; i++)
        pthread_join(tid[i], NULL);

    // Dump all wanted content (count and sorted words) into file
    LL_SORT(word_list, word_compare);
    LL_COUNT(word_list, current_word, elements_count);
    output_file = fopen(output_filename, "w");
    GOTO_CONDITION_CHECKER(output_file == NULL, is_error, EXIT_MAIN_9);
    fprintf(output_file, "%d\n", elements_count);
    LL_FOREACH(word_list, current_word){
        fprintf(output_file, "%s\n", current_word->pointer);
    }
    fclose(output_file);

    LL_FOREACH_SAFE(word_list, current_word, temp){
        LL_DELETE(word_list, current_word);
        free(current_word);
    }
    EXIT_MAIN_9:
        pthread_mutex_destroy(&insert_mutex);
    EXIT_MAIN_8:
        free(decryption_key);
    EXIT_MAIN_7:
        allocated_limit = (is_error && i < threads_count) ? i : threads_count;
        for (i = 0; i < allocated_limit; i++)
            free(parameters[i].frequencies);
    EXIT_MAIN_6:
        free(parameters);
    EXIT_MAIN_5:
        free(tid);
    EXIT_MAIN_4:
        free(char_order);
    EXIT_MAIN_3:
        free(input_text);
    EXIT_MAIN_2:
        free(indexes_array);
    EXIT_MAIN_1:
        return is_error;

}