#include "../common/global_aliases.h"
#include "../common/version.h"

// Function (which looks like a pre defined variable) to return the type of
// platform.
void fun_device(void) {
    g_string_rtn = GetTempStrMemory();
    strcpy(g_string_rtn, "Linux"); // x86_64");
    CtoM(g_string_rtn);
    g_rtn_type = T_STR;
}
