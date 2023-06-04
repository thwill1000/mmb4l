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

#define ERROR_FUNCTION_NOT_FOUND  error_throw(kFunctionNotFound)

void fun_call(void) {
    MMINTEGER i64 = 0;
    char *s = NULL;
    MMFLOAT f;
    const char *q = ep;  // store the value of 'ep' because calling getCstring() can change it.
    char *fun = getCstring(ep);  // get the function we want to call
    cstring_toupper(fun);
    q = skipexpression(q);
    if (*q == ',') q++;
    int i = FindSubFun(fun, kFunction);  // find a function.
    cstring_cat(fun, " ", STRINGSIZE);
    cstring_cat(fun, q, STRINGSIZE);
    targ = T_NOTYPE;
    if (i >= 0) {  // >= 0 means it is a user defined function
        DefinedSubFun(true, fun, i, &f, &i64, &s, &targ);
    } else {
        ERROR_FUNCTION_NOT_FOUND;
    }
    if (targ & T_STR) {
        sret = GetTempStrMemory();
        Mstrcpy(sret, s);  // if it is a string then save it
    }
    if (targ & T_INT) iret = i64;
    if (targ & T_NBR) fret = f;
}
