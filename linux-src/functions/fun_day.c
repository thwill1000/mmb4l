/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_day.c

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

#include <stdlib.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/mmtime.h"

void fun_day(void) {
    int64_t time;

    if (checkstring(ep, "NOW")) {
        time = mmtime_now_ns();
    } else {
        char *arg = getCstring(ep);
        getargs(&arg, 5, "-/");  // this is a macro and must be the first
                                 // executable stmt in a block
        if (argc != 5) ERROR_SYNTAX;
        int d = atoi(argv[0]);
        int m = atoi(argv[2]);
        int y = atoi(argv[4]);
        if (d > 1000) {
            int tmp = d;
            d = y;
            y = tmp;
        }
        if (y >= 0 && y < 100) y += 2000;
        if (d < 1 || d > 31 || m < 1 || m > 12 || y < 1902 || y > 2999) ERROR_INVALID("date");
        struct tm tmbuf = { 0 };
        tmbuf.tm_year = y - 1900;
        tmbuf.tm_mon = m - 1;
        tmbuf.tm_mday = d;
        time = SECONDS_TO_NANOSECONDS(timegm(&tmbuf));
    }

    targ = T_STR;
    sret = GetTempStrMemory();
    mmtime_day_of_week(time, sret);
    CtoM(sret);
}
