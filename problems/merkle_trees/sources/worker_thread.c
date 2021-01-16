#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include <openssl/md5.h>
#include "../../helpers/CCookbook/multitasking/replicated_workers.h"
#include "../../helpers/CCookbook/strings/characters.h"
#include "../../helpers/CCookbook/miscellaneous/data_types.h"
#include "../../helpers/CCookbook/miscellaneous/conditon_checkers.h"
#include "../../helpers/CCookbook/miscellaneous/logger.h"

#define INITIAL_LEVELS 20
#define MD5_BYTES_COUNT 16
#define UINT_MAX_DIGITS 10
#define END_SYMBOL "END"
#define HASHES_SEPARATOR "#"
#define HASH_STARTER_CHAR '0'
#define FINAL_HASH_PREFIX "Final Hash "

typedef enum node_type {
    NOT_INITED = 0,
    BASIC = 1,
    WITH_RAW_DATA = 1,
    LIFTED = 2,
    SHALLOW = 3
} node_type;

typedef struct node{
    uint id;
    char *string;
    uint salt;
    struct node *next;
    node_type type;
} node;

typedef struct mining_data{
    node *first_node;
    node *second_node;
    uint level;
} mining_data;

extern char *outFile;
extern int P;
extern int difficulty;

node **levels = NULL;
pthread_mutex_t *levels_mutexes;
pthread_mutex_t transmitters_mutex;
uint new_id = UINT_MAX;
char *new_string = NULL;
int locks_count = 0;
int raw_data_nodes_count = 0;
uint tree_height = UINT_MAX;

void mine_hash(void *data, int thread_id);

int compare_nodes(node *first, node *second){

    return (first->id - second->id);

}

void init(){

    pthread_mutexattr_t attributes;
    int is_error = 0, level_elements_count, i, j;

    // Initialize recursive mutex
    pthread_mutexattr_init(&attributes);
    pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&transmitters_mutex, &attributes);

    // Create array of arrays, one for each layer (dynamic arrays, with size proportional to tree height, could be used here), and required mutexes
    levels = (node **)malloc(INITIAL_LEVELS * sizeof(node *));
    GOTO_CONDITION_CHECKER(levels == NULL, is_error, ERROR_INIT_1);
    levels_mutexes = (pthread_mutex_t *)malloc(INITIAL_LEVELS * sizeof(pthread_mutex_t));
    GOTO_CONDITION_CHECKER(levels_mutexes == NULL, is_error, ERROR_INIT_2);
    for (i = 0; i < INITIAL_LEVELS; i++){

        level_elements_count = pow(2, INITIAL_LEVELS - i);
        levels[i] = calloc(level_elements_count, sizeof(node));
        GOTO_CONDITION_CHECKER(levels[i] == NULL, is_error, ERROR_INIT_3);

        pthread_mutex_init(&levels_mutexes[i], NULL);

    }

    // Init replicated workers
    init_workers();

    ERROR_INIT_3:
        for (j = 0; j < i; j++)
            free(levels[i]);
    ERROR_INIT_2:
        if (is_error)
            free(levels);
    ERROR_INIT_1:
        return;

}

void finalize(){

    int level_elements_count, i, j;

    // Destroy recursive mutex
    pthread_mutex_destroy(&transmitters_mutex);
 
    // Free memory
    for (i = 0; i < INITIAL_LEVELS; i++){

        level_elements_count = pow(2, INITIAL_LEVELS - i);
        for (j = 0; j < level_elements_count; j++)
            if (levels[i][j].type != SHALLOW)
                free(levels[i][j].string);

        free(levels[i]);

        pthread_mutex_destroy(&levels_mutexes[i]);

    }
    free(levels_mutexes);
    free(levels);

}

