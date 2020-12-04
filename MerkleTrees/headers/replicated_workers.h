#ifndef _REPLICATED_WORKERS_H

#define _REPLICATED_WORKERS_H

#include <pthread.h>
#include <semaphore.h>

#define QUEUE_SIZE 1000000

extern int threads_count;
 
extern int got_tasks;
extern int put_tasks;

typedef struct task{
    pthread_mutex_t wait_until_finished;
    void (*task_to_run)(void *, int);
    void *data;
} task;

void init_workers();
void start_workers();
void put_task(task new_task);
void join_worker_threads();
void force_shutdown_workers();

#endif