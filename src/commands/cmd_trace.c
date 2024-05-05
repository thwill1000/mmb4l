/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_trace.c

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

#include "../common/mmb4l.h"
#include "../common/console.h"

#include <stdlib.h>
#include <string.h>

// count the number of lines up to and including the line pointed to by the argument
// used for error reporting in programs that do not use line numbers
int TraceLines(const char *target) {
    char *p;
    int i, cnt;

    p = (char *) ProgMemory;
    cnt = 0;
    if (target == NULL) return cnt;
    while (1) {
        if (*p == 0xff || (p[0] == 0 && p[1] == 0))  // end of the program
            return cnt;

        if (*p == T_NEWLINE) {
            p++;  // and step over the line number
            if (p >= target) {
                char buf[STRINGSIZE], buff[10];
                char *ename, *cpos = NULL;
                memcpy(buf, p, STRINGSIZE);
                i = 0;
                while (!(buf[i] == 0 && buf[i + 1] == 0)) i++;
                while (i > 0) {
                    if (buf[i] == '|') cpos = &buf[i];
                    i--;
                }
                if (cpos != NULL) {
                    console_puts("[");
                    if ((ename = strchr(cpos, ',')) != NULL) {
                        *ename = 0;
                        cpos++;
                        ename++;
                        console_puts(cpos);
                        console_puts(":");
                        console_puts(ename);
                    } else {
                        cpos++;
                        IntToStr(buff, atoi(cpos), 10);
                        console_puts(buff);
                    }
                    console_puts("] ");
                }
            }
            continue;
        }

        if (*p == T_LINENBR) {
            p += 3;  // and step over the line number
            continue;
        }

        if (*p == T_LABEL) {
            p += p[0] + 2;  // still looking! skip over the label
            continue;
        }

        if (p++ > target) return cnt;
    }
}

void cmd_trace(void) {
    if (checkstring(cmdline, "ON"))
        TraceOn = true;
    else if (checkstring(cmdline, "OFF"))
        TraceOn = false;
    else if (checkstring(cmdline, "LIST")) {
        int i;
        cmdline += 4;
        skipspace(cmdline);
        if (*cmdline == 0 || *cmdline == '\'')  //'
            i = TRACE_BUFF_SIZE - 1;
        else
            i = getint(cmdline, 0, TRACE_BUFF_SIZE - 1);
        i = TraceBuffIndex - i;
        if (i < 0) i += TRACE_BUFF_SIZE;
        while (i != TraceBuffIndex) {
            TraceLines(TraceBuff[i]);
            if (++i >= TRACE_BUFF_SIZE) i = 0;
        }
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("TRACE");
    }
}
