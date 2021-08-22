#include "../common/global_aliases.h"
#include "../common/version.h"

void fun_lgetbyte(void) {
    getargs(&ep, 3, ",");

    if (argc != 3) error("Argument count");

    void *pvar = findvar(argv[0], V_FIND | V_EMPTY_OK);
    char *s = NULL;
    if (g_var_tbl[g_current_var_idx ].type & T_INT) {
        if (g_var_tbl[g_current_var_idx ].dims[1] != 0) error("Invalid variable");
        if (g_var_tbl[g_current_var_idx ].dims[0] <= 0) {  // Not an array
            error("Argument 1 must be integer array");
        }

        int64_t *src = (int64_t *)pvar;
        s = (char *)&src[1];
    } else {
        error("Argument 1 must be integer array");
    }

    int j = (g_var_tbl[g_current_var_idx ].dims[0] - OptionBase) * 8 - 1;
    int start = getint(argv[2], OptionBase, j - OptionBase);
    g_integer_return = s[start - OptionBase];
    g_return_type = T_INT;
}
