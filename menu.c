// #include "string_match.h"

#include "menu.h"

/* Global Variables */
bool end_of_program = 0;

/* Static Function Declarations */
static const char *argtype2str(const menu_option_t *opt);

/* Static Variables */
static char m_char_buf[256];
static char *m_argv_buf[16];

// static const menu_option_t *_functions;
// static const menu_option_t _default_functions[] = {
//     {"help",    MENU_FUNC_HELP,         menu_func_help,         "Display this summary"},
//     {"string",  MENU_FUNC_STRING_MATCH, menu_func_string_match, "Demo string match algorithm"},
//     {NULL}
// };

// static const menu_option_t *_options;
// static const menu_option_t _default_options[] = {
//     {NULL}
// };

const static char menu_help_str[] = "--";
const static char menu_more_str[] = "---";

static char **m_arg_val = NULL;
// The total number of arguments
static int m_arg_max = 0;
static int m_arg_idx = 0;
static int m_arg_typ = 0;

// static char *m_param;
static int m_param_idx = 0;
static int m_para_cnt = 0;
static int _value;

static char _func_name[MENU_MAX_FUNC_NAME_LEN];


static int _memcnt = 0;
static int _config_cnt = 0;

ret_code menu_register(menu_option_t **menu_opt , const menu_option_t *menu_config)
{
    if (menu_opt == NULL)
        return MENU_RET_FAIL;

    if (_memcnt == 0) {
        _config_cnt = 1;
        _memcnt = 2;
        (*menu_opt) = malloc(sizeof(menu_option_t) * _memcnt);
        memcpy((*menu_opt), menu_config, sizeof(menu_option_t));
        memset((*menu_opt) + _config_cnt, 0, sizeof(menu_option_t) * (_memcnt - _config_cnt));
    } else {
        memcpy((*menu_opt) + _config_cnt++, menu_config, sizeof(menu_option_t));
    }

    if (_memcnt == _config_cnt) {

        if (_memcnt == (1UL << 31))
            return MENU_RET_FAIL;

        _memcnt *= 2;
        (*menu_opt) = realloc((*menu_opt), sizeof(menu_option_t) * _memcnt);
        memset((*menu_opt) + _config_cnt, 0, sizeof(menu_option_t) * (_memcnt - _config_cnt));
    }

    return MENU_RET_SUCCESS;
}

char *menu_get_prog_name(const char *s)
{
    size_t i, n;
    const char *p;
    char *q;

    /* Find the last '/', '\' or ':' */
    for (p = s + strlen(s); --p > s;) {
        if (*p == '/' || *p == '\\' || *p == ':') {
            p++;
            break;
        }
    }

    /* Strip off trailing nonsense. */
    n = strlen(p);
    if ((n > 4)
    && ((strcmp(&p[n - 4], ".exe") == 0) || (strcmp(&p[n - 4], ".EXE") == 0)))
        n -= 4;

    /* Copy over the prog name, in lowercae. */
    if (n > sizeof(_func_name) - 1)
        n = sizeof(_func_name) - 1;

    for (q = _func_name, i = 0; i < n; i++, p++)
        *q++ = isupper(*p) ? tolower(*p) : *p;

    *q = '\0';

    return(_func_name);
}

/**
  * @brief  Initialize menu option module
  * @param  argc
  * @param  argv
  * @param  i
  * @param  menu_opt
  * @retval None
  */
ret_code menu_opt_init(int argc, char **argv, int i)
{
    const menu_option_t *opt;

    if (argc < i)
        return MENU_RET_INVALID_ARG;

    /* Initialize related variable in this module */
    if (argc > 0)
        m_arg_max = argc;

    if (argv != NULL)
        m_arg_val = argv;

    if (i >= 0)
        m_arg_idx = i;

    m_arg_typ = 0;

    return MENU_RET_SUCCESS;
}

ret_code menu_opt_check(const menu_option_t *menu_opt)
{
    const menu_option_t *opt;

    /* Check if option name is legit */
    for (opt = menu_opt; opt->name; opt++) {
        const menu_option_t *next_opt;

        /* Make sure opt name is legit */
        if (opt->name[0] == '\0') {
            return MENU_RET_INVALID_NAME;
        }

        /* Make sure there are no duplicates. */
        for (next_opt = opt + 1; next_opt->name; next_opt++) {
            if (strcmp(opt->name, next_opt->name) == 0) {
                return MENU_RET_DUPLICATED_OPT;
            }
        }
    }

    return MENU_RET_SUCCESS;
}

