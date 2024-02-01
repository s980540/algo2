#include "menu_c_test.h"
#include "crc32.h"

ret_code run_c_test(int argc, char **argv)
{
    c_test();

    return MENU_RET_SUCCESS;
}

ret_code run_crc32(int argc, char **argv)
{
    char mic_string[] = "123456789";
    char mic_test1[32];
    char mic_test2[32];
    char mic_test3[30];
    char mic_test4[32];

    // if (argc < 4) {
    //     printf("Usage: %s \"some string\"\n", argv[0]);
    //     return -1;
    // }

    // return crc32(argv[3]);
    int length;

    length = sizeof(mic_string) - 1;
    crc32(mic_string, length);

    length = sizeof(mic_test1);
    memset(mic_test1, 0x00, length);
    crc32(mic_test1, length);

    length = sizeof(mic_test2);
    memset(mic_test2, 0xFF, length);
    crc32(mic_test2, length);

    length = sizeof(mic_test3);
    for (int i = 0; i < length; i++) {
        mic_test3[i] = i;
        // printf("%2x ", mic_test3[i]);
    }
    // printf("\n");
    crc32(mic_test3, length);

    length = sizeof(mic_test4);
    for (int i = 0; i < length; i++) {
        mic_test4[i] = length - i - 1;
    }
    crc32(mic_test4, length);
}

extern int crc_table_main(void);
ret_code run_crc_table(int argc, char **argv)
{
    crc_table_main();
    return 0;
}

static const menu_option_t m_memu_opt[] = {
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
