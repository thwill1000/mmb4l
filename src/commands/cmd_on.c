/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_on.c

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

#include <ctype.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/interrupt.h"

#define ERROR_TOO_MANY_NESTED_GOSUB  error_throw_ex(kError, "Too many nested GOSUB")

int g_key_complete = 0;

static void on_error_abort(const char* p) {
    // Historically does not clear the error state, is this a bug or a feature?
    mmb_error_state_ptr->skip = 0;
}

static void on_error_clear(const char *p) {
    error_init(mmb_error_state_ptr);
}

static void on_error_ignore(const char *p) {
    error_init(mmb_error_state_ptr);
    mmb_error_state_ptr->skip = -1;
}

static void on_error_skip(const char *p) {
    error_init(mmb_error_state_ptr);
    mmb_error_state_ptr->skip = (*p == 0 || *p == '\'') ? 2 : getint(p, 1, 10000) + 1;
}

static void on_error(const char *p) {
    const char *p2;
    if ((p2 = checkstring(p, "ABORT"))) {
        on_error_abort(p2);
    } else if ((p2 = checkstring(p, "CLEAR"))) {
        on_error_clear(p2);
    } else if ((p2 = checkstring(p, "IGNORE"))) {
        on_error_ignore(p2);
    } else if ((p2 = checkstring(p, "SKIP"))) {
        on_error_skip(p2);
    } else {
        ERROR_SYNTAX;
    }
}

/**
 * ON KEY target
 * ON KEY ASCIIcode, target
 */
static void on_key(const char *p) {
    getargs(&p, 3, ",");
    if (argc == 1) {
        if (*argv[0] == '0' && !isdigit(*(argv[0] + 1))) {
            interrupt_disable_any_key();
        } else {
            interrupt_enable_any_key(GetIntAddress(argv[0]));
        }
    } else {
        int key = getint(argv[0], 0, 255);
        if (key == 0) {
            interrupt_disable_specific_key();
        } else {
            if (*argv[2] == '0' && !isdigit(*(argv[2] + 1))) {
                interrupt_disable_specific_key();
            } else {
                interrupt_enable_specific_key(key, GetIntAddress(argv[2]));
            }
        }
    }
}

/** ON nbr GOTO | GOSUB target[,target, target,...] */
static void on_number(const char *p) {
    int r;
    char ss[4] = {tokenGOTO, tokenGOSUB, ',', 0};
    {  // start a new block
        getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ss);  // getargs macro must be the first executable stmt in a block
        if (argc < 3 || !(*argv[1] == ss[0] || *argv[1] == ss[1])) ERROR_SYNTAX;
        if (argc % 2 == 0) ERROR_SYNTAX;

        r = getint(argv[0], 0, 255);  // evaluate the expression controlling the statement
        if (r == 0 || r > argc / 2) return;  // microsoft say that we just go on to the next line

        if (*argv[1] == ss[1]) {
            // this is a GOSUB, same as a GOTO but we need to first push the
            // return pointer
            if (gosubindex >= MAXGOSUB) ERROR_TOO_MANY_NESTED_GOSUB;
            errorstack[gosubindex] = CurrentLinePtr;
            gosubstack[gosubindex++] = nextstmt;
            LocalIndex++;
        }

        if (isnamestart(*argv[r * 2]))
            nextstmt = findlabel(argv[r * 2]);  // must be a label
        else
            nextstmt = findline(getinteger(argv[r * 2]), true);  // try for a line number
    }
    IgnorePIN = false;
}

void cmd_on(void) {
    const char *p;
    if ((p = checkstring(cmdline, "ERROR"))) {
        on_error(p);
    } else if ((p = checkstring(cmdline, "KEY"))) {
        on_key(p);
    } else {
        on_number(p);
    }
}