/**
  * @brief  Initialize menu option module
  * @param  prog_name
  * @param  argc
  * @param  argv
  * @retval None
  */
ret_code menu_opt_init2(
    int argc,
    int argc_min,
    char **argv,
    const menu_option_t *menu_opt)
{
    const menu_option_t *opt;

    if (argc < argc_min)
        return MENU_RET_INVALID_ARG;

    /* Initialize related variable in this module */
    m_arg_max = argc;
    m_arg_val = argv;
    m_arg_idx = 2;
    m_arg_typ = 0;

    // if (menu_opt != NULL) {
    //     _options = menu_opt;
    // } else {
    //     _options = _default_options;
    // }

    /* Check if option name is legit */
    for (opt = menu_opt; opt->name; opt++) {
        const menu_option_t *next_opt;

        /* Make sure opt name is legit */
        if (opt->name[0] == '\0') {
            return MENU_RET_INVALID_NAME;
        }

        // /* Check if code is correct */
        // if (opt->code <= 0
        // ||  opt->code >= MENU_FUNC_CODE_COUNT) {
        //     return MENU_RET_INVALID_CODE;
        // }

        /* Check if arg type is correct */
        switch (opt->arg_type) {
        case 0:
        case '-':
        case 'a':
        case 'r':
        case 'g':
        case '/':
        case '<':
        case '>':
        case 'E':
        case 'F':
        case 'M':
        case 'U':
        case 'f':
        case 'l':
        case 'n':
        case 'p':
        case 's':
        case 'u':
        case 'c':
            break;

        default:
            return MENU_RET_INVALID_ARG_TYPE;
        }

        /* Make sure there are no duplicates. */
        for (next_opt = opt + 1; next_opt->name; next_opt++) {
            printf("menu_opt_init2 %s %s\n", opt->name, next_opt->name);
            if (strcmp(opt->name, next_opt->name) == 0) {
                return MENU_RET_DUPLICATED_OPT;
            }
        }
    }

    return MENU_RET_SUCCESS;
}

/**
  * @brief  Get user's input from console
  * @param  None
  * @retval None
  */
ret_code menu_get_char(void)
{
    int i;
    char *p, c;

    printf("Algo> ");

    /* Get user's input from console */
    for (i = 0;;) {
        c = getchar();

        /* c == 10, 0xA, LF, line feed */
        if (c == 10) {
            m_char_buf[i] = '\0';
            break;
        }
        /* If the user press Ctrl+c to end the execution of the program */
        else if (c == EOF) {
            // end_of_program = 1;
            // debug
            printf("Ctrl + C!\n");
            return MENU_RET_EOF;
        } else {
            m_char_buf[i++] = c;
        }
    }

    /* Convert string from console to _argv_buf[] */
    for (i = 0, p = m_char_buf, m_arg_val = m_argv_buf; i < 16; ) {

        if (p == NULL || *p == '\0') {
            break;
        }

        /* Skip all space before the next command */
        while (*p == ' ')
            p++;

        m_argv_buf[i] = p;

        if ((p = strchr(p, ' ')) != NULL)
            *p++ = '\0';

        i++;
    }

    /* Clean up next cmd buffer to indicate no more cmd here */
    if (i < 16)
        m_argv_buf[i] = NULL;

    /*
     * Because the first element of _argv_buf is not the prog name,
     * so we set m_arg_idx to 0.
     */
    m_arg_idx = 0;

    return MENU_RET_SUCCESS;
}

