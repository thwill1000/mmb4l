#include <stdio.h>
#include <time.h>

#include "../common/version.h"

void cmd_pause(void) {
    struct timespec one_micro_second;
    one_micro_second.tv_nsec = 1000;
    one_micro_second.tv_sec = 0;

    struct timespec wakeup;
    clock_gettime(CLOCK_REALTIME, &wakeup);
    int64_t delay = getinteger(cmdline);
    wakeup.tv_sec += delay / 1000;
    wakeup.tv_nsec += 1000000 * (delay % 1000);

    struct timespec now;
    int64_t ms;
    for (;;) {
        CheckAbort();
        clock_gettime(CLOCK_REALTIME, &now);
        ms = (1000 * (wakeup.tv_sec - now.tv_sec))
             + ((wakeup.tv_nsec - now.tv_nsec) / 1000000);
        //printf("Now:    %ld, %ld, %ld\n", now.tv_sec, now.tv_nsec, ms);
        if (ms <= 0) break;

        // A short sleep so we do not continue to thrash CPU when paused.
        nanosleep(&one_micro_second, NULL);
    }
}
