/*-*****************************************************************************

MMBasic for Linux (MMB4L)

Commands.c

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

// Provides all the core commands in MMBasic.

#include "../Hardware_Includes.h"
#include "MMBasic.h"
#include "Commands.h"
#include "commandtbl.h"
#include "tokentbl.h"
#include "funtbl.h"
#include "vartbl.h"
#include "../common/cstring.h"
#include "../common/parse.h"
#include "../common/utility.h"

void flist(int, int, int);
void clearprog(void);
void execute_one_command(char *p);
void op_equal(void);

// stack to keep track of nested FOR/NEXT loops
struct s_forstack forstack[MAXFORLOOPS + 1];
int forindex;

// stack to keep track of nested DO/LOOP loops
struct s_dostack dostack[MAXDOLOOPS];
int doindex;                                                        // counts the number of nested DO/LOOP loops

// stack to keep track of GOSUBs, SUBs and FUNCTIONs
const char *gosubstack[MAXGOSUB];
const char *errorstack[MAXGOSUB];
int gosubindex;

char DimUsed = false;                                               // used to catch OPTION BASE after DIM has been used

int TraceOn;                                                        // used to track the state of TRON/TROFF
const char *TraceBuff[TRACE_BUFF_SIZE];
int TraceBuffIndex;                                                 // used for listing the contents of the trace buffer

#if !defined(__mmb4l__)
int OptionErrorSkip;                                                // how to handle an error
int MMerrno;                                                        // the error number
char MMErrMsg[MAXERRMSG];                                           // the error message
#endif



void cmd_null(void) {
  // do nothing (this is just a placeholder for commands that have no action)
}

#if !defined(__mmb4l__)
// the PRINT command
void cmd_print(void) {
    char *s, *p;
    MMFLOAT f;
    MMINTEGER i64;
    int i, t, fnbr;
    int docrlf;                                                     // this is used to suppress the cr/lf if needed

    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ";,");               // this is a macro and must be the first executable stmt

    // s = 0; *s = 56;                                              // for testing the exception handler

    docrlf = true;

    if(argc > 0 && *argv[0] == '#') {                               // check if the first arg is a file number
        argv[0]++;
        fnbr = getinteger(argv[0]);                                 // get the number
        i = 1;
        if(argc >= 2 && *argv[1] == ',') i = 2;                     // and set the next argument to be looked at
    } else {
        fnbr = 0;                                                   // no file number so default to the standard output
        i = 0;
    }

    for(; i < argc; i++) {                                          // step through the arguments
        if(*argv[i] == ',') {
            MMfputc('\t', fnbr);                                    // print a tab for a comma
            docrlf = false;                                         // a trailing comma should suppress CR/LF
        }
        else if(*argv[i] == ';') {
            docrlf = false;                                         // other than suppress cr/lf do nothing for a semicolon
        }
        else {                                                      // we have a normal expression
            p = argv[i];
            while(*p) {
                t = T_NOTYPE;
                p = evaluate(p, &f, &i64, &s, &t, true);            // get the value and type of the argument
                if(t & T_NBR) {
                    *inpbuf = ' ';                                  // preload a space
                    FloatToStr(inpbuf + ((f >= 0) ? 1:0), f, 0, STR_AUTO_PRECISION, ' ');// if positive output a space instead of the sign
                    MMfputs(CtoM(inpbuf), fnbr);                    // convert to a MMBasic string and output
                } else if(t & T_INT) {
                    *inpbuf = ' ';                                  // preload a space
                    IntToStr(inpbuf + ((i64 >= 0) ? 1:0), i64, 10); // if positive output a space instead of the sign
                    MMfputs(CtoM(inpbuf), fnbr);                    // convert to a MMBasic string and output
                } else if(t & T_STR) {
                    MMfputs(s, fnbr);                               // print if a string (s is a MMBasic string)
                }
            }
            docrlf = true;
        }
    }
    if(docrlf) MMfputs("\2\r\n", fnbr);                             // print the terminating cr/lf unless it has been suppressed
}
#endif



// the LET command
// because the LET is implied (ie, line does not have a recognisable command)
// it ends up as the place where mistyped commands are discovered.  This is why
// the error message is "Unknown command"
void cmd_let(void) {
    int t, size;
    MMFLOAT f;
    MMINTEGER i64;
    char *s;

    const char *p1 = cmdline;

    // search through the line looking for the equals sign
    while(*p1 && tokenfunction(*p1) != op_equal) p1++;
    if(!*p1) error("Unknown command");

    // check that we have a straight forward variable
    const char *p2 = skipvar(cmdline, false);
    skipspace(p2);
    if (p1 != p2) ERROR_SYNTAX;

    // create the variable and get the length if it is a string
    char *pvar = findvar(cmdline, V_FIND);
    size = vartbl[VarIndex].size;
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");

    // step over the equals sign, evaluate the rest of the command and save in the variable
    p1++;
    if(vartbl[VarIndex].type & T_STR) {
        t = T_STR;
        p1 = evaluate(p1, &f, &i64, &s, &t, false);
        if(*s > size) error("String too long");
        Mstrcpy(pvar, s);
    }
    else if(vartbl[VarIndex].type & T_NBR) {
        t = T_NBR;
        p1 = evaluate(p1, &f, &i64, &s, &t, false);
        if(t & T_NBR)
            (*(MMFLOAT *)pvar) = f;
        else
            (*(MMFLOAT *)pvar) = (MMFLOAT)i64;
    } else {
        t = T_INT;
        p1 = evaluate(p1, &f, &i64, &s, &t, false);
        if(t & T_INT)
            (*(MMINTEGER *)pvar) = i64;
        else
            (*(MMINTEGER *)pvar) = FloatToInt64(f);
    }
    checkend(p1);
}



void ListNewLine(int *ListCnt, int all) {
    MMPrintString("\r\n");
    (*ListCnt)++;
    if(!all && *ListCnt >= Option.Height) {
        MMPrintString("PRESS ANY KEY ...");
        MMgetchar();
        MMPrintString("\r                 \r");
        *ListCnt = 1;
    }
}


void ListProgram(const char *p, int all) {
    char b[STRINGSIZE];
    char *pp;
    int ListCnt = 1;

#if defined(__386__)
  GetConsoleSize();                                                 // this allows the user to change screen size anytime
#endif

    while(!(*p == 0 || *p == 0xff)) {                               // normally a LIST ends at the break so this is a safety precaution
        if(*p == T_NEWLINE) {
            p = llist(b, p);                                        // otherwise expand the line
            pp = b;
            while(*pp) {
                if(MMCharPos >= Option.Width) ListNewLine(&ListCnt, all);
                MMputchar(*pp++);
            }
            ListNewLine(&ListCnt, all);
            if(p[0] == 0 && p[1] == 0) break;                       // end of the listing ?
        }
    }
}



void cmd_continue(void) {
    if(*cmdline == tokenFOR) {
        if(forindex == 0) error("No FOR loop is in effect");
        nextstmt = forstack[forindex - 1].nextptr;
        return;
    }
    if(checkstring(cmdline, "DO")) {
        if(doindex == 0) error("No DO loop is in effect");
        nextstmt = dostack[doindex - 1].loopptr;
        return;
    }
    // must be a normal CONTINUE
    checkend(cmdline);
    if(CurrentLinePtr) error("Invalid in a program");
    if(ContinuePoint == NULL) error("Cannot continue");
    IgnorePIN = false;
    nextstmt = ContinuePoint;
}



void cmd_clear(void) {
    checkend(cmdline);
    ClearVars(0);
}



void cmd_goto(void) {
    if(isnamestart(*cmdline))
        nextstmt = findlabel(cmdline);                              // must be a label
    else
        nextstmt = findline(getinteger(cmdline), true);             // try for a line number
    IgnorePIN = false;

    CurrentLinePtr = nextstmt;
}



void cmd_if(void) {
    int r, i, testgoto, testelseif;
    char ss[3];                                                     // this will be used to split up the argument line
    const char *p, *tp;
    const char *rp = NULL;

    ss[0] = tokenTHEN;
    ss[1] = tokenELSE;
    ss[2] = 0;

    testgoto = false;
    testelseif = false;

retest_an_if:
    {                                                               // start a new block
        getargs(&cmdline, 20, ss);                                  // getargs macro must be the first executable stmt in a block

        if(testelseif && argc > 2) error("Unexpected text");

        // if there is no THEN token retry the test with a GOTO.  If that fails flag an error
        if(argc < 2 || *argv[1] != ss[0]) {
            if(testgoto) error("IF without THEN");
            ss[0] = tokenGOTO;
            testgoto = true;
            goto retest_an_if;
        }

        // allow for IF statements embedded inside this IF
        if(argc >= 3 && *argv[2] == cmdIF) argc = 3;                // this is IF xx=yy THEN IF ... so we want to evaluate only the first 3
        if(argc >= 5 && *argv[4] == cmdIF) argc = 5;                // this is IF xx=yy THEN cmd ELSE IF ... so we want to evaluate only the first 5

        if(argc == 4 || (argc == 5 && *argv[3] != ss[1])) ERROR_SYNTAX;

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
                if(*argv[1] == tokenGOTO) {
                    cmdline = argv[2];
                    cmd_goto();
                    return;
                } else if(isdigit(*argv[2])) {
                    nextstmt = findline(getinteger(argv[2]), true);
                } else {
                    if(argc == 5) {
                        // this is a full IF THEN ELSE and the statement we want to execute is between the THEN & ELSE
                        // this is handled by a special routine
                        execute_one_command(argv[2]);
                    } else {
                        // easy - there is no ELSE clause so just point the next statement pointer to the byte after the THEN token
                        for(p = cmdline; *p && *p != ss[0]; p++);   // search for the token
                        nextstmt = p + 1;                           // and point to the byte after
                    }
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
                        while(*tp && *tp != ss[0]) tp++;
                        if(*tp) tp++;                               // step over the THEN
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
                if(argc == 5) {
                    // there is an ELSE command
                    if(isdigit(*argv[4]))
                        // and it is just a number, so get it and find the line
                        nextstmt = findline(getinteger(argv[4]), true);
                    else {
                        // there is a statement after the ELSE clause  so just point to it (the byte after the ELSE token)
                        for(p = cmdline; *p && *p != ss[1]; p++);   // search for the token
                        nextstmt = p + 1;                           // and point to the byte after
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



void cmd_else(void) {
    int i;
    const char *p, *tp;

    // search for the next ENDIF and pass control to the following line
    i = 1; p = nextstmt;

    if (cmdtoken == cmdELSE) checkend(cmdline);

    while(1) {
        p = GetNextCommand(p, NULL, "No matching ENDIF");
        const CommandToken cmd = commandtbl_decode(p);
        if(cmd == cmdIF) {
            // found a nested IF command, we now need to determine if it is a single or multiline IF
            // search for a THEN, then check if only white space follows.  If so, it is multiline.
            tp = p + sizeof(CommandToken);
            while(*tp && *tp != tokenTHEN) tp++;
            if(*tp) tp++;                                           // step over the THEN
            skipspace(tp);
            if(*tp == 0 || *tp == '\'')                             // yes, only whitespace follows
                i++;                                                // count it as a nested IF
        }
        if(cmd == cmdENDIF || cmd == cmdEND_IF) i--;                // found an ENDIF so decrement our nested counter
        if(i == 0) break;                                           // found our matching ENDIF stmt
    }
    // found a matching ENDIF.  Step over it and continue with the statement after it
    skipelement(p);
    nextstmt = p;
}



void cmd_select(void) {
    int i, type;
    const char *p, *rp = NULL, *SaveCurrentLinePtr;
    void *v;
    MMFLOAT f = 0;
    MMINTEGER i64 = 0;
    char s[STRINGSIZE];

    // these are the tokens that we will be searching for
    // they are cached the first time this command is called

    type = T_NOTYPE;
    v = DoExpression(cmdline, &type);                               // evaluate the select case value
    type = TypeMask(type);
    if(type & T_NBR) f = *(MMFLOAT *)v;
    if(type & T_INT) i64 = *(MMINTEGER *)v;
    if(type & T_STR) Mstrcpy(s, (char *)v);

    // now search through the program looking for a matching CASE statement
    // i tracks the nesting level of any nested SELECT CASE commands
    SaveCurrentLinePtr = CurrentLinePtr;                            // save where we are because we will have to fake CurrentLinePtr to get errors reported correctly
    i = 1; p = nextstmt;
    while(1) {
        p = GetNextCommand(p, &rp, "No matching END SELECT");
        const CommandToken cmd = commandtbl_decode(p);

        if (cmd == cmdSELECT_CASE) i++;                             // found a nested SELECT CASE command, increase the nested count and carry on searching

        // is this a CASE stmt at the same level as this SELECT CASE.
        if (cmd == cmdCASE && i == 1) {
            int t;
            MMFLOAT ft, ftt;
            MMINTEGER i64t, i64tt;
            char *st, *stt;

            CurrentLinePtr = rp;                                    // and report errors at the line we are on
            p += sizeof(CommandToken) - 1;

            // loop through the comparison elements on the CASE line.  Each element is separated by a comma
            do {
                p++;
                skipspace(p);
                t = type;
                // check for CASE IS,  eg  CASE IS > 5  -or-  CASE > 5  and process it if it is
                // an operator can be >, <>, etc but it can also be a prefix + or - so we must not catch them
                if((SaveCurrentLinePtr = checkstring(p, "IS")) || ((tokentype(*p) & T_OPER) && !(*p == GetTokenValue("+") || *p == GetTokenValue("-")))) {
                    int o = 0;
                    if(SaveCurrentLinePtr) p += 2;
                    skipspace(p);
                    if(tokentype(*p) & T_OPER)
                        o = *p++ - C_BASETOKEN;                     // get the operator
                    else
                        ERROR_SYNTAX;
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
                if(*p == tokenTO) {                                 // is there is a TO keyword?
                    p++;
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
                if(((type & T_NBR) && f == ft) ||  ((type & T_INT) && i64 == i64t) ||  ((type & T_STR) && Mstrcmp(s, st) == 0)) {
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
            p += sizeof(CommandToken) - 1;
        }

        if (i == 0) {
            // found our matching END SELECT stmt.  Step over it and continue with the statement after it
            skipelement(p);
            nextstmt = p;
            CurrentLinePtr = SaveCurrentLinePtr;
            return;
        }
    }
}



// if we have hit a CASE or CASE ELSE we must search for a END SELECT at this level and resume at that point
void cmd_case(void) {
    int i;
    const char *p;

    // search through the program looking for a END SELECT statement
    // i tracks the nesting level of any nested SELECT CASE commands
    i = 1; p = nextstmt;
    while(1) {
        p = GetNextCommand(p, NULL, "No matching END SELECT");
        const CommandToken cmd = commandtbl_decode(p);

        if (cmd == cmdSELECT_CASE) i++;                             // found a nested SELECT CASE command, we now need to search for its END CASE

        if (cmd == cmdEND_SELECT) i--;                              // found an END SELECT so decrement our nested counter
        if(i == 0) {
            // found our matching END SELECT stmt.  Step over it and continue with the statement after it
            skipelement(p);
            nextstmt = p;
            break;
        }
    }
}



void cmd_input(void) {
    char s[STRINGSIZE];
    char *p, *sp, *tp;
    int i, fnbr;
    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",;");               // this is a macro and must be the first executable stmt

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
            MMPrintString(argv[0]);
            if(*argv[1] == ';') MMPrintString("? ");
            i = 2;
        } else {
            MMPrintString("? ");                                    // no prompt?  then just print the question mark
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
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        if(vartbl[VarIndex].type & T_STR) {
            if(strlen(s) > vartbl[VarIndex].size) error("String too long");
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



void cmd_tron(void) {
    checkend(cmdline);
    TraceOn = true;
}



void cmd_troff(void) {
    checkend(cmdline);
    TraceOn = false;
}



#if !defined(__mmb4l__)
void cmd_trace(void) {
    if(checkstring(cmdline, "ON"))
        TraceOn = true;
    else if(checkstring(cmdline, "OFF"))
        TraceOn = false;
#if !defined(MX170)
    else if(checkstring(cmdline, "LIST")) {
        int i;
        cmdline += 4;
        skipspace(cmdline);
        if(*cmdline == 0 || *cmdline =='\'')  //'
            i = TRACE_BUFF_SIZE - 1;
        else
            i = getint(cmdline, 0, TRACE_BUFF_SIZE - 1);
        i = TraceBuffIndex - i;
        if(i < 0) i += TRACE_BUFF_SIZE;
        while(i != TraceBuffIndex) {
            inpbuf[0] = '[';
            IntToStr(inpbuf + 1, CountLines(TraceBuff[i]), 10);
            strcat(inpbuf, "]");
            MMPrintString(inpbuf);
            if(++i >= TRACE_BUFF_SIZE) i = 0;
        }
    }
#endif
    else
        error("Unknown command");
}
#endif



// FOR command
void cmd_for(void) {
    int i, t, vlen, test;
    char ss[4];                                                     // this will be used to split up the argument line
    const char *p, *tp, *xp;
    void *vptr;
    char *vname, vtype;

    ss[0] = tokenEQUAL;
    ss[1] = tokenTO;
    ss[2] = tokenSTEP;
    ss[3] = 0;

    {                                                               // start a new block
        getargs(&cmdline, 7, ss);                                   // getargs macro must be the first executable stmt in a block
        if(argc < 5 || argc == 6 || *argv[1] != ss[0] || *argv[3] != ss[1]) error("FOR with misplaced = or TO");
        if(argc == 6 || (argc == 7 && *argv[5] != ss[2])) ERROR_SYNTAX;

        // get the variable name and trim any spaces
        vname = argv[0];
        if(*vname && *vname == ' ') vname++;
        while(*vname && vname[strlen(vname) - 1] == ' ') vname[strlen(vname) - 1] = 0;
        vlen = strlen(vname);
        vptr = findvar(argv[0], V_FIND);                            // create the variable
        if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        vtype = TypeMask(vartbl[VarIndex].type);
        if(vtype & T_STR) error("Invalid variable");                // sanity check

        // check if the FOR variable is already in the stack and remove it if it is
        // this is necessary as the program can jump out of the loop without hitting
        // the NEXT statement and this will eventually result in a stack overflow
        for(i = 0; i < forindex ;i++) {
            if(forstack[i].var == vptr && forstack[i].level == LocalIndex) {
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
                break;
            }
        }

        if(forindex == MAXFORLOOPS) error("Too many nested FOR loops");

        forstack[forindex].var = vptr;                              // save the variable index
        forstack[forindex].vartype = vtype;                         // save the type of the variable
        forstack[forindex].level = LocalIndex;                      // save the level of the variable in terms of sub/funs
        forindex++;                                                 // incase functions use for loops
        if(vtype & T_NBR) {
            *(MMFLOAT *)vptr = getnumber(argv[2]);                  // get the starting value for a float and save
            forstack[forindex - 1].tovalue.f = getnumber(argv[4]);  // get the to value and save
            if(argc == 7)
                forstack[forindex - 1].stepvalue.f = getnumber(argv[6]);// get the step value for a float and save
            else
                forstack[forindex - 1].stepvalue.f = 1.0;           // default is +1
        } else {
            *(MMINTEGER *)vptr = getinteger(argv[2]);               // get the starting value for an integer and save
            forstack[forindex - 1].tovalue.i = getinteger(argv[4]); // get the to value and save
            if(argc == 7)
                forstack[forindex - 1].stepvalue.i = getinteger(argv[6]);// get the step value for an integer and save
            else
                forstack[forindex - 1].stepvalue.i = 1;             // default is +1
        }
        forindex--;

        forstack[forindex].forptr = nextstmt + 1;                   // return to here when looping

        // now find the matching NEXT command
        t = 1; p = nextstmt;
        while(1) {
            p = GetNextCommand(p, &tp, "No matching NEXT");
            const CommandToken cmd = commandtbl_decode(p);
            if (cmd == cmdFOR) t++;                                 // count the FOR
            if (cmd == cmdNEXT) {                                   // is it NEXT
                xp = p + sizeof(CommandToken);                      // point to after the NEXT token
                while(*xp && strncasecmp(xp, vname, vlen)) xp++;    // step through looking for our variable
                if(*xp && !isnamechar(xp[vlen]))                    // is it terminated correctly?
                    t = 0;                                          // yes, found the matching NEXT
                else
                    t--;                                            // no luck, just decrement our stack counter
            }
            if(t == 0) {                                            // found the matching NEXT
                forstack[forindex].nextptr = p;                     // pointer to the start of the NEXT command
                break;
            }
        }

        // test the loop value at the start
        if(forstack[forindex].vartype & T_INT)
            test = (forstack[forindex].stepvalue.i >= 0 && *(MMINTEGER *)vptr > forstack[forindex].tovalue.i) || (forstack[forindex].stepvalue.i < 0 && *(MMINTEGER *)vptr < forstack[forindex].tovalue.i) ;
        else
            test = (forstack[forindex].stepvalue.f >= 0 && *(MMFLOAT *)vptr > forstack[forindex].tovalue.f) || (forstack[forindex].stepvalue.f < 0 && *(MMFLOAT *)vptr < forstack[forindex].tovalue.f) ;

        if(test) {
            // loop is invalid at the start, so go to the end of the NEXT command
            skipelement(p);                                         // find the command after the NEXT command
            nextstmt = p;                                           // this is where we will continue
        } else {
            forindex++;                                             // save the loop data and continue on with the command after the FOR statement
        }
    }
}



void cmd_next(void) {
    int i, vindex, test;
    void *vtbl[MAXFORLOOPS];
    int vcnt;
    const char *p;
    getargs(&cmdline, MAXFORLOOPS * 2, ",");                        // getargs macro must be the first executable stmt in a block

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

    error("Cannot find a matching FOR");

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



void cmd_exitfor(void) {
    if(forindex == 0) error("No FOR loop is in effect");
    nextstmt = forstack[--forindex].nextptr;
    checkend(cmdline);
    skipelement(nextstmt);
}



void cmd_exit(void) {
    if(doindex == 0) error("No DO loop is in effect");
    nextstmt = dostack[--doindex].loopptr;
    checkend(cmdline);
    skipelement(nextstmt);
}



#if !defined(__mmb4l__)
void cmd_error(void) {
    char *s;
    if(*cmdline && *cmdline != '\'') {
        s = getCstring(cmdline);
        CurrentLinePtr = NULL; // suppress printing the line that caused the issue
        error(s);
    }
    else
        error("");
}
#endif



void cmd_randomize(void) {
    int i;
    i = getint(cmdline, 0, INT_MAX);
    srand(i);
}



// this is the Sub or Fun command
// it simply skips over text until it finds the end of it
void cmd_subfun(void) {
    const char *p;
    CommandToken returntoken;
    CommandToken errtoken;

    if(gosubindex != 0) error("No matching END declaration");       // We have hit a SUB/FUNCTION while in another SUB or FUN.
                                                                    // This can also happen if we GOSUB somewhere and then
                                                                    // encounter SUB/FUNCTION before we RETURN.
    if (cmdtoken == cmdSUB) {
        returntoken = cmdEND_SUB;
        errtoken = cmdEND_FUNCTION;
    } else {
        returntoken = cmdEND_FUNCTION;
        errtoken = cmdEND_SUB;
    }
    p = nextstmt;
    while(1) {
        p = GetNextCommand(p, NULL, "No matching END declaration");
        const CommandToken cmd = commandtbl_decode(p);
        if (cmd == cmdSUB || cmd == cmdFUN || cmd == errtoken) error("No matching END declaration");
        if (cmd == returntoken) {                                   // found the next return
            skipelement(p);
            nextstmt = p;                                           // point to the next command
            break;
        }
    }
}



void cmd_gosub(void) {
    if(gosubindex >= MAXGOSUB) error("Too many nested GOSUB");
    const char *return_to = nextstmt;
    if(isnamestart(*cmdline))
        nextstmt = findlabel(cmdline);                              // must be a label
    else
        nextstmt = findline(getinteger(cmdline), true);             // try for a line number
    IgnorePIN = false;

    // Do not update the interpreter state until successfully finding
    // the target line/label.
    errorstack[gosubindex] = CurrentLinePtr;
    gosubstack[gosubindex++] = return_to;
    LocalIndex++;
    CurrentLinePtr = nextstmt;
}



void cmd_return(void) {
    checkend(cmdline);
    if(gosubindex == 0 || gosubstack[gosubindex - 1] == NULL) error("Nothing to return to");
    ClearVars(LocalIndex--);                                        // delete any local variables
    TempMemoryIsChanged = true;                                     // signal that temporary memory should be checked
    nextstmt = gosubstack[--gosubindex];                            // return to the caller
    CurrentLinePtr = errorstack[gosubindex];
}



void cmd_endfun(void) {
    checkend(cmdline);
    if(gosubindex == 0 || gosubstack[gosubindex - 1] != NULL) error("Nothing to return to");
    nextstmt = "\0\0\0";                                            // now terminate this run of ExecuteProgram()
}



void cmd_lineinput(void) {
    char *vp;
    int i, fnbr;
    getargs(&cmdline, 3, ",;");                                     // this is a macro and must be the first executable stmt
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
    if(vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
    if(!(vartbl[VarIndex].type & T_STR)) error("Invalid variable");
    MMgetline(fnbr, inpbuf);                                        // get the input line
    if(strlen(inpbuf) > vartbl[VarIndex].size) error("String too long");
    strcpy(vp, inpbuf);
    CtoM(vp);                                                       // convert to a MMBasic string
}



#if !defined(__mmb4l__)
void cmd_on(void) {
    int r;
    char ss[4];                                                     // this will be used to split up the argument line
    char *p;

 #if !defined(__386__)
    // first check if this is:   ON KEY location
    if((p = checkstring(cmdline, "KEY")) != NULL) {
        if(*p == '0' && !isdigit(*(p+1)))
            OnKeyGOSUB = NULL;                                      // the program wants to turn the interrupt off
        else {
            OnKeyGOSUB = GetIntAddress(p);                          // get a pointer to the interrupt routine
            InterruptUsed = true;
        }
        return;
    }
#endif
    p = checkstring(cmdline, "ERROR");
    if(p) {
        if(checkstring(p, "ABORT")) {
            OptionErrorSkip = 0;
            return;
        }
        MMerrno = 0;                                                // clear the error flags
        *MMErrMsg = 0;
        if(checkstring(p, "CLEAR")) return;
        if(checkstring(p, "IGNORE")) {
            OptionErrorSkip = -1;
            return;
        }
        if((p = checkstring(p, "SKIP"))) {
            if(*p == 0 || *p == '\'')
                OptionErrorSkip = 2;
            else
                OptionErrorSkip = getint(p, 1, 10000) + 1;
            return;
        }
        ERROR_SYNTAX;
    }

    // if we got here the command must be the traditional:  ON nbr GOTO|GOSUB line1, line2,... etc

    ss[0] = tokenGOTO;
    ss[1] = tokenGOSUB;
    ss[2] = ',';
    ss[3] = 0;
    {                                                               // start a new block
        getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ss);             // getargs macro must be the first executable stmt in a block
        if(argc < 3 || !(*argv[1] == ss[0] || *argv[1] == ss[1])) ERROR_SYNTAX;
        if(argc%2 == 0) ERROR_SYNTAX;

        r = getint(argv[0], 0, 255);                                // evaluate the expression controlling the statement
        if(r == 0 || r > argc/2) return;                            // microsoft say that we just go on to the next line

        if(*argv[1] == ss[1]) {
            // this is a GOSUB, same as a GOTO but we need to first push the return pointer
            if(gosubindex >= MAXGOSUB) error("Too many nested GOSUB");
            errorstack[gosubindex] = CurrentLinePtr;
            gosubstack[gosubindex++] = nextstmt;
            LocalIndex++;
        }

        if(isnamestart(*argv[r*2]))
            nextstmt = findlabel(argv[r*2]);                        // must be a label
        else
            nextstmt = findline(getinteger(argv[r*2]), true);       // try for a line number
    }
    IgnorePIN = false;
}
#endif



const char *SetValue(const char *p, int t, void *v) {
    MMFLOAT f;
    MMINTEGER i64;
    char *s;
    char TempCurrentSubFunName[MAXVARLEN + 2];                      // requires extra byte to store optional type suffix.

    strcpy(TempCurrentSubFunName, CurrentSubFunName);               // save the current sub/fun name
    if(t & T_STR) {
        p = evaluate(p, &f, &i64, &s, &t, true);
        Mstrcpy(v, s);
    }
    else if(t & T_NBR) {
        p = evaluate(p, &f, &i64, &s, &t, false);
        if(t & T_NBR)
            (*(MMFLOAT *)v) = f;
        else
            (*(MMFLOAT *)v) = (MMFLOAT)i64;
    } else {
        p = evaluate(p, &f, &i64, &s, &t, false);
        if(t & T_INT)
            (*(MMINTEGER *)v) = i64;
        else
            (*(MMINTEGER *)v) = FloatToInt64(f);
    }
    strcpy(CurrentSubFunName, TempCurrentSubFunName);               // restore the current sub/fun name
    return p;
}



// define a variable
// DIM [AS INTEGER|FLOAT|STRING] var[(d1 [,d2,...]] [AS INTEGER|FLOAT|STRING] [, ..., ...]
// LOCAL also uses this function the routines only differ in that LOCAL can only be used in a sub/fun
void cmd_dim(void) {
    int i, j, k, type, typeSave, ImpliedType = 0, VIndexSave, StaticVar = false;
    char chSave, *chPosit;
    char VarName[STRINGSIZE];
    void *v, *tv;

    if (*cmdline == tokenAS) cmdline++;                             // this means that we can use DIM AS INTEGER a, b, etc
    const char *pconst = CheckIfTypeSpecified(cmdline, &type, true);  // check for DIM FLOAT A, B, ...
    ImpliedType = type;
    {                                                               // getargs macro must be the first executable stmt in a block
        getargs(&pconst, (MAX_ARG_COUNT * 2) - 1, ",");
        if((argc & 0x01) == 0) ERROR_SYNTAX;

        // 'p' will be pointing into the items of argv[] which we know are not
        // constants so we can cast away const-ness as necessary to remove
        // warnings.
        char *p;
        for(i = 0; i < argc; i += 2) {
            p = (char *) skipvar(argv[i], false);                   // point to after the variable
            while(!(*p == 0 || *p == tokenAS || *p == '\'' || *p == tokenEQUAL))
                p++;                                                // skip over a LENGTH keyword if there and see if we can find "AS"
            chSave = *p; chPosit = p; *p = 0;                       // save the char then terminate the string so that LENGTH is evaluated correctly
            if(chSave == tokenAS) {                                 // are we using Microsoft syntax (eg, AS INTEGER)?
                if(ImpliedType & T_IMPLIED) error("Type specified twice");
                p++;                                                // step over the AS token
                p = (char *) CheckIfTypeSpecified(p, &type, true);  // and get the type
                if(!(type & T_IMPLIED)) error("Variable type");
            }

            if(cmdtoken == GetCommandValue("Local")) {
                if(LocalIndex == 0) error("Invalid here");
                type |= V_LOCAL;                                    // local if defined in a sub/fun
            }

            if (cmdtoken == GetCommandValue("Static")) {
                if (LocalIndex == 0) error("Invalid here");
                // Create a unique global name by prefixing variable name with sub/fun name.
                if (*CurrentInterruptName) {
                    strcpy(VarName, CurrentInterruptName);          // we must be in an interrupt sub
                } else {
                    strcpy(VarName, CurrentSubFunName);             // normal sub/fun
                }
                for (k = 1; k <= MAXVARLEN; k++) {
                    if (!isnamechar(VarName[k])) {
                        VarName[k] = 0;                             // terminate the string on a non valid char
                        break;
                    }
                }
                if (FAILED(cstring_cat(VarName, argv[i], sizeof(VarName)))) ERROR_LINE_LENGTH;
                StaticVar = true;
            } else {
                if (FAILED(cstring_cpy(VarName, argv[i], sizeof(VarName)))) ERROR_LINE_LENGTH;
            }

            v = findvar(VarName, type | V_NOFIND_NULL);             // check if the variable exists
            typeSave = type;
            VIndexSave = VarIndex;
            if(v == NULL) {                                         // if not found
                v = findvar(VarName, type | V_FIND | V_DIM_VAR);    // create the variable
                type = TypeMask(vartbl[VarIndex].type);
                VIndexSave = VarIndex;
                *chPosit = chSave;                                  // restore the char previously removed
                if(vartbl[VarIndex].dims[0] == -1) error("Array dimensions");
                if(vartbl[VarIndex].dims[0] > 0) {
                    DimUsed = true;                                 // prevent OPTION BASE from being used
                    v = vartbl[VarIndex].val.s;
                }
                while(*p && *p != '\'' && tokenfunction(*p) != op_equal) p++;   // search through the line looking for the equals sign
                if(tokenfunction(*p) == op_equal) {
                    p++;                                            // step over the equals sign
                    skipspace(p);
                    if(vartbl[VarIndex].dims[0] > 0 && *p == '(') {
                        // calculate the overall size of the array
                        for(j = 1, k = 0; k < MAXDIM && vartbl[VIndexSave].dims[k]; k++) {
                            j *= (vartbl[VIndexSave].dims[k] + 1 - OptionBase);
                        }
                        do {
                            p++;                                    // step over the opening bracket or terminating comma
                            p = (char *) SetValue(p, type, v);
                            if(type & T_STR) v = (char *)v + vartbl[VIndexSave].size + 1;
                            if(type & T_NBR) v = (char *)v + sizeof(MMFLOAT);
                            if(type & T_INT) v = (char *)v + sizeof(MMINTEGER);
                            skipspace(p); j--;
                        } while(j > 0 && *p == ',');
                        if(*p != ')') error("Number of initialising values");
                        if(j != 0) error("Number of initialising values");
                    } else
                        SetValue(p, type, v);
                }
                type = ImpliedType;
            } else {
                if(!StaticVar) error("$ already declared", VarName);
            }

            // if it is a STATIC var create a local var pointing to the global var
            if(StaticVar) {
                tv = findvar(argv[i], typeSave | V_LOCAL | V_NOFIND_NULL);                        // check if the local variable exists
                if(tv != NULL) error("$ already declared", argv[i]);
                tv = findvar(argv[i], typeSave | V_LOCAL | V_FIND | V_DIM_VAR);                   // create the variable
                if(vartbl[VIndexSave].dims[0] > 0 || (vartbl[VIndexSave].type & T_STR)) {
                    FreeMemory(tv);                                                               // we don't need the memory allocated to the local
                    vartbl[VarIndex].val.s = vartbl[VIndexSave].val.s;                            // point to the memory of the global variable
                } else
                    vartbl[VarIndex].val.ia = &(vartbl[VIndexSave].val.i);                        // point to the data of the variable
                vartbl[VarIndex].type = vartbl[VIndexSave].type | T_PTR;                          // set the type to a pointer
                vartbl[VarIndex].size = vartbl[VIndexSave].size;                                  // just in case it is a string copy the size
                for(j = 0; j < MAXDIM; j++) vartbl[VarIndex].dims[j] = vartbl[VIndexSave].dims[j];// just in case it is an array copy the dimensions
            }
        }
    }
}



void cmd_const(void) {
    const char *p;
    void *v;
    int i, type;

    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");                // getargs macro must be the first executable stmt in a block
    if((argc & 0x01) == 0) ERROR_SYNTAX;

    for(i = 0; i < argc; i += 2) {
        p = skipvar(argv[i], false);                                // point to after the variable
        skipspace(p);
        if(tokenfunction(*p) != op_equal) ERROR_SYNTAX;             // must be followed by an equals sign
        p++;                                                        // step over the equals sign
        type = T_NOTYPE;
        v = DoExpression(p, &type);                                 // evaluate the constant's value
        type = TypeMask(type);
        type |= V_FIND | V_DIM_VAR | T_CONST | T_IMPLIED;
        if(LocalIndex != 0) type |= V_LOCAL;                        // local if defined in a sub/fun
        findvar(argv[i], type);                                     // create the variable
        if(vartbl[VarIndex].dims[0] != 0) error("Invalid constant");
        if(TypeMask(vartbl[VarIndex].type) != TypeMask(type)) error("Invalid constant");
        else {
            if(type & T_NBR) vartbl[VarIndex].val.f = *(MMFLOAT *)v; // and set its value
            if(type & T_INT) vartbl[VarIndex].val.i = *(MMINTEGER *)v;
            if(type & T_STR) Mstrcpy(vartbl[VarIndex].val.s, (char *)v);
        }
    }
}



void cmd_auto(void) {
#if 0
    getargs(&cmdline, 3, ",");
    if(CurrentLinePtr) error("Invalid in a program");
    if(argc == 0) {                                                                                                        // if there are no arguments just stuff the lines into program memory
        ClearRuntime();                                                                                                // clear any leftovers from the previous program
        while(1) {                                                                                                        // while forever (actually until CTRL-C)
            *inpbuf = 0;                                                                                        // clear the input buffer
            EditInputLine();                                                                                // get the input
            tokenise(false);                                                                                // turn into executable code
            AddProgramLine(true);                                   // add to program memory
        }
    }

    if(argc == 2) error("Invalid syntax");
    if(argc >= 1) autoNext = getinteger(argv[0]);
    if(argc == 3) autoIncr = getinteger(argv[2]);
    autoOn = true;
#endif
}



/***********************************************************************************************
utility functions used by the various commands
************************************************************************************************/



