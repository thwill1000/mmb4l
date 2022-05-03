/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_call.c

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

#define ERROR_UNKNOWN_USER_FUNCTION  error_throw_ex(kError, "Unknown user function")

void fun_call(void) {
    int i;
    MMINTEGER i64 = 0;
    char *s = NULL;
    MMFLOAT f;
    char *q;
    char *p = getCstring(ep);  // get the function we want to call
    q = p;
    while (*q) {  // convert to upper case for the match
        *q = toupper(*q);
        q++;
    }
    q = ep;
    while (*q) {
        if (*q == ',' || *q == '\'') break;
        q++;
    }
    if (*q == ',') q++;
    i = FindSubFun(p, true);  // it could be a defined function
    cstring_cat(p, " ", STRINGSIZE);
    cstring_cat(p, q, STRINGSIZE);
    targ = T_NOTYPE;
    if (i >= 0) {  // >= 0 means it is a user defined function
        DefinedSubFun(true, p, i, &f, &i64, &s, &targ);
    } else {
        ERROR_UNKNOWN_USER_FUNCTION;
    }
    if (targ & T_STR) {
        sret = GetTempStrMemory();
        Mstrcpy(sret, s);  // if it is a string then save it
    }
    if (targ & T_INT) iret = i64;
    if (targ & T_NBR) fret = f;
}
