#include <time.h>

#include "../common/error.h"
#include "../common/global_aliases.h"
#include "../common/mmtime.h"
#include "../common/version.h"

// this is invoked as a command (ie, TIMER = 0)
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command and save in the timer
void cmd_timer(void) {
    while (*cmdline && *cmdline != tokenEQUAL) cmdline++;
    if (!*cmdline) ERROR_SYNTAX;

    int64_t msec = getinteger(++cmdline);

    mmtime_set_timer_ns(MILLISECONDS_TO_NANOSECONDS(msec));
}
