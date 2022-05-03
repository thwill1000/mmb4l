/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_format.c

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

#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/error.h"

#define ERROR_ONLY_ONE_FORMAT_SPECIFIER_ALLOWED  error_throw_ex(kError, "Only one format specifier (%) allowed")
#define ERROR_ILLEGAL_CHARACTER_IN_FORMAT        error_throw_ex(kError, "Illegal character in format specification")
#define ERROR_FORMAT_SPECIFICATION_NOT_FOUND     error_throw_ex(kError, "Format specification not found")

#define IsDigitinline(a) ( a >= '0' && a <= '9' )

void fun_format(void) {
    char *p, *fmt;
    int inspec;
    getargs(&ep, 3, ",");
    if (argc % 2 == 0) ERROR_SYNTAX;
    if (argc == 3)
        fmt = getCstring(argv[2]);
    else
        fmt = "%g";

    // check the format string for errors that might crash the CPU
    for (inspec = 0, p = fmt; *p; p++) {
        if (*p == '%') {
            inspec++;
            if (inspec > 1) ERROR_ONLY_ONE_FORMAT_SPECIFIER_ALLOWED;
            continue;
        }

        if (inspec == 1 && (*p == 'g' || *p == 'G' || *p == 'f' || *p == 'e' ||
                            *p == 'E' || *p == 'l'))
            inspec++;

        if (inspec == 1 && !(IsDigitinline(*p) || *p == '+' || *p == '-' ||
                             *p == '.' || *p == ' '))
            ERROR_ILLEGAL_CHARACTER_IN_FORMAT;
    }
    if (inspec != 2) ERROR_FORMAT_SPECIFICATION_NOT_FOUND;
    sret = GetTempStrMemory();  // this will last for the life of the command
    sprintf(sret, fmt, getnumber(argv[0]));
    CtoM(sret);
    targ = T_STR;
}
