/*-*****************************************************************************

MMBasic for Linux (MMB4L)

Commands.c

Copyright 2011-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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



void cmd_tron(void) {
    checkend(cmdline);
    TraceOn = true;
}



void cmd_troff(void) {
    checkend(cmdline);
    TraceOn = false;
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



void cmd_randomize(void) {
    int i;
    i = getint(cmdline, 0, INT_MAX);
    srand(i);
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
