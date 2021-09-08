#include "../common/console.h"
#include "../common/error.h"
#include "../common/global_aliases.h"
#include "../common/version.h"

void fun_vres(void) {
    int width, height;
    if (!console_get_size(&width, &height)) {
        ERROR_COULD_NOT("determine console size");
    }
    g_integer_rtn = height;
    g_rtn_type = T_INT;
}
