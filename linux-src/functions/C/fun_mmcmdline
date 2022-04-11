#include "../common/mmb4l.h"

void get_mmcmdline(char *cmdline); // fun_mminfo.c

void fun_mmcmdline(void) {
    g_string_rtn = GetTempStrMemory();
    get_mmcmdline(g_string_rtn);
    CtoM(g_string_rtn);
    g_rtn_type = T_STR;
}