// utility function used by llist() below
// it copys a command or function honouring the case selected by the user
void strCopyWithCase(char *d, const char *s) {
    if(Option.Listcase == CONFIG_LOWER) {
        while(*s) *d++ = tolower(*s++);
    } else if(Option.Listcase == CONFIG_UPPER) {
        while(*s) *d++ = toupper(*s++);
    } else {
        while(*s) *d++ = *s++;
    }
    *d = 0;
}



// list a line into a buffer (b) given a pointer to the beginning of the line (p).
// the returned string is a C style string (terminated with a zero)
// this is used by cmd_list(), cmd_edit() and cmd_xmodem()
const char *llist(char *b, const char *p) {
    int i, firstnonwhite = true;
    char *b_start = b;

    while(1) {
        if(*p == T_NEWLINE) {
            p++;
            firstnonwhite = true;
            continue;
        }

        if(*p == T_LINENBR) {
            i = (((p[1]) << 8) | (p[2]));                           // get the line number
            p += 3;                                                 // and step over the number
            IntToStr(b, i, 10);
            b += strlen(b);
            if(*p != ' ') *b++ = ' ';
        }

        if(*p == T_LABEL) {                                         // got a label
            for(i = p[1], p += 2; i > 0; i--)
                *b++ = *p++;                                        // copy to the buffer
            *b++ = ':';                                             // terminate with a colon
            if(*p && *p != ' ') *b++ = ' ';                         // and a space if necessary
            firstnonwhite = true;
        }                                                           // this deliberately drops through in case the label is the only thing on the line

        if(*p >= C_BASETOKEN) {
            if(firstnonwhite) {
                const CommandToken cmd = commandtbl_decode(p);
                if (cmd == cmdLET)
                    *b = 0;                                         // use nothing if it LET
                else {
                    strCopyWithCase(b, commandname(cmd));           // expand the command (if it is not LET)
                    b += strlen(b);                                 // update pointer to the end of the buffer
                    if(isalpha(*(b - 1))) *b++ = ' ';               // add a space to the end of the command name
                }
                firstnonwhite = false;
                p += sizeof(CommandToken);
            } else {                                                // not a command so must be a token
                strCopyWithCase(b, tokenname(*p));                  // expand the token
                b += strlen(b);                                     // update pointer to the end of the buffer
                if(*p == tokenTHEN || *p == tokenELSE)
                    firstnonwhite = true;
                else
                    firstnonwhite = false;
                p++;
            }
            continue;
        }

        // hey, an ordinary char, just copy it to the output
        if(*p) {
            *b = *p;                                                // place the char in the buffer
            if(*p != ' ') firstnonwhite = false;
            p++;  b++;                                              // move the pointers
            continue;
        }

        // at this point the char must be a zero
        // zero char can mean both a separator or end of line
        if(!(p[1] == T_NEWLINE || p[1] == 0)) {
            *b++ = ':';                                             // just a separator
            firstnonwhite = true;
            p++;
            continue;
        }

        // must be the end of a line - so return to the caller
        while(*(b-1) == ' ' && b > b_start) --b;                    // eat any spaces on the end of the line
        *b = 0;                                                     // terminate the output buffer
        return ++p;
    } // end while
}



void execute_one_command(char *p) {
    CheckAbort();
    targ = T_CMD;
    skipspace(p);                                                   // skip any whitespace
    if (p[0]>= C_BASETOKEN && p[1]>=C_BASETOKEN) {
        const CommandToken cmd = commandtbl_decode(p);
        if (cmd == cmdWHILE || cmd== cmdDO || cmd == cmdFOR) error("Invalid inside THEN ... ELSE") ;
        cmdtoken = cmd;
        cmdline = p + sizeof(CommandToken);
        skipspace(cmdline);
        commandtbl[cmd].fptr(); // execute the command
    } else {
        if(!isnamestart(*p)) error("Invalid character");
        int i = FindSubFun(p, kSub);                                // find a subroutine.
        if(i >= 0)                                                  // >= 0 means it is a user defined command
            DefinedSubFun(false, p, i, NULL, NULL, NULL, NULL);
        else
            error("Unknown command");
    }
    ClearTempMemory();                                              // at the end of each command we need to clear any temporary string vars
}
