#ifndef MUTEX_H
#define MUTEX_H

#include "global.h"
#include "pthread.h"

int mutex_init(pthread_mutex_t *mutex);
int mutex_lock(pthread_mutex_t *mutex);
int mutex_unlock(pthread_mutex_t *mutex);

#endif // ~ MUTEX_H