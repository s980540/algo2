#include "global.h"
#include "thread.h"
#include "main.h"

#include "menu.h"
#include "menu_c_test.h"

static thread_info_t m_thread_info;

static menu_option_t *m_menu_opt;

void main_cpu0_unit_test(void)
{
    pthread_mutex_lock(&foo.mutex);
    printf("[Thread %d]: %d\n", m_thread_info.thread_id, foo.val);
    pthread_mutex_unlock(&foo.mutex);
}

void main_init(void)
{
    menu_register(&m_menu_opt, &menu_config_c_test);
}

void *main_cpu0(void *para)
{
    int ret;
    int argc = ((thread_info_t *)para)->argc;
    char **argv = ((thread_info_t *)para)->argv;

    const char *func_name = menu_get_prog_name(argv[0]);

    main_init();

    /* Initialize menu option module */
    ret = menu_opt_init(argc, argv, 1);
    if (ret != MENU_RET_SUCCESS) {
        printf("menu_opt_init failed (%d)\n", ret);
        goto exit;
    }

    ret = menu_opt_check(m_menu_opt);
    if (ret != MENU_RET_SUCCESS) {
        printf("menu_opt_check failed (%d)\n", ret);
        goto exit;
    }

    ret = menu_opt_process(argc, argv, func_name, m_menu_opt);
    if (ret != MENU_RET_SUCCESS) {
        if (ret != MENU_RET_EOF) {
            printf("menu_opt_process failed (%d)\n", ret);
            menu_func_help(func_name, m_menu_opt);
            goto exit;
        }
    }

exit:
    free(m_menu_opt);

    pthread_exit(NULL);
}

#if 0
void *main_cpu0(void *para)
{
    int ret;

    m_thread_info.thread = ((thread_info_t *)para)->thread;
    m_thread_info.thread_id = ((thread_info_t *)para)->thread_id;
    m_thread_info.sleep_nsec = ((thread_info_t *)para)->sleep_nsec;

    struct timespec request;
    request.tv_sec = 0;
    request.tv_nsec = m_thread_info.sleep_nsec;

    while (1) {
        main_cpu0_unit_test();

        if (m_thread_info.sleep_nsec) {
            ret = nanosleep(&request, NULL);
            if (ret == -1)
                printf("nanosleep error, errno=%d [%s]\n",
                    request.tv_nsec, errno, strerror(errno));
        }
    }

    pthread_exit(NULL);
}
#endif