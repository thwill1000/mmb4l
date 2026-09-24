/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_mid.c

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

#include "../common/mmb4l.h"

// Returns a substring of ?string$? beginning at ?start? and continuing for ?nbr? characters.
// S$ = MID$(s, spos [, nbr])
void fun_mid(void) {
    char *s, *p1, *p2;
    int spos, nbr = 0, i;
    getargs(&ep, 5, DELIM_COMMA);

    if(argc == 5) {                                                   // we have MID$(s, n, m)
        nbr = getint(argv[4], 0, MAXSTRLEN);                          // nbr of chars to return
    }
    else if(argc == 3) {                                              // we have MID$(s, n)
        nbr = MAXSTRLEN;                                              // default to all chars
    }
    else {
        error_throw_legacy("Argument count");
    }

    s = getstring(argv[0]);                                           // the string
    spos = getint(argv[2], 1, MAXSTRLEN);                             // the mid position

    sret = GetTempStrMemory();                                        // this will last for the life of the command
    targ = T_STR;
    if(spos > *s || nbr == 0)                                         // if the numeric args are not in the string
        return;                                                       // return a null string
    else {
        i = *s - spos + 1;                                            // find how many chars remaining in the string
        if(i > nbr) i = nbr;                                          // reduce it if we don't need that many
        p1 = sret; p2 = s + spos;
        *p1++ = i;                                                    // set the length of the MMBasic string
        while(i--) *p1++ = *p2++;                                     // copy the nbr chars required
    }
}