ret_code menu_opt_process(
    int argc,
    char **argv,
    const char *func_name,
    const menu_option_t *menu_opt)
{
    MENU_RET_CODE ret;

    // if (argc < 2)
    //     return MENU_RET_INVALID_ARG;

    char *arg_val = menu_get_argv_inc(NULL, NULL);
    if (arg_val == NULL)
        return MENU_RET_INVALID_ARG;

#if (CONFIG_MENU_OPT_DEBUG_MESSAGE)
    printf("m_arg_val ::%s\n", arg_val, menu_opt->name);
#endif

    if ((strcmp(arg_val, "help") == 0) || (strcmp(arg_val, "--help") == 0)) {
        menu_func_help(func_name, menu_opt);
        return MENU_RET_SUCCESS;
    } else {
        for (; menu_opt->name; menu_opt++) {
            /* If not this option, move on to the next one. */
            if (strcmp(arg_val, menu_opt->name) != 0)
                continue;

            /* Excute associate oper */
            if (menu_opt->oper)
                ret = menu_opt->oper(argc, argv);

            return ret;
        }
    }

    return MENU_RET_UNKOWN_CMD;
}

ret_code menu_get_opt_code(
    int *opt_code,
    const menu_option_t *menu_opt
    )
{
//     const menu_option_t *opt;
//     char *argv;
//     int ival;
//     long lval;
//     unsigned long ulval;

//     // m_param = NULL;
//     // m_param = 0;
//     m_para_cnt = 0;

//     /* Look at current arg; at the end of the list? */
//     if ((argv = m_arg_val[m_arg_idx++]) == NULL) {
//         return MENU_RET_EOF;
//     }

// #if (CONFIG_MENU_OPT_DEBUG_MESSAGE)
//     /* Use for debugging */
//     printf("m_arg_val ::%s\n", argv);
// #endif

//     // /* If word doesn't start with a -, we're done. */
//     // if (*p != '-' && *p != '-') {
//     //     return MENU_RET_EOF;
//     // }

//     // /* Hit "--"? We're done. */
//     // if (strcmp(p, "--") == 0 || strcmp(p, "##") == 0) {
//     //     return MENU_RET_EOF;
//     // }

//     // /* Allow -mmm and --mmm */
//     // if (*++p == '-' || *p == '-') {
//     //     p++;
//     // }

//     // /* If we have --flag=foo, snip it off. */
//     // if ((m_param = strchr(p, '=')) != NULL)
//     //     *m_param++ = '\0';

//     for (opt = menu_opt; opt->name; opt++) {
//         /* If not this option, move on to the next one. */
//         if (strcmp(argv, opt->name) != 0)
//             continue;

//         /* If it doesn't take a value, make sure none was given. */
//         if (opt->arg_type == 0 || opt->arg_type == '-') {
//             // if (m_param)
//             //     return MENU_RET_INVALID_ARG_TYPE;

//             *opt_code = opt->code;
//             m_arg_typ = 0;

//             return MENU_RET_SUCCESS;
//         }

//         /* Want a value; get the next param if =foo not use. */
//         // if (m_param == NULL) {
//             if (m_arg_val[m_arg_idx] == NULL)
//                 return MENU_RET_INVALID_ARG;
//             // else
//             //     m_param_idx = m_arg_idx++;
//                 // m_param = m_arg_val[m_arg_idx++];
//         // }

//         /* Syntax-check value. */
//         switch (opt->arg_type) {
//         default:
//         case 's':
//         case 'a':
//             m_arg_typ = opt->arg_type;
//             m_para_cnt = 1;
//             break;

//         case 'r':
//             m_arg_typ = opt->arg_type;
//             m_para_cnt = 2;
//             break;

//         case 'g':
//             m_arg_typ = opt->arg_type;
//             m_para_cnt = 3;
//             break;

//         case '/':
//         case '>':
//         case 'p':
//         case 'n':
//         case 'M':
//         case 'U':
//         case 'l':
//         case 'u':
//         case 'c':
//         case 'E':
//         case 'F':
//         case 'f':
//             return MENU_RET_INVALID_ARG_TYPE;
//         }

//         *opt_code = opt->code;

//         return MENU_RET_SUCCESS;
//     }

    return MENU_RET_UNKOWN_CMD;
}

