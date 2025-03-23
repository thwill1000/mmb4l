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

#include <string.h>

#include "../common/mmb4l.h"

// syntax:  nbr = INSTR([start,] string1, string2)
//          find the position of string2 in string1 starting at start chars in string1
// returns an integer
void fun_instr(void) {
    char *s1 = NULL, *s2 = NULL;
    int start = 0;
    getargs(&ep, 5, ",");

    if(argc == 5) {
        start = getint(argv[0], 1, MAXSTRLEN + 1) - 1;
        s1 = getstring(argv[2]);
        s2 = getstring(argv[4]);
    }
    else if(argc == 3) {
        start = 0;
        s1 = getstring(argv[0]);
        s2 = getstring(argv[2]);
    }
    else {
        error_throw_legacy("Argument count");
    }

    targ = T_INT;
    if(start > *s1 - *s2 + 1 || *s2 == 0)
        iret = 0;
    else {
        // find s2 in s1 using MMBasic strings
        int i;
        for(i = start; i < *s1 - *s2 + 1; i++) {
            if(memcmp(s1 + i + 1, s2 + 1, *s2) == 0) {
                iret = i + 1;
                return;
            }
        }
    }
    iret = 0;
}
