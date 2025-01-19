/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_on.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
#include "../common/utility.h"
#include "../core/tokentbl.h"

#define ERROR_TOO_MANY_NESTED_GOSUB  error_throw_ex(kError, "Too many nested GOSUB")

int g_key_complete = 0;

static MmResult on_error_abort(const char* p) {
    // Historically does not clear the error state, is this a bug or a feature?
    mmb_error_state_ptr->skip = 0;
    return kOk;
}

static MmResult on_error_clear(const char *p) {
    error_init(mmb_error_state_ptr);
    return kOk;
}

static MmResult on_error_ignore(const char *p) {
    error_init(mmb_error_state_ptr);
    mmb_error_state_ptr->skip = -1;
    return kOk;
}

static MmResult on_error_skip(const char *p) {
    error_init(mmb_error_state_ptr);
    mmb_error_state_ptr->skip = (*p == 0 || *p == '\'') ? 2 : getint(p, 1, 10000) + 1;
    return kOk;
}

static MmResult on_error(const char *p) {
    MmResult result = kOk;
    const char *p2;
    if ((p2 = checkstring(p, "ABORT"))) {
        result = on_error_abort(p2);
    } else if ((p2 = checkstring(p, "CLEAR"))) {
        result = on_error_clear(p2);
    } else if ((p2 = checkstring(p, "IGNORE"))) {
        result = on_error_ignore(p2);
    } else if ((p2 = checkstring(p, "SKIP"))) {
        result = on_error_skip(p2);
    } else {
        ERROR_UNKNOWN_SUBCOMMAND("ON ERROR");
    }
    return result;
}

/**
 * ON KEY {interrupt|0}
 * ON KEY ASCIIcode, {interrupt|0}
 */
static MmResult on_key(const char *p) {
    getargs(&p, 3, ",");
    if (argc == 1) {
        const char *interrupt_addr = GetIntAddressOrNull(argv[0]);
        if (interrupt_addr) {
            interrupt_enable_any_key(interrupt_addr);
        } else {
            interrupt_disable_any_key();
        }
    } else if (argc == 3) {
        int key = getint(argv[0], 0, 255);
        if (key == 0) {
            interrupt_disable_specific_key();
        } else {
            const char *interrupt_addr = GetIntAddressOrNull(argv[2]);
            if (interrupt_addr) {
                interrupt_enable_specific_key(key, interrupt_addr);
            } else {
                interrupt_disable_specific_key();
            }
        }
    } else {
        return kArgumentCount;
    }
    return kOk;
}

/** ON nbr GOTO | GOSUB target[,target, target,...] */
static MmResult on_number(const char *p) {
    char ss[4] = {tokenGOTO, tokenGOSUB, ',', 0};

    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ss);
    if (argc < 3 || argc % 2 == 0) return kArgumentCount;
    if (*argv[1] != ss[0] && *argv[1] != ss[1]) return kSyntax;

    int r = getint(argv[0], 0, 255);  // evaluate the expression controlling the statement
    if (r == 0 || r > argc / 2) return kOk;  // microsoft say that we just go on to the next line

    if (*argv[1] == ss[1]) {
        // this is a GOSUB, same as a GOTO but we need to first push the return pointer.
        if (gosubindex >= MAXGOSUB) ERROR_TOO_MANY_NESTED_GOSUB;
        errorstack[gosubindex] = CurrentLinePtr;
        gosubstack[gosubindex++] = nextstmt;
        LocalIndex++;
    }

    if (isnamestart(*argv[r * 2])) {
        nextstmt = findlabel(argv[r * 2]);  // must be a label
    } else {
        nextstmt = findline(getinteger(argv[r * 2]), true);  // try for a line number
    }
    IgnorePIN = false;
    return kOk;
}

/** ON PS2 {interrupt|0} */
static MmResult on_ps2(const char *p) {
    getargs(&p, 1, ",");
    if (argc != 1) return kArgumentCount;
    const char *interrupt_addr = GetIntAddressOrNull(argv[0]);
    if (interrupt_addr) {
        interrupt_enable(kInterruptKeyboardPs2, interrupt_addr);
    } else {
        interrupt_disable(kInterruptKeyboardPs2);
    }
    return kOk;
}

void cmd_on(void) {
    MmResult result = kOk;
    const char *p;
    if ((p = checkstring(cmdline, "ERROR"))) {
        result = on_error(p);
    } else if ((p = checkstring(cmdline, "KEY"))) {
        result = on_key(p);
    } else if ((p = checkstring(cmdline, "PS2"))) {
        result = on_ps2(p);
    } else {
        result = on_number(p);
    }
    ON_FAILURE_ERROR(result);
}
