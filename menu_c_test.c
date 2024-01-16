#include "menu_c_test.h"

ret_code run_c_test(int argc, char **argv)
{
    c_test();

    return MENU_RET_SUCCESS;
}

extern int crc32_main(char *data);
ret_code run_crc32(int argc, char **argv)
{
    if (argc < 4) {
        printf("Usage: %s \"some string\"\n", argv[0]);
        return -1;
    }

    return crc32_main(argv[3]);

}

extern int crc_table_main(void);
ret_code run_crc_table(int argc, char **argv)
{
    crc_table_main();
    return 0;
}

static const menu_option_t m_memu_opt[] =
{
    {"--help",                      '-',    NULL,                       "Display this summary.",},
    {"--run",                       '-',    run_c_test,                 "Run C test.",},
    {"--crc32",                     '-',    run_crc32,                  "Run CRC32 test.",},
    {"--crc-table",                 '-',    run_crc_table,              "Run CRC table.",},

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