char *compute_md5(char* string){

    unsigned char digest[16];
    char *hash = NULL;
    int is_error = 0;

    // Allocate memory
    hash = (char *)calloc(2 * MD5_BYTES_COUNT + 1, 1);
    GOTO_CONDITION_CHECKER(hash == NULL, is_error, EXIT_MD5);

    // Initialize MD5 context and compute hash
    MD5_CTX context;
    MD5_Init(&context);
    MD5_Update(&context, string, strlen(string));
    MD5_Final(digest, &context);

    // Dump hexadecimal representation of hash in buffer
    for(int i = 0; i < MD5_BYTES_COUNT; i ++)
        sprintf(&hash[i * 2], "%02x", (unsigned int)digest[i]);

    EXIT_MD5:
        return hash;

}

int dump_levels_to_file(){

    FILE *output_file;
    int is_error = 0, i, j;

    output_file = fopen(outFile, "w");
    GOTO_CONDITION_CHECKER(output_file == NULL, is_error, EXIT_DUMP_LEVELS_TO_FILE);

    // Write all salts and the final hash to file
    for (i = 1; i <= tree_height; i++){
        for (j = 0; j < pow(2, tree_height - i); j++){
            if (levels[i][j].type == BASIC)
                fprintf(output_file, "%d ", levels[i][j].salt);
        }
        fprintf(output_file, "\n");
    }
    fprintf(output_file, FINAL_HASH_PREFIX "%s\n", levels[tree_height][0].string);

    fclose(output_file);

    EXIT_DUMP_LEVELS_TO_FILE:
        return is_error;

}

task create_task(node *first_node, node *second_node, uint level){

    task new_task;
    mining_data *mined_data = NULL;
    int is_error;

    // Allocate memory
    mined_data = (mining_data *)malloc(sizeof(mining_data));
    GOTO_CONDITION_CHECKER(mined_data == NULL, is_error, EXIT_CREATE_TASK);

    // Initialize members of new task
    mined_data->first_node = first_node;
    mined_data->second_node = second_node;
    mined_data->level = level;
    new_task.data = mined_data;
    new_task.task_to_run = &mine_hash;

    EXIT_CREATE_TASK:
        return new_task;

}

int create_new_node(uint id, char *string, uint level, uint salt, node_type type){

    node *node_cursor = NULL;
    node new_node;
    uint searched_id;
    int is_error = 0;

    // Allocate memory and initialize members of new node
    if (type == SHALLOW || type == WITH_RAW_DATA)
        new_node.string = string;
    else if (string != NULL){

        new_node.string =(char *)malloc((strlen(string) + 1) * sizeof(char));
        GOTO_CONDITION_CHECKER(new_node.string == NULL, is_error, EXIT_CREATE_NEW_NODE);

        strcpy(new_node.string, string);

        // If the node is basic, then the string is the MD5 hash and it can be freed
        if (type == BASIC)
            free(string);

    }
    new_node.id = id;
    new_node.salt = salt;
    new_node.next = NULL;
    new_node.type = type;

    // Check if the node can be paired with an already inserted one and insert in the list corresponding to its level
    searched_id = (new_node.id % 2 == 0) ? new_node.id + 1 : new_node.id - 1;
    pthread_mutex_lock(&levels_mutexes[level]);
    levels[level][id] = new_node;
    node_cursor = &levels[level][searched_id];
    if (node_cursor->type != NOT_INITED){
        pthread_mutex_unlock(&levels_mutexes[level]);
        if (new_node.id % 2 == 0)
            put_task(create_task(&levels[level][id], node_cursor, level));
        else
            put_task(create_task(node_cursor, &levels[level][id], level));
    }
    else{
        pthread_mutex_unlock(&levels_mutexes[level]);
    }

    // If the root is reached, then dump to file and shutdown replicated workers
    if (level == tree_height){
        dump_levels_to_file();
        force_shutdown_workers();
    }

    if (is_error == 1 && new_node.string != NULL)
        free(new_node.string);
    EXIT_CREATE_NEW_NODE:
        return is_error;

}

