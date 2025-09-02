/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_lineinput.c

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
#include "../common/mmgetchar.h"
#include "../common/streamio.h"

#define MMfputs(mmbstr, fnbr)  streamio_write(fnbr, mmbstr + 1, mmbstr[0])

void cmd_lineinput(void) {
    char *vp;
    int i, fnbr;
    const DelimType delim[] = { ',', ';', 0 };
    getargs(&cmdline, 3, delim);
    if(argc == 0 || argc == 2) ERROR_SYNTAX;

    i = 0;
    fnbr = 0;
    if(argc == 3) {
        // is the first argument a file number specifier?  If so, get it
        if(*argv[0] == '#' && *argv[1] == ',') {
            argv[0]++;
            fnbr = getinteger(argv[0]);
        }
        else {
            // is the first argument a prompt?  if so, print it otherwise there are too many arguments
            if(*argv[1] != ',' && *argv[1] != ';') ERROR_SYNTAX;
            MMfputs(getstring(argv[0]), 0);
        }
        i = 2;
    }

    if(argc - i != 1) ERROR_SYNTAX;
    vp = findvar(argv[i], V_FIND);
    if(vartbl[VarIndex].type & T_CONST) error_throw_legacy("Cannot change a constant");
    if(!(vartbl[VarIndex].type & T_STR)) error_throw_legacy("Invalid variable");
    MMgetline(fnbr, inpbuf);                                        // get the input line
    if(strlen(inpbuf) > vartbl[VarIndex].size) error_throw_legacy("String too long");
    strcpy(vp, inpbuf);
    CtoM(vp);                                                       // convert to a MMBasic string
}
