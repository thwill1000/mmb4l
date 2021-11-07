#include "../common/global_aliases.h"
#include "../common/mmtime.h"
#include "../common/version.h"

void fun_timer(void) {
    g_rtn_type = T_INT;
    g_integer_rtn = NANOSECONDS_TO_MILLISECONDS(mmtime_get_timer_ns());
}