ret_code menu_func_help(
    const char *func_name,
    const menu_option_t *menu_opt)
{
    const menu_option_t *opt;
    int i;
    int w = 5;
    char s_buf[80 + 1];
    const char *help_str;
    char *p;

    for (opt = menu_opt; opt->name; opt++) {
        i = 2 + strlen(opt->name);

        if (opt->arg_type != '-')
            i += 1 + strlen(argtype2str(opt));

        if (i < MENU_FUNC_MAX_HELP_STR_LEN && i > w) {
            w = i;
        }
    }

    printf("\nUsage: %s [options]\n\nValid options are:\n",
            func_name ? func_name : "");

    for (opt = menu_opt; opt->name; opt++) {
        help_str = opt->help_str ? opt->help_str : "(No additional info)";

        /* Pad out prefix */
        memset(s_buf, ' ', sizeof(s_buf) - 1);
        s_buf[sizeof(s_buf) - 1] = '\0';

        /* Build up the "-flag [param]" part. */
        p = s_buf;
        *p++ = ' ';
        // *p++ = '-';

        if (opt->name[0])
            p += strlen(strcpy(p, opt->name));
        else
            *p++ = '*';

        if (opt->arg_type != '-') {
            *p++ = ' ';
            p += strlen(strcpy(p, argtype2str(opt)));
        }

        *p = ' ';

        // if ((int)(p - s_buf) >= MENU_OPT_MAX_HELP_STR_LEN) {
        //     *p = '\0';
        //     printf("%s\n", s_buf);
        //     memset(s_buf, ' ', sizeof(s_buf));
        // }

        s_buf[w] = '\0';
        printf("%s  %s\n", s_buf, help_str);
    }

    printf("\n");

    return MENU_RET_SUCCESS;
}

static const char *argtype2str(const menu_option_t *opt)
{
    switch (opt->arg_type) {
    case 0:
    case '-':
        return("");

    case 'a':
        return("val");

    case '/':
        return("dentry");

    case '<':
        return("infile");

    case '>':
        return("outfile");

    case 'p':
        return("+int");

    case 'n':
        return("int");

    case 'l':
        return("long");

    case 'u':
        return("ulong");

    case 'E':
        return("PEM|DER|ENGINE");

    case 'F':
        return("PEM|DER");

    case 'f':
        return("format");

    case 'M':
        return("intmax");

    case 'U':
        return("uintmax");
    }

    return("[args]");
}

void *menu_get_arg(void *result, arg_convert_func_t convert)
{
    if (m_arg_idx >= m_arg_max) {
        result = NULL;
        return NULL;
    }

    if (convert != NULL) {
        return convert(result, m_arg_val[m_arg_idx]);
    } else {
        if (result != NULL)
            result = m_arg_val[m_arg_idx];

        return m_arg_val[m_arg_idx];
    }
}

void *menu_get_argv_inc(void *result, arg_convert_func_t convert)
{
    if (m_arg_idx >= m_arg_max) {
        result = NULL;
        return NULL;
    }

    if (convert != NULL)
        return convert(result, m_arg_val[m_arg_idx++]);
    else {
        if (result != NULL)
            result = m_arg_val[m_arg_idx];

        return m_arg_val[m_arg_idx++];
    }
}

/**
 * @brief Using convert function to convert a selected argument to a desire result
 * @param index The index of selected argument
 * @param result The output of convert function
 * @param convert The convert function
 * @retval a void pointer point to result
 */
void *menu_get_arg_idx(int index, void *result, arg_convert_func_t convert)
{
    if (/*(index >= m_para_cnt) || */((m_param_idx + index) > m_arg_max)) {
        result = NULL;
        return NULL;
    }

    if (convert != NULL)
        return convert(result, m_arg_val[m_param_idx + index]);
    else {
        if (result != NULL)
            result = m_arg_val[m_param_idx + index];

        return m_arg_val[m_param_idx + index];
    }
}

/**
 * @brief Using convert function to convert a selected argument to a desire result
 * @param index The index of selected argument
 * @param result The output of convert function
 * @param convert The convert function
 * @retval a void pointer point to result
 */
void *menu_get_arg_idx_inc(int index, void *result, arg_convert_func_t convert)
{
    if (/*(index >= m_para_cnt) || */((m_arg_idx + index) > m_arg_max)) {
        result = NULL;
        return NULL;
    }

    if (convert != NULL)
        return convert(result, m_arg_val[m_arg_idx++ + index]);
    else {
        if (result != NULL)
            result = m_arg_val[m_arg_idx + index];

        return m_arg_val[m_arg_idx++ + index];
    }
}
