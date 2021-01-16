#ifndef _MULTIPROCESSING_H

#define _MULTIPROCESSING_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/*
 * Makes a process wait until a debugger is attached to it and modifies a
 * variable value
 */
void wait_until_debugger_attached(){

    char hostname[256];
    volatile int i = 0;

    gethostname(hostname, sizeof(hostname));
    printf("PID %d on %s ready for attach\n", getpid(), hostname);
    fflush(stdout);
    while (0 == i)
        sleep(5);

}

#endif