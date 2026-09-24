/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_next.c

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
#include "../core/Commands.h"

void cmd_next(void) {
    int i, vindex, test;
    void *vtbl[MAXFORLOOPS];
    int vcnt;
    const char *p;
    getargs(&cmdline, MAXFORLOOPS * 2, DELIM_COMMA);                // getargs macro must be the first executable stmt in a block

    vindex = 0;                                                     // keep lint happy

    for(vcnt = i = 0; i < argc; i++) {
        if(i & 0x01) {
            if(*argv[i] != ',') ERROR_SYNTAX;
        } else
            vtbl[vcnt++] = findvar(argv[i], V_FIND | V_NOFIND_ERR); // find the variable and error if not found
    }

loopback:
    // first search the for stack for a loop with the same variable specified on the NEXT's line
    if(vcnt) {
        for(i = forindex - 1; i >= 0; i--)
            for(vindex = vcnt - 1; vindex >= 0 ; vindex--)
                if(forstack[i].var == vtbl[vindex])
                    goto breakout;
    } else {
        // if no variables specified search the for stack looking for an entry with the same program position as
        // this NEXT statement. This cheats by using the cmdline as an identifier and may not work inside an IF THEN ELSE
        for(i = 0; i < forindex; i++) {
            p = forstack[i].nextptr + sizeof(CommandToken);
            skipspace(p);
            if(p == cmdline) goto breakout;
        }
    }

    error_throw_legacy("Cannot find a matching FOR");

breakout:

    // found a match
    // apply the STEP value to the variable and test against the TO value
    if(forstack[i].vartype & T_INT) {
        *(MMINTEGER *)forstack[i].var += forstack[i].stepvalue.i;
        test = (forstack[i].stepvalue.i >= 0 && *(MMINTEGER *)forstack[i].var > forstack[i].tovalue.i) || (forstack[i].stepvalue.i < 0 && *(MMINTEGER *)forstack[i].var < forstack[i].tovalue.i) ;
    } else {
        *(MMFLOAT *)forstack[i].var += forstack[i].stepvalue.f;
        test = (forstack[i].stepvalue.f >= 0 && *(MMFLOAT *)forstack[i].var > forstack[i].tovalue.f) || (forstack[i].stepvalue.f < 0 && *(MMFLOAT *)forstack[i].var < forstack[i].tovalue.f) ;
    }

    if(test) {
        // the loop has terminated
        // remove the entry in the table, then skip forward to the next element and continue on from there
        while(i < forindex - 1) {
            forstack[i].forptr = forstack[i+1].forptr;
            forstack[i].nextptr = forstack[i+1].nextptr;
            forstack[i].var = forstack[i+1].var;
            forstack[i].vartype = forstack[i+1].vartype;
            forstack[i].level = forstack[i+1].level;
            forstack[i].tovalue.i = forstack[i+1].tovalue.i;
            forstack[i].stepvalue.i = forstack[i+1].stepvalue.i;
            i++;
        }
        forindex--;
        if(vcnt > 0) {
            // remove that entry from our FOR stack
            for(; vindex < vcnt - 1; vindex++) vtbl[vindex] = vtbl[vindex + 1];
            vcnt--;
            if(vcnt > 0)
                goto loopback;
            else
                return;
        }

    } else {
        // we have not reached the terminal value yet, so go back and loop again
        nextstmt = forstack[i].forptr;
    }
}
