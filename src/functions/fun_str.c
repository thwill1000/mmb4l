/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_str.c

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include <string.h>

#include "../common/mmb4l.h"

// Returns a string in the decimal (base 10) representation of  'number'.
// s$ = STR$( number, m, n, c$ )
void fun_str(void) {
    char *s;
    MMFLOAT f;
    MMINTEGER i64;
    int t;
    int m, n;
    char ch;
    const char *p;

    getargs(&ep, 7, ",");
    if((argc & 1) != 1) ERROR_SYNTAX;
    t = T_NOTYPE;
    p = evaluate(argv[0], &f, &i64, &s, &t, false);                 // get the value and type of the argument
    if(t & T_STR) error_throw_legacy("Expected a number");
    m = 0; n = STR_AUTO_PRECISION; ch = ' ';
    if(argc > 2) m = getint(argv[2], -128, 128);                    // get the number of digits before the point
    if(argc > 4) n = getint(argv[4], -20, 20);                      // get the number of digits after the point
    if(argc == 7) {
        p = getstring(argv[6]);
        if(*p == 0) error_throw_legacy("Zero length argument");
        ch = ((unsigned char)p[1] & 0x7f);
    }

    sret = GetTempStrMemory();                                      // this will last for the life of the command
    if(t & T_NBR)
        FloatToStr(sret, f, m, n, ch);                              // convert the float
    else {
        if(n < 0)
            FloatToStr(sret, i64, m, n, ch);                        // convert as a float
        else {
            IntToStrPad(sret, i64, ch, m, 10);                      // convert the integer
            if(n != STR_AUTO_PRECISION && n > 0) {
                strcat(sret, ".");
                while(n--) strcat(sret, "0");                       // and add on any zeros after the point
            }
        }
    }
    CtoM(sret);
    targ = T_STR;
}
