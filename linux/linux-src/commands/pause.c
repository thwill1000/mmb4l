#include <time.h>

#include "../common/version.h"

void cmd_pause(void) {
    struct timespec wakeup;
    clock_gettime(CLOCK_REALTIME, &wakeup);
    //printf("%ld, %ld\n", wakeup.tv_sec, wakeup.tv_nsec);
    int64_t delay = getinteger(cmdline);
    //printf("%ld\n", delay);
    wakeup.tv_sec += delay / 1000;
    wakeup.tv_nsec += 1000000 * (delay % 1000);
    //printf("Wakeup: %ld, %ld\n", wakeup.tv_sec, wakeup.tv_nsec);
    struct timespec now;
    int64_t ms;
    while (!MMAbort) {
        clock_gettime(CLOCK_REALTIME, &now);
        ms = (1000 * (wakeup.tv_sec - now.tv_sec))
             + ((wakeup.tv_nsec - now.tv_nsec) / 1000000);
        //printf("Now:    %ld, %ld, %ld\n", now.tv_sec, now.tv_nsec, ms);
        if (ms <= 0) break;
    }
}
