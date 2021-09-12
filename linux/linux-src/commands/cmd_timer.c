#include <time.h>

#include "../common/error.h"
#include "../common/global_aliases.h"
#include "../common/version.h"

extern struct timespec g_timer; // main.c

// this is invoked as a command (ie, TIMER = 0)
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command and save in the timer
void cmd_timer(void) {
    while (*cmdline && *cmdline != tokenEQUAL) cmdline++;
    if (!*cmdline) {
        ERROR_SYNTAX;
        return;
    }

    int64_t ms_offset = getinteger(++cmdline);

    clock_gettime(CLOCK_REALTIME, &g_timer);
    int64_t ns_timer = 1000000000 * g_timer.tv_sec + g_timer.tv_nsec;
    ns_timer -= 1000000 * ms_offset;
    g_timer.tv_sec = ns_timer / 1000000000;
    g_timer.tv_nsec = ns_timer % 1000000000;
}
