#include <stdio.h>

#include "../common/mmtime.h"
#include "../common/version.h"

void cmd_pause(void) {
    int64_t wakeup = mmtime_now_ns() + MILLISECONDS_TO_NANOSECONDS(getinteger(cmdline));
    while (mmtime_now_ns() < wakeup) {
        CheckAbort();

        // A short sleep so we do not continue to thrash CPU when paused.
        nanosleep(&ONE_MICROSECOND, NULL);
    }
}
