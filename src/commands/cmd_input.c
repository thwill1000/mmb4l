/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_input.c

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

#include <stdlib.h>
#include <string.h>

#include "../common/display.h"
#include "../common/mmb4l.h"
#include "../common/mmgetline.h"

void cmd_input(void) {
    char s[STRINGSIZE];
    char *p, *sp, *tp;
    int i, fnbr;
    const DelimType delim[] = { ',', ';', 0 };
    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, delim);

    // is the first argument a file number specifier?  If so, get it
    if(argc >= 3 && *argv[0] == '#') {
        argv[0]++;
        fnbr = getinteger(argv[0]);
        i = 2;
    }
    else {
        fnbr = 0;
        // is the first argument a prompt?
        // if so, print it followed by an optional question mark
        if(argc >= 3 && *argv[0] == '"' && (*argv[1] == ',' || *argv[1] == ';')) {
            *(argv[0] + strlen(argv[0]) - 1) = 0;
            argv[0]++;
            display_puts(argv[0]);
            if(*argv[1] == ';') display_puts("? ");
            i = 2;
        } else {
            display_puts("? ");                                     // no prompt?  then just print the question mark
            i = 0;
        }
    }

    if(argc - i < 1) ERROR_SYNTAX;                                  // no variable to input to

    MMgetline(fnbr, inpbuf);                                        // get the line
    p = inpbuf;

    // step through the variables listed for the input statement
    // and find the next item on the line and assign it to the variable
    for(; i < argc; i++) {
        sp = s;                                                     // sp is a temp pointer into s[]
        if(*argv[i] == ',' || *argv[i] == ';') continue;
        skipspace(p);
        if(*p != 0) {
            if(*p == '"') {                                         // if it is a quoted string
                p++;                                                // step over the quote
                while(*p && *p != '"')  *sp++ = *p++;               // and copy everything upto the next quote
                while(*p && *p != ',') p++;                         // then find the next comma
            } else {                                                // otherwise it is a normal string of characters
                while(*p && *p != ',') *sp++ = *p++;                // copy up to the comma
                while(sp > s && sp[-1] == ' ') sp--;                // and trim trailing whitespace
            }
        }
        *sp = 0;                                                    // terminate the string
        tp = findvar(argv[i], V_FIND);                              // get the variable and save its new value
        if(vartbl[VarIndex].type & T_CONST) error_throw_legacy("Cannot change a constant");
        if(vartbl[VarIndex].type & T_STR) {
            if(strlen(s) > vartbl[VarIndex].size) error_throw_legacy("String too long");
            strcpy(tp, s);
            CtoM(tp);                                               // convert to a MMBasic string
        }
        else if(vartbl[VarIndex].type & T_INT) {
            *((MMINTEGER *)tp) = strtoll(s, &sp, 10);               // convert to an integer
        }
        else
            *((MMFLOAT *)tp) = (MMFLOAT)atof(s);
        if(*p == ',') p++;
    }
}
