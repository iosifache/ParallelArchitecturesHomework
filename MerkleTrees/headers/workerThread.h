/*
 * Author: Cristian Chilipirea
 */
#ifndef _WORKER_THREAD_H

#define _WORKER_THREAD_H

char *outFile;
int P;
int difficulty;

void init();
void finalize();
void sendToWorker(char *data);
void *workerThread(void *var);

#endif