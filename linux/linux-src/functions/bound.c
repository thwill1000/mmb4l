#include "../common/global_aliases.h"
#include "../common/version.h"

void fun_bound(void) {
    getargs(&ep, 3, ",");
    int which = (argc == 3) ? getint(argv[2], 0, MAXDIM) : 1;
    findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);

    g_integer_rtn = (which == 0) ? OptionBase : g_var_tbl[g_current_var_idx].dims[which - 1];
    if (g_integer_rtn == -1) g_integer_rtn = 0;
    g_rtn_type = T_INT;
}
