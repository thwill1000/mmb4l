/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_pause.c

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

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
