#ifndef _MENU_H
#define _MENU_H

#include <string.h>
#include <ctype.h>

#include "global.h"

#define CONFIG_MENU_OPT_DEBUG_MESSAGE   (1)
#define OCNFIG_MENU_DEBUG_ENABLE        (TRUE)

#define MENU_MAX_PROG_NAME_LEN          (40)
#define MENU_MAX_FUNC_NAME_LEN          (40)
#define MENU_FUNC_MAX_HELP_STR_LEN      (100)
#define MENU_OPT_MAX_HELP_STR_LEN       (100)
#define MENU_FUNC_MAX_HELP

#define GET_ARGUMENT(type) \
    *(type *)opt_get_arg()

#define MENU_OPT_ASSERT(cond)                               \
    do {                                                    \
        if (!(cond)) {                                      \
            printf("[%s][%s][%d] assertion failed!\n",      \
                   __FILE__, __FUNCTION__, __LINE__);      \
            while(1);                                       \
        }                                                   \
    } while(0)

#if (CONFIG_MENU_DEBUG_ENABLE)
    #define MENU_ASSERT(cond)   ASSERT(cond)
#else
    #define MENU_ASSERT(cond)
#endif

typedef enum _MENU_RET_CODE
{
    MENU_RET_SUCCESS = 0,
    MENU_RET_FAIL = 1,
    MENU_RET_INVALID_NAME,
    MENU_RET_INVALID_CODE,
    MENU_RET_INVALID_ARG_TYPE,
    MENU_RET_DUPLICATED_OPT,
    MENU_RET_INVALID_ARG,
    MENU_RET_INVALID_INDEX,
    MENU_RET_UNKOWN_CMD,
    MENU_RET_EOF,

    MENU_RET_CODE_COUNT
} MENU_RET_CODE;

typedef struct _menu_option_t
{
    const char *name;
    // int code;
    int arg_type;
    ret_code(*oper)(int argc, char **argv);
    const char *help_str;
    const char *usage;
} menu_option_t;

/**
 * @brief A convert function used to convert a input value to a desire result
 * @param r The final converted result
 * @param v The input value
 * @retval a void pointer point to result
 */
typedef void *(*arg_convert_func_t)(void *r, const void *v);

char *menu_get_prog_name(const char *s);
void get_name(char *buf, const char *s);
void *opt_get_arg(void);
ret_code menu_get_char(void);

// ret_code menu_process(int menu_func_code);

// ret_code menu_opt_init(int argc, char **argv, int i, const menu_option_t *menu_opt);
ret_code menu_register(menu_option_t **menu_opt, const menu_option_t *menu_config);
ret_code menu_opt_init(int argc, char **argv, int i);
ret_code menu_opt_check(const menu_option_t *menu_opt);
ret_code menu_func_help(const char *func_name, const menu_option_t *menu_opt);
void menu_opt_help(const char *func_name, const menu_option_t *menu_options2);
ret_code menu_opt_process(int argc, char **argv, const char *func_name, const menu_option_t *menu_opt);
ret_code menu_opt_init2(int argc, int argc_min, char **argv, const menu_option_t *menu_options2);
ret_code menu_get_opt_code(int *opt_code, const menu_option_t *menu_options2);

void *menu_get_arg(void *result, arg_convert_func_t convert);
void *menu_get_argv_inc(void *result, arg_convert_func_t convert);
void *menu_get_arg_idx(int index, void *result, arg_convert_func_t convert);
void *menu_get_arg_idx_inc(int index, void *result, arg_convert_func_t convert);

extern bool end_of_program;

#endif
