#include "thread.h"

extern void *main_cpu0(void *para);
extern void *main_cpu1(void *para);
extern void *main_cpu2(void *para);
extern void *main_cpu3(void *para);

thread_info_t *m_thread_info_cpu0;
thread_info_t *m_thread_info_cpu1;
thread_info_t *m_thread_info_cpu2;
thread_info_t *m_thread_info_cpu3;

int thread_init(int argc, char *argv[])
{
    m_thread_info_cpu0 = malloc(sizeof(thread_info_t));
    m_thread_info_cpu0->thread_id = 0;
    m_thread_info_cpu0->sleep_nsec = THREAD_SLEEP_MSEC(500);

    m_thread_info_cpu0->argc = argc;
    m_thread_info_cpu0->argv = argv;
    pthread_create(&m_thread_info_cpu0->thread, NULL, main_cpu0, m_thread_info_cpu0);

    // m_thread_info_cpu1 = malloc(sizeof(thread_info_t));
    // m_thread_info_cpu1->thread_id = 1;
    // m_thread_info_cpu1->sleep_nsec = THREAD_SLEEP_MSEC(10);
    // pthread_create(&m_thread_info_cpu1->thread, NULL, main_cpu1, m_thread_info_cpu1);

    // m_thread_info_cpu2 = malloc(sizeof(thread_info_t));
    // m_thread_info_cpu2->thread_id = 2;
    // m_thread_info_cpu2->sleep_nsec = THREAD_SLEEP_MSEC(300);
    // pthread_create(&m_thread_info_cpu2->thread, NULL, main_cpu2, m_thread_info_cpu2);

    // m_thread_info_cpu3 = malloc(sizeof(thread_info_t));
    // m_thread_info_cpu3->thread_id = 3;
    // m_thread_info_cpu3->sleep_nsec = THREAD_SLEEP_MSEC(500);
    // pthread_create(&m_thread_info_cpu3->thread, NULL, main_cpu3, m_thread_info_cpu3);

    pthread_join(m_thread_info_cpu0->thread, NULL);
    // pthread_join(m_thread_info_cpu1->thread, NULL);
    // pthread_join(m_thread_info_cpu2->thread, NULL);
    // pthread_join(m_thread_info_cpu3->thread, NULL);

    return 0;
}

int thread_delete(void)
{
    free(m_thread_info_cpu0);
    free(m_thread_info_cpu1);
    free(m_thread_info_cpu2);
    free(m_thread_info_cpu3);

    return 0;
}