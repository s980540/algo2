#include "menu_c_test.h"

ret_code run_c_test(int argc, char **argv)
{
    c_test();

    return MENU_RET_SUCCESS;
}

static const menu_option_t m_memu_opt[] =
{
    {"--help",                      '-',    NULL,                       "Display this summary.",},
    {"--run",                       '-',    run_c_test,                 "Run C test.",},

    {NULL}
};

ret_code menu_func_c_test(int argc, char **argv)
{
    int ret;
    const char *func_name = menu_config_c_test.name;

    ret = menu_opt_check(m_memu_opt);
    if (ret != MENU_RET_SUCCESS) {
        printf("menu_opt_check failed (%d)\n", ret);
        exit(ret);
    }

    ret = menu_opt_process(argc, argv, func_name, m_memu_opt);
    if (ret != MENU_RET_SUCCESS) {
        if (ret != MENU_RET_EOF) {
            printf("menu_opt_process failed (%d)\n", ret);
            menu_func_help(func_name, m_memu_opt);
            exit(ret);
        }
    }

    return ret;
}

const menu_option_t  menu_config_c_test =
    {"c-test",  's',    menu_func_c_test,   "Test different functions"};
