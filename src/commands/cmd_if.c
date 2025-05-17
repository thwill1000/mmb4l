/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_else.c

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
#include "../core/tokentbl.h"

static void execute_one_command(char *p) {
    CheckAbort();
    targ = T_CMD;
    skipspace(p);                                                   // skip any whitespace
    if (p[0]>= C_BASETOKEN && p[1]>=C_BASETOKEN) {
        const CommandToken cmd = commandtbl_decode(p);
        if (cmd == cmdWHILE || cmd== cmdDO || cmd == cmdFOR) {
            error_throw_legacy("Invalid inside THEN ... ELSE") ;
        }
        cmdtoken = cmd;
        cmdline = p + sizeof(CommandToken);
        skipspace(cmdline);
        commandtbl[cmd].fptr(); // execute the command
    } else {
        if(!isnamestart(*p)) error_throw_legacy("Invalid character");
        int i = FindSubFun(p, kSub);                                // find a subroutine.
        if(i >= 0)                                                  // >= 0 means it is a user defined command
            DefinedSubFun(false, p, i, NULL, NULL, NULL, NULL);
        else
            error_throw_legacy("Unknown command");
    }
    ClearTempMemory();                                              // at the end of each command we need to clear any temporary string vars
}

/**
 * IF <condition> THEN <statement> ELSE <statement>
 */
void cmd_if(void) {
    int r, i, testgoto, testelseif;
    DelimType delim[] = { tokenTHEN, tokenELSE, 0 };
    const char *p, *tp;
    const char *rp = NULL;

    testgoto = false;
    testelseif = false;

retest_an_if:
    {
        getargs(&cmdline, 20, delim);

        if(testelseif && argc > 2) error_throw_legacy("Unexpected text");

        // if there is no THEN token retry the test with a GOTO.  If that fails flag an error
        if(argc < 2 || tokentbl_peek(argv[1]) != delim[0]) {
            if(testgoto) error_throw_legacy("IF without THEN");
            delim[0] = tokenGOTO;
            testgoto = true;
            goto retest_an_if;
        }

        // allow for IF statements embedded inside this IF
        if (argc >= 3 && commandtbl_decode(argv[2]) == cmdIF) argc = 3;  // this is IF xx=yy THEN IF ... so we want to evaluate only the first 3
        if (argc >= 5 && commandtbl_decode(argv[4]) == cmdIF) argc = 5;  // this is IF xx=yy THEN cmd ELSE IF ... so we want to evaluate only the first 5

        if (argc == 4 || (argc == 5 && tokentbl_peek(argv[3]) != tokenELSE)) ERROR_SYNTAX;

        r = (getnumber(argv[0]) != 0);                              // evaluate the expression controlling the if statement

        if(r) {
            // the test returned TRUE
            // first check if it is a multiline IF (ie, only 2 args)
            if(argc == 2) {
                // if multiline do nothing, control will fall through to the next line (which is what we want to execute next)
                ;
            }
            else {
                // This is a standard single line IF statement
                // Because the test was TRUE we are just interested in the THEN cmd stage.
                if (tokentbl_peek(argv[1]) == tokenGOTO) {
                    // IF <condition> GOTO <line>
                    cmdline = argv[2];
                    cmd_goto();
                    return;
                } else if (isdigit(*argv[2])) {
                    // IF <condition> THEN <line>
                    nextstmt = findline(getinteger(argv[2]), true);
                } else if (argc == 5) {
                    // IF <condition> THEN <statement1> ELSE <statement2>
                    execute_one_command(argv[2]);
                } else {
                    // IF <condition> THEN <statement>
                    for (p = cmdline; *p && tokentbl_read(&p) != tokenTHEN; ) { }
                    nextstmt = p;  // The statement after the THEN token.
                }
            }
        } else {
            // the test returned FALSE so we are just interested in the ELSE stage (if present)
            // first check if it is a multiline IF (ie, only 2 args)
            if(argc == 2) {
                // search for the next ELSE, or ENDIF and pass control to the following line
                // if an ELSEIF is found re execute this function to evaluate the condition following the ELSEIF
                i = 1; p = nextstmt;
                while(1) {
                    p = GetNextCommand(p, &rp, "No matching ENDIF");
                    const CommandToken cmd = commandtbl_decode(p);
                    if (cmd == cmdtoken) {
                        // found a nested IF command, we now need to determine if it is a single or multiline IF
                        // search for a THEN, then check if only white space follows.  If so, it is multiline.
                        tp = p + sizeof(CommandToken);
                        while (*tp && tokentbl_read(&tp) != delim[0]) { }  // find and step over THEN
                        skipspace(tp);
                        if(*tp == 0 || *tp == '\'')                 // yes, only whitespace follows
                            i++;                                    // count it as a nested IF
                        else                                        // no, it is a single line IF
                            skipelement(p);                         // skip to the end so that we avoid an ELSE
                        continue;
                    }

                    if (cmd == cmdELSE && i == 1) {
                        // found an ELSE at the same level as this IF.  Step over it and continue with the statement after it
                        skipelement(p);
                        nextstmt = p;
                        break;
                    }

                    if((cmd == cmdELSEIF || cmd == cmdELSE_IF) && i == 1) {
                        // we have found an ELSEIF statement at the same level as our IF statement
                        // setup the environment to make this function evaluate the test following ELSEIF and jump back
                        // to the start of the function.  This is not very clean (it uses the dreaded goto for a start) but it works
                        p += sizeof(CommandToken);                  // step over the token
                        skipspace(p);
                        CurrentLinePtr = rp;
                        if(*p == 0) ERROR_SYNTAX;                   // there must be a test after the elseif
                        cmdline = p;
                        skipelement(p);
                        nextstmt = p;
                        testgoto = false;
                        testelseif = true;
                        goto retest_an_if;
                    }

                    if(cmd == cmdENDIF || cmd == cmdEND_IF) i--;    // found an ENDIF so decrement our nested counter
                    if(i == 0) {
                        // found our matching ENDIF stmt.  Step over it and continue with the statement after it
                        skipelement(p);
                        nextstmt = p;
                        break;
                    }
                }
            }
            else {
                // this must be a single line IF statement
                // check if there is an ELSE on the same line
                if (argc == 5) {
                    // There is an ELSE command
                    if (isdigit(*argv[4])) {
                        // IF <condition> THEN <statement> ELSE <line>
                        nextstmt = findline(getinteger(argv[4]), true);
                    } else {
                        // IF <condition> THEN <statement1> ELSE <statement2>

                        // Find and read the THEN function token.
                        //
                        // IMPORTANT! we cannot simply start from the beginning of the IF statement
                        // and look for the ELSE token because <statement1> might begin with a
                        // command that has a command token ID equal to the ELSE function token ID.
                        for (p = cmdline; *p && (tokentbl_read(&p) != tokenTHEN); ) { }

                        // Skip the command that <statement1> must start with.
                        skipspace(p);
                        p += sizeof(CommandToken);

                        // Find and read the ELSE function token.
                        for (; *p && (tokentbl_read(&p) != tokenELSE); ) { }

                        nextstmt = p;  // The statement after the ELSE token.
                    }
                } else {
                    // no ELSE on a single line IF statement, so just continue with the next statement
                    // this used to be just skipline(cmdline), but extra error checking is needed
                    skipline(cmdline);
                    nextstmt = cmdline;
                }
            }
        }
    }
}
