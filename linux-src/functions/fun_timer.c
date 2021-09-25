#include <time.h>

#include "../common/global_aliases.h"
#include "../common/version.h"

extern struct timespec g_timer; // main.c

void fun_timer(void) {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    int64_t timer = (1000000000 * now.tv_sec + now.tv_nsec)
            - (1000000000 * g_timer.tv_sec + g_timer.tv_nsec);
    g_integer_rtn = timer / 1000000;
    g_rtn_type = T_INT;
}
