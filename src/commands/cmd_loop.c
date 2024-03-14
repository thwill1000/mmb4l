/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_loop.c

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

void cmd_loop(void) {
    const char *p;
    int tst = 0;  // initialise tst to stop the compiler from complaining
    int i;

    // search the do table looking for an entry with the same program position as this LOOP statement
    for(i = 0; i < doindex ;i++) {
        p = dostack[i].loopptr + 1;
        skipspace(p);
        if(p == cmdline) {
            // found a match
            // first check if the DO statement had a WHILE component
            // if not find the WHILE statement here and evaluate it
            if(dostack[i].evalptr == NULL) {                        // if it was a DO without a WHILE
                if(*cmdline >= 0x80) {                              // if there is something
                    if(*cmdline == tokenWHILE)
                        tst = (getnumber(++cmdline) != 0);          // evaluate the expression
                    else if(*cmdline == tokenUNTIL)
                        tst = (getnumber(++cmdline) == 0);          // evaluate the expression
                    else
                        ERROR_SYNTAX;
                }
                else {
                    tst = 1;                                        // and loop forever
                    checkend(cmdline);                              // make sure that there is nothing else
                }
            }
            else {                                                  // if was DO WHILE
                tst = (getnumber(dostack[i].evalptr) != 0);         // evaluate its expression
                checkend(cmdline);                                  // make sure that there is nothing else
            }

            // test the expression value and reset the program pointer if we are still looping
            // otherwise remove this entry from the do stack
            if(tst)
                nextstmt = dostack[i].doptr;                        // loop again
            else {
                // the loop has terminated
                // remove the entry in the table, then just let the default nextstmt run and continue on from there
                doindex = i;
                // just let the default nextstmt run
            }
            return;
        }
    }
    error_throw_ex(kError, "LOOP without a matching DO");
}
