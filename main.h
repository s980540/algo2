#ifndef MAIN_H
#define MAIN_H

#include "global.h"

#include <pthread.h>

typedef struct _foo_ts
{
    pthread_mutex_t mutex;
    int val;
} foo_ts;

extern foo_ts foo;

extern void *main_cpu0(void *para);
extern void *main_cpu1(void *para);
extern void *main_cpu2(void *para);
extern void *main_cpu3(void *para);

#endif // MAIN_H