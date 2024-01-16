#include "mutex.h"

int mutex_init(pthread_mutex_t *mutex)
{
    int sts;

    sts = pthread_mutex_init(mutex, NULL);
    if (sts) {
        printf("pthread_mutex_init error: %d\n", sts);
    }

    return sts;
}

int mutex_lock(pthread_mutex_t *mutex)
{
    int sts;
    sts = pthread_mutex_lock(mutex);
    if (sts) {
        printf("pthread_mutex_lock error: %d\n", sts);
        return sts;
    }

    return 0;
}

int mutex_unlock(pthread_mutex_t *mutex)
{
    int sts;
    sts = pthread_mutex_unlock(mutex);
    if (sts) {
        printf("pthread_mutex_unlock error: %d\n", sts);
        return sts;
    }

    return 0;
}