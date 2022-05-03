/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_call.c

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
#include "../common/cstring.h"
#include "../common/error.h"
#include "../common/utility.h"

#define ERROR_UNKNOWN_SUBROUTINE  error_throw_ex(kError, "Unknown user subroutine")

void cmd_call(void) {
    int i;
    char *q;
    char *p = getCstring(cmdline);  // get the command we want to call
    q = p;
    while (*q) {  // convert to upper case for the match
        *q = toupper(*q);
        q++;
    }
    q = cmdline;
    while (*q) {
        if (*q == ',' || *q == '\'') break;
        q++;
    }
    if (*q == ',') q++;
    i = FindSubFun(p, false);  // it could be a defined command
    cstring_cat(p, " ", STRINGSIZE);
    cstring_cat(p, q, STRINGSIZE);
    // MMPrintString(p);PRet();
    if (i >= 0) {  // >= 0 means it is a user defined command
        DefinedSubFun(false, p, i, NULL, NULL, NULL, NULL);
    } else {
        ERROR_UNKNOWN_SUBROUTINE;
    }
}
