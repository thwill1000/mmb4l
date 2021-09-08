#include <time.h>

#include "../common/global_aliases.h"
#include "../common/version.h"

// this is invoked as a command (ie, TIMER = 0)
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command and save in the timer
void cmd_timer(void) {
    while (*cmdline && *cmdline != tokenEQUAL) cmdline++;
    if (!*cmdline) error("Invalid syntax");
    int64_t ms = getinteger(++cmdline);

    clock_gettime(CLOCK_REALTIME, &g_timer);
    g_timer.tv_sec -= ms / 1000;
    g_timer.tv_nsec += 1000000 * (ms % 1000);
}