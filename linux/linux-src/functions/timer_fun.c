#include <time.h>

#include "../common/global_aliases.h"
#include "../common/version.h"

void fun_timer(void) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    g_integer_rtn = 1000 * (now.tv_sec - g_timer.tv_sec)
                    + (now.tv_nsec - g_timer.tv_nsec) / 1000000;
    g_rtn_type = T_INT;
}
