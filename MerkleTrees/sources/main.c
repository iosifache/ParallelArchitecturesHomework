/*
 * Author: Cristian Chilipirea
 */
#define _POSIX_C_SOURCE 200112L

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include "workerThread.h"

char **strings;
char **ids;
char *inFile;
pthread_barrier_t transmittersBarrier;
int N;
int transmitterThreads;

void getArgs(int argc, char **argv){

    if(argc < 4){
        printf("Not enough paramters: ./program inFile outFile P\n");
        exit(1);
    }

    inFile = argv[1];
    outFile = argv[2];
    P = atoi(argv[3]);

}

void *transmitterThread(void *var){

    int thread_id, start, end, i;

    thread_id = *(int *)var;
    start = N * thread_id / transmitterThreads;
    end = N * (thread_id+1) / transmitterThreads;

    pthread_barrier_wait(&transmittersBarrier);

    N = 0;
    for(i = start; i < end; i++){
        sendToWorker(ids[i]);
        sendToWorker(strings[i]);
        free(ids[i]); 
        free(strings[i]);
    }

    pthread_barrier_wait(&transmittersBarrier);

    if(thread_id == 0)
        sendToWorker("END");

    return NULL;

}

void readFile(){

    FILE *inF;
    int scanfRet, i;
    
    inF = fopen(inFile, "rt");
    scanfRet = fscanf(inF, "%i %i %i\n", &N, &transmitterThreads, &difficulty);
    if(scanfRet < 3){
        printf("Error #%i reading input file\n", errno);
        exit(errno);
    }

    strings = malloc(sizeof(char *) * N);
    ids = malloc(sizeof(char *) * N);
    for(i = 0; i < N; i++){
        strings[i] = malloc(sizeof(char) * (300 + 1));
        ids[i] = malloc(sizeof(char) * (20 + 1));
        scanfRet = fscanf(inF, "%s %s\n", ids[i], strings[i]);
        if(scanfRet < 2){
            printf("Error #%i reading input file\n", errno);
            exit(errno);
        }
    }

    fclose(inF);

}

int main(int argc, char **argv){

    getArgs(argc, argv);
    init();

    readFile();

    pthread_barrier_init(&transmittersBarrier, NULL, transmitterThreads);

    pthread_t tid[P + transmitterThreads];
    int thread_id[P + transmitterThreads];

    for(int i = 0; i < transmitterThreads; i++){
        thread_id[i] = i;
        pthread_create(&(tid[i]), NULL, transmitterThread, &(thread_id[i]));
    }

    for(int i = 0; i < P; i++){
        thread_id[transmitterThreads + i] = i;
        pthread_create(&(tid[transmitterThreads + i]), NULL, workerThread, &(thread_id[transmitterThreads + i]));
    }

    for(int i = 0; i < P + transmitterThreads; i++){
        pthread_join(tid[i], NULL);
    }

    pthread_barrier_destroy(&transmittersBarrier);

    finalize();

    free(ids);
    free(strings);

    return 0;

}