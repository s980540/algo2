#ifndef THREAD_H
#define THREAD_H

#include "global.h"

#include <pthread.h>
#include <unistd.h>
#include <time.h>

typedef struct _thread_info_t {
    pthread_t thread;
    int thread_id;
    long sleep_nsec;
    int argc;
    char **argv;
} thread_info_t;

#define THREAD_SLEEP_MSEC(msec) ((msec) * 1000000)

extern int thread_init(int argc, char *argv[]);
extern int thread_delete(void);

#endif // THREAD_H