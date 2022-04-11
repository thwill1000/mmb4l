#include "../common/mmb4l.h"

void get_mmdevice(char *device); // fun_mminfo.c

void fun_mmdevice(void) {
    g_string_rtn = GetTempStrMemory();
    get_mmdevice(g_string_rtn);
    CtoM(g_string_rtn);
    g_rtn_type = T_STR;
}
