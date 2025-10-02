/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_settick.c

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include <limits.h>
#include <string.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/interrupt.h"
#include "../common/mmtime.h"

/** SETTICK { period | PAUSE | RESUME }, target [, nbr] */
void cmd_settick(void) {
    const char *p = checkstring(cmdline, "FAST");
    if (p) ERROR_UNIMPLEMENTED("SETTICK FAST");

    getargs(&cmdline, 5, DELIM_COMMA);
    if (argc != 3 && argc != 5) ERROR_ARGUMENT_COUNT;

    const int irq = has_arg(4) ? getint(argv[4], 1, NBRSETTICKS) - 1 : 0;

    if (strcasecmp(argv[0], "PAUSE") == 0) {
        ON_FAILURE_ERROR(interrupt_pause_tick(irq));
    } else if (strcasecmp(argv[0], "RESUME") == 0){
        ON_FAILURE_ERROR(interrupt_resume_tick(irq));
    } else {
        int64_t period_ns = MILLISECONDS_TO_NANOSECONDS(getint(argv[0], 0, INT_MAX));
        if (period_ns == 0) {
            interrupt_disable_tick(irq);
        } else {
            interrupt_enable_tick(irq, period_ns, GetIntAddress(argv[2]));
        }
    }
}
