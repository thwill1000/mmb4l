#include <stdio.h>

#include "../common/utility.h"
#include "../common/version.h"

void cmd_pause(void) {
    uint64_t wakeup = time_now_ns() + 1000000UL * (uint64_t) getinteger(cmdline);
    while (time_now_ns() < wakeup) {
        CheckAbort();

        // A short sleep so we do not continue to thrash CPU when paused.
        nanosleep(&ONE_MICROSECOND, NULL);
    }
}
