/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_linstr.c

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
#include "../common/error.h"

void fun_linstr(void) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    char *srch;
    char *str = NULL;
    int slen, found = 0, i, j, n;
    getargs(&ep, 5, ",");
    if (argc < 3 || argc > 5) ERROR_ARGUMENT_COUNT;
    int64_t start;
    if (argc == 5)
        start = getinteger(argv[4]) - 1;
    else
        start = 0;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *)ptr1;
        str = (char *)&dest[0];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    j = (vartbl[VarIndex].dims[0] - mmb_options.base);
    srch = getstring(argv[2]);
    slen = *srch;
    iret = 0;
    if (start > dest[0] || start < 0 || slen == 0 || dest[0] == 0 ||
        slen > dest[0] - start)
        found = 1;
    if (!found) {
        n = dest[0] - slen - start;

        for (i = start; i <= n + start; i++) {
            if (str[i + 8] == srch[1]) {
                for (j = 0; j < slen; j++)
                    if (str[j + i + 8] != srch[j + 1]) break;
                if (j == slen) {
                    iret = i + 1;
                    break;
                }
            }
        }
    }
    targ = T_INT;
}
