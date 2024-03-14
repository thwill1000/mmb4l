/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_do.c

Copyright 2011-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include "../Configuration.h"
#include "../common/error.h"
#include "../core/Commands.h"
#include "../core/MMBasic.h"
#include "../core/tokentbl.h"

#include <stdio.h>

void cmd_do(void) {
    int i;
    const char *p, *tp, *evalp;
    char looptoken;

    char whiletoken = GetCommandValue("While");
    char whileloop = (cmdtoken == whiletoken);
    if(whileloop)
        looptoken = GetCommandValue("WEnd");
    else {
        looptoken = GetCommandValue("Loop");
        whiletoken = tokenWHILE;
    }

    if(whileloop)
        // if it is a WHILE WEND loop we can just point to the command line
        evalp = cmdline;
    else {
        // if it is a DO loop find the WHILE token and (if found) get a pointer to its expression
        while(*cmdline && *cmdline != whiletoken) cmdline++;
        if(*cmdline == whiletoken) {
            evalp = ++cmdline;
        }
        else
            evalp = NULL;
    }

    // check if this loop is already in the stack and remove it if it is
    // this is necessary as the program can jump out of the loop without hitting
    // the LOOP or WEND stmt and this will eventually result in a stack overflow
    for(i = 0; i < doindex ;i++) {
        if(dostack[i].doptr == nextstmt) {
            while(i < doindex - 1) {
                dostack[i].evalptr = dostack[i+1].evalptr;
                dostack[i].loopptr = dostack[i+1].loopptr;
                dostack[i].doptr = dostack[i+1].doptr;
                dostack[i].level = dostack[i+1].level;
                i++;
            }
            doindex--;
            break;
        }
    }

    // add our pointers to the top of the stack
    if(doindex == MAXDOLOOPS) error_throw_ex(kError, "Too many nested DO or WHILE loops");
    dostack[doindex].evalptr = evalp;
    dostack[doindex].doptr = nextstmt;
    dostack[doindex].level = LocalIndex;

    // now find the matching LOOP command
    i = 1; p = nextstmt;
    while(1) {
        p = GetNextCommand(p, &tp, "No matching LOOP");
        if(*p == cmdtoken) i++;                                     // entered a nested DO or WHILE loop
        if(*p == looptoken) i--;                                    // exited a nested loop
        if(i == 0) {                                                // found our matching LOOP or WEND stmt
            dostack[doindex].loopptr = p;
            break;
        }
    }

    if(!whileloop && dostack[doindex].evalptr != NULL) {
        // if this is a DO WHILE ... LOOP statement
        // search the LOOP statement for a WHILE or UNTIL token (p is pointing to the matching LOOP statement)
        p++;
        while(*p && *p < 0x80) p++;
        if(*p == tokenWHILE) error_throw_ex(kError, "LOOP has a WHILE test");
        if(*p == tokenUNTIL) error_throw_ex(kError, "LOOP has an UNTIL test");
    }

    doindex++;

    // do the evaluation (if there is something to evaluate) and if false go straight to the command after the LOOP or WEND statement
    if(dostack[doindex - 1].evalptr != NULL && getnumber(dostack[doindex - 1].evalptr) == 0) {
        doindex--;                                                  // remove the entry in the table
        nextstmt = dostack[doindex].loopptr;                        // point to the LOOP or WEND statement
        skipelement(nextstmt);                                      // skip to the next command
    }

}
