#include "../common/console.h"
#include "../common/global_aliases.h"
#include "../common/version.h"

void fun_hres(void) {
    console_get_size(&Option.Height, &Option.Width);
    g_integer_rtn = Option.Width;
    g_rtn_type = T_INT;
}
