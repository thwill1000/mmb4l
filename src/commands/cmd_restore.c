/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_rename.c

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

/**
 * RESTORE label
 * RESTORE <string expression>
 * RESTORE <number expression>
 */
void cmd_restore(void) {
    if (*cmdline == 0 || *cmdline == '\'') {
        NextDataLine = ProgMemory;
        NextData = 0;
    }
    else {
        skipspace(cmdline);
        if (*cmdline == '"') {
            // Restore target starts with a literal string.
            NextDataLine = findlabel(getCstring(cmdline));
            NextData = 0;
        }
        else if (isdigit(*cmdline)
                || *cmdline == GetTokenValue("+")
                || *cmdline == GetTokenValue("-")
                || *cmdline == '.'
                || *cmdline == '&') {
            // Restore target starts with a number.
            NextDataLine = findline(getinteger(cmdline), true);
            NextData = 0;
        }
        else {
            void *ptr = findvar(cmdline, V_NOFIND_NULL);
            if (ptr) {
                // Restore target starts with a variable.
                if (vartbl[VarIndex].type & T_NBR) {
                    if (vartbl[VarIndex].dims[0] > 0) ERROR_SYNTAX;  // must be scalar
                    NextDataLine = findline(getinteger(cmdline), true);
                }
                else if (vartbl[VarIndex].type & T_INT) {
                    if (vartbl[VarIndex].dims[0] > 0) ERROR_SYNTAX;  // must be scalar
                    NextDataLine = findline(getinteger(cmdline), true);
                }
                else {
                    NextDataLine = findlabel(getCstring(cmdline));
                }
            }
            else if (isnamestart(*cmdline)) {
                // Restore target is a label.
                NextDataLine = findlabel(cmdline);
            }
            NextData = 0;
        }
    }
}
