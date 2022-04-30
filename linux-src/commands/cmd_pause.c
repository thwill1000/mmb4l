#include <stdbool.h>
#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/interrupt.h"
#include "../common/mmtime.h"

static void cmd_pause_in_interrupt(int64_t duration_ns) {
    int64_t wakeup = mmtime_now_ns() + duration_ns;
    while (mmtime_now_ns() < wakeup) {
        CheckAbort();

        // A short sleep so we do not continue to thrash CPU when paused.
        nanosleep(&ONE_MICROSECOND, NULL);
    }
    return;
}

static void cmd_pause_in_main_program(int64_t duration_ns) {
    static int64_t wakeup = 0;

    if (!interrupt_pause_needs_resuming()) {
        // Completely new PAUSE.
        wakeup = mmtime_now_ns() + duration_ns;
    }

    while (mmtime_now_ns() < wakeup) {
        CheckAbort();

        if (interrupt_check()) {
            // If there is an interrupt fake the return point to the start of
            // the PAUSE statement and return immediately to the program processor so
            // that it can send us off to the interrupt routine.  When the
            // interrupt routine finishes we should reexecute this PAUSE
            // statement and because the variable interrupted is static we can
            // see that we need to resume pausing rather than start a new pause time.
            while (*cmdline && *cmdline != cmdtoken) cmdline--;
            interrupt_pause(cmdline);
            return;
        }

        // A short sleep so we do not continue to thrash CPU when paused.
        nanosleep(&ONE_MICROSECOND, NULL);
    }
}

void cmd_pause(void) {
    int64_t duration_ns = (int64_t) (getnumber(cmdline) * 1000000.0);
    if (duration_ns < 0) {
        ERROR_NUMBER_OUT_OF_BOUNDS;
    } else if (duration_ns < 50000) {
        // Do nothing.
    } else if (duration_ns < 1500000) {
        // If less than 1.5ms do the pause right now and exit.
        mmtime_sleep_ns(duration_ns);
    } else if (interrupt_running()) {
        cmd_pause_in_interrupt(duration_ns);
    } else {
        cmd_pause_in_main_program(duration_ns);
    }
}