void mine_hash(void *data, int thread_id){

    mining_data *mined_data = NULL;
    char *current_buffer;
    char *computed_hash;
    uint current_salt = 0;
    int is_error = 0, is_valid, buffer_length, i;

    mined_data = (mining_data *)data;

    // Check if both nodes are shallow
    if (mined_data->first_node->type == SHALLOW && mined_data->second_node->type == SHALLOW){
        create_new_node(mined_data->first_node->id / 2, NULL, mined_data->level + 1, 0, SHALLOW);
        goto EXIT_MINE_HASH;
    }

    // Check if one node is shallow
    if (mined_data->second_node->type == SHALLOW){
        create_new_node(mined_data->first_node->id / 2, mined_data->first_node->string, mined_data->level + 1, 0, LIFTED);
        goto EXIT_MINE_HASH;
    }

    // Allocate memory for buffer (two strings, one unsigned int, two separators and one null character)
    buffer_length = strlen(mined_data->first_node->string) + strlen(mined_data->second_node->string) + UINT_MAX_DIGITS + 3;
    current_buffer = (char *)malloc(buffer_length * sizeof(char));
    GOTO_CONDITION_CHECKER(current_buffer == NULL, is_error, EXIT_MINE_HASH);
    
    // Initialize buffer
    strcpy(current_buffer, mined_data->first_node->string);
    strcat(current_buffer, HASHES_SEPARATOR);
    strcat(current_buffer, mined_data->second_node->string);
    strcat(current_buffer, HASHES_SEPARATOR);

    while (current_salt < UINT_MAX){

        // Remove last salt and append the current one
        current_buffer[buffer_length - UINT_MAX_DIGITS - 1] = '\0';
        sprintf(current_buffer + buffer_length - UINT_MAX_DIGITS - 1, "%u", current_salt);
        computed_hash = compute_md5(current_buffer);

        // Check if the hash corresponds to complexity
        is_valid = 1;
        if (difficulty > 0)
            for (i = 0; i < difficulty; i++)
                if (computed_hash[i] != HASH_STARTER_CHAR)
                    is_valid = 0;

        if (is_valid == 1){

            // If hash is valid, then insert new node in tree
            create_new_node(mined_data->first_node->id / 2, computed_hash, mined_data->level + 1, current_salt, BASIC);
            break;

        }
        else{
            free(computed_hash);
            current_salt++;
        }

    }

    free(current_buffer);
    EXIT_MINE_HASH:
        free(mined_data);

}

void sendToWorker(char *data){

    int was_inited_now = 0, is_error = 0, all_nodes_count, i, ret_val;

    pthread_mutex_lock(&transmitters_mutex);

    // Detect if data is the end symbol
    if (strcmp(data, END_SYMBOL) == 0){

        // Compute tree heigth based on number of leafs
        tree_height = ceil(log2(raw_data_nodes_count));

        // Insert required shallow nodes
        all_nodes_count = pow(2, tree_height);
        int j = 0;
        for (i = raw_data_nodes_count; i < all_nodes_count; i++){
            j++;
            create_new_node(i, NULL, 0, 0, SHALLOW);
        }

    }
    else{

        // Check if new node needs to be allocated
        if (new_id == UINT_MAX && new_string == NULL)
            was_inited_now = 1;

        // Detect if data is an id or information
        if (CHECK_IF_LOWERCASE_LETTER(data[0])){
            new_string =(char *)malloc((strlen(data) + 1) * sizeof(char));
            GOTO_CONDITION_CHECKER(new_string == NULL, is_error, EXIT_SENDTOWORKER);
            strcpy(new_string, data);
        }
        else if (CHECK_IF_NUMBER(data[0]))
            new_id = atoi(data);

        // Check if the node can be entered in the list
        if (was_inited_now == 0){
            ret_val = create_new_node(new_id, new_string, 0, 0, WITH_RAW_DATA);
            new_id = UINT_MAX;
            new_string = NULL;
            raw_data_nodes_count++;
        }

    }

    // Check if the recursive mutex needs to be restored
    if (locks_count == 0)
        locks_count++;
    else{
        locks_count = 0;
        pthread_mutex_unlock(&transmitters_mutex);
        pthread_mutex_unlock(&transmitters_mutex);
    }

    EXIT_SENDTOWORKER:
        return;

}