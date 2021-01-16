#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>
#include "replicated_workers.h"

typedef struct tasks{
    int shutting_down;
    sem_t sem_full;
    sem_t sem_empty;
    pthread_mutex_t mutex;
    task tasks_queue[QUEUE_SIZE];
    int get_index;
    int put_index;
} tasks;

tasks tasks_manager;
pthread_t *tid;
int threads_count = 8;
int got_tasks = 0;
int put_tasks = 0;

void init_workers(){

    sem_init(&(tasks_manager.sem_full), 0, 0);
    sem_init(&(tasks_manager.sem_empty), 0, QUEUE_SIZE);
    pthread_mutex_init(&(tasks_manager.mutex), NULL);

    tasks_manager.put_index = 0;
    tasks_manager.get_index = 0;
    tasks_manager.shutting_down = 0;

}

task get_task(){

    task extracted_task;

    sem_wait(&(tasks_manager.sem_full));

    pthread_mutex_lock(&(tasks_manager.mutex));
        extracted_task = tasks_manager.tasks_queue[tasks_manager.get_index];
        tasks_manager.get_index++;
        tasks_manager.get_index %= QUEUE_SIZE;
        got_tasks++;
    pthread_mutex_unlock(&(tasks_manager.mutex));

    sem_post(&(tasks_manager.sem_empty));

    return extracted_task;

}
void put_task(task task){

    sem_wait(&(tasks_manager.sem_empty));

    pthread_mutex_lock(&(tasks_manager.mutex));
        tasks_manager.tasks_queue[tasks_manager.put_index] = task;
        tasks_manager.put_index++;
        tasks_manager.put_index %= QUEUE_SIZE;
        put_tasks++;
    pthread_mutex_unlock(&(tasks_manager.mutex));

    sem_post(&(tasks_manager.sem_full));

}

void *threaded_worker(void *argument){

    task extracted_task;
    int thread_id = *((int *)argument);

    while(1){
        extracted_task = get_task();
        if(tasks_manager.shutting_down)
            break;
        extracted_task.task_to_run(extracted_task.data, thread_id);
    }

    return NULL;

}

void start_workers(){

    int *thread_id;
    int i;

    sem_init(&(tasks_manager.sem_full), 0, 0);
    sem_init(&(tasks_manager.sem_empty), 0, QUEUE_SIZE);
    pthread_mutex_init(&(tasks_manager.mutex), NULL);

    tasks_manager.put_index = 0;
    tasks_manager.get_index = 0;
    tasks_manager.shutting_down = 0;

    tid = (pthread_t *)malloc(threads_count * sizeof(pthread_t));
    thread_id = (int *)malloc(threads_count * sizeof(int));
    for(i = 0;i < threads_count; i++)
        thread_id[i] = i;
    for(i = 0; i < threads_count; i++)
        pthread_create(&(tid[i]), NULL, threaded_worker, &(thread_id[i]));

}

void force_shutdown_workers(){

    int i;

    pthread_mutex_lock(&(tasks_manager.mutex));
        tasks_manager.shutting_down = 1;
        for(i = 0; i < threads_count; i++){
            sem_post(&(tasks_manager.sem_full));
        }
    pthread_mutex_unlock(&tasks_manager.mutex);

}