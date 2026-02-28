/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_select.c

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed on the console at startup (additional copyright messages may
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
#include "../core/tokentbl.h"

/**
 * SELECT CASE ...
 *
 * Note that 'SELECT CASE' is a single command token.
 */
void cmd_select(void) {
    const char *rp = NULL, *SaveCurrentLinePtr;
    MMFLOAT f = 0;
    MMINTEGER i64 = 0;
    char s[STRINGSIZE];

    int type = T_NOTYPE;
    void *v = DoExpression(cmdline, &type);                         // evaluate the select case value
    type = TypeMask(type);
    if (type & T_NBR) f = *(MMFLOAT *)v;
    if (type & T_INT) i64 = *(MMINTEGER *)v;
    if (type & T_STR) {
        Mstrcpy(s, (char *)v);
        ClearSpecificTempMemory(v);  // Free temp memory now that value is copied
        v = NULL;
    }

    int t;
    MMFLOAT ft = 0.0, ftt = 0.0;
    MMINTEGER i64t = 0, i64tt = 0;
    char *st = NULL, *stt = NULL;

    // Now search through the program looking for a matching END SELECT statement
    // i tracks the nesting level of any nested SELECT CASE commands
    SaveCurrentLinePtr = CurrentLinePtr;                            // save where we are because we will have to fake CurrentLinePtr to get errors reported correctly
    int i = 1;
    const char *p = nextstmt;
    while(1) {
        p = GetNextCommand(p, &rp, "No matching END SELECT");
        const CommandToken cmd = commandtbl_decode(p);

        if (cmd == cmdSELECT_CASE) i++;                             // found a nested SELECT CASE command, increase the nested count and carry on searching

        // is this a CASE stmt at the same level as this SELECT CASE.
        if (cmd == cmdCASE && i == 1) {
            CurrentLinePtr = rp;                                    // and report errors at the line we are on
            p += sizeof(CommandToken) - 1;                          // step over the CASE command

            // loop through the comparison elements on the CASE line.  Each element is separated by a comma
            do {
                // Release temporary memory buffers from previous iteration
                if (st) {
                    ClearSpecificTempMemory(st);
                    st = NULL;
                }
                if (stt) {
                    ClearSpecificTempMemory(stt);
                    stt = NULL;
                }

                p++;                                                // step over the comma, or the last byte of the CASE command token.
                skipspace(p);
                t = type;
                // check for CASE IS,  eg  CASE IS > 5  -or-  CASE > 5  and process it if it is
                // an operator can be >, <>, etc but it can also be a prefix + or - so we must not catch them
                FunctionToken funtok = tokentbl_peek(p);
                if (
                    (SaveCurrentLinePtr = checkstring(p, "IS")) ||
                    (
                        (tokentype(funtok) & T_OPER) &&
                        funtok != tokenADD &&
                        funtok != tokenSUBTRACT
                    )
                ) {
                    if(SaveCurrentLinePtr) p += 2;                  // step over the IS keyword
                    skipspace(p);
                    FunctionToken o = tokentbl_read(&p);            // get the operator
                    if (!(tokentype(o) & T_OPER)) ERROR_SYNTAX;
                    if(type & T_NBR) ft = f;
                    if(type & T_INT) i64t = i64;
                    if(type & T_STR) st = s;
                    while(o != E_END) p = doexpr(p, &ft, &i64t, &st, &o, &t); // get the right hand side of the expression and evaluate the operator in o
                    if(!(t & T_INT)) ERROR_SYNTAX;                  // comparisons must always return an integer
                    if(i64t) {                                      // evaluates to true
                        skipelement(p);
                        nextstmt = p;
                        CurrentLinePtr = SaveCurrentLinePtr;
                        return;                                     // if we have a match just return to the interpreter and let it execute the code
                    } else {                                        // evaluates to false
                        skipspace(p);
                        continue;
                    }
                }

                // it must be either a single value (eg, "foo") or a range (eg, "foo" TO "zoo")
                // evaluate the first value
                p = evaluate(p, &ft, &i64t, &st, &t, true);
                skipspace(p);
                if (tokentbl_peek(p) == tokenTO) {                  // is there is a TO keyword?
                    tokentbl_read(&p);
                    t = type;
                    p = evaluate(p, &ftt, &i64tt, &stt, &t, false); // evaluate the right hand side of the TO expression
                    if(((type & T_NBR) && f >= ft && f <= ftt) || ((type & T_INT) && i64 >= i64t && i64 <= i64tt) || (((type & T_STR) && Mstrcmp(s, st) >= 0) && (Mstrcmp(s, stt) <= 0))) {
                        skipelement(p);
                        nextstmt = p;
                        CurrentLinePtr = SaveCurrentLinePtr;
                        return;                                     // if we have a match just return to the interpreter and let it execute the code
                    } else {
                        skipspace(p);
                        continue;                                   // otherwise continue searching
                    }
                }

                // if we got to here the element must be just a single match.  So make the test
                if (((type & T_NBR) && f == ft) || ((type & T_INT) && i64 == i64t) || ((type & T_STR) && Mstrcmp(s, st) == 0)) {
                    skipelement(p);
                    nextstmt = p;
                    CurrentLinePtr = SaveCurrentLinePtr;
                    return;                                         // if we have a match just return to the interpreter and let it execute the code
                }
                skipspace(p);
            } while(*p == ',');                                     // keep looping through the elements on the CASE line
            checkend(p);
            CurrentLinePtr = SaveCurrentLinePtr;
        }

        // test if we have found a CASE ELSE statement at the same level as this SELECT CASE
        // if true it means that we did not find a matching CASE - so execute this code
        if (cmd == cmdCASE_ELSE && i == 1) {
            p += sizeof(CommandToken);                              // step over the token
            checkend(p);
            skipelement(p);
            nextstmt = p;
            CurrentLinePtr = SaveCurrentLinePtr;
            return;
        }

        if (cmd == cmdEND_SELECT) {                                 // found an END SELECT so decrement our nested counter
            i--;
            p += sizeof(CommandToken);                              // step over the token
        }

        if (i == 0) {
            // found our matching END SELECT stmt. Continue with the statement after it
            skipelement(p);
            nextstmt = p;
            CurrentLinePtr = SaveCurrentLinePtr;
            return;
        }
    }
}
