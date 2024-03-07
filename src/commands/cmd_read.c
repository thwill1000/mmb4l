/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_read.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
#include "../core/commandtbl.h"

#include <assert.h>
#include <string.h>

#define ERROR_NO_DATA             error_throw_ex(kError, "No DATA to read")
#define ERROR_NOTHING_TO_RESTORE  error_throw_ex(kError, "Nothing to restore")
#define ERROR_TOO_MANY_SAVES      error_throw_ex(kError, "Too many saves")

// This odd number allows us to match the behavious of MMB4W.
#define READ_STACK_SIZE  49

static DataReadPointer cmd_read_stack[READ_STACK_SIZE] = { 0 };
static DataReadPointer* cmd_read_sp = cmd_read_stack;

void cmd_read_clear_cache() {
    assert(sizeof(DataReadPointer) == sizeof(MMINTEGER));
    memset(cmd_read_stack, 0, sizeof(cmd_read_stack));
    cmd_read_sp = cmd_read_stack;
}

static void cmd_read_save(void) {
    if (cmd_read_sp == cmd_read_stack + READ_STACK_SIZE) ERROR_TOO_MANY_SAVES;
    cmd_read_sp->next_line_offset = NextDataLine - ProgMemory;
    cmd_read_sp->next_data = NextData;
    cmd_read_sp++;
}

static void cmd_read_restore(void) {
    if (cmd_read_sp == cmd_read_stack) ERROR_NOTHING_TO_RESTORE;
    cmd_read_sp--;
    NextDataLine = ProgMemory + cmd_read_sp->next_line_offset;
    NextData = cmd_read_sp->next_data;
}

void cmd_read_data(void) {
    int i, len;
    const char *p, *lineptr = NULL;
    char *vtbl[MAX_ARG_COUNT];
    int vtype[MAX_ARG_COUNT];
    int vsize[MAX_ARG_COUNT];
    int vcnt, vidx;
    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");                // getargs macro must be the first executable stmt in a block

    if (argc == 0) ERROR_SYNTAX;

    // step through the arguments and save the pointer and type
    for(vcnt = i = 0; i < argc; i++) {
        if(i & 0x01) {
            if (*argv[i] != ',') ERROR_SYNTAX;
        }
        else {
            vtbl[vcnt] = findvar(argv[i], V_FIND);
            if(vartbl[VarIndex].type & T_CONST) ERROR_CANNOT_CHANGE_A_CONSTANT;
            vtype[vcnt] = TypeMask(vartbl[VarIndex].type);
            vsize[vcnt] = vartbl[VarIndex].size;
            vcnt++;
        }
    }

    // setup for a search through the whole memory
    vidx = 0;
    int datatoken = GetCommandValue("Data");
    p = lineptr = NextDataLine;
    if (*p == 0xff) ERROR_NO_DATA;  // error if there is no program

  // search looking for a DATA statement.  We keep returning to this point until all the data is found
search_again:
    while(1) {
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0 || *p == 0xff) ERROR_NO_DATA;                    // end of the program and we still need more data
        if(*p == T_NEWLINE) lineptr = p++;
        if(*p == T_LINENBR) p += 3;
        skipspace(p);
        if(*p == T_LABEL) {                                         // if there is a label here
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }
        if(*p == datatoken) break;                                  // found a DATA statement
        while(*p) p++;                                              // look for the zero marking the start of the next element
    }
    NextDataLine = lineptr;
    p++;                                                            // step over the token
    skipspace(p);
    if(!*p || *p == '\'') { CurrentLinePtr = lineptr; ERROR_NO_DATA; }

        // we have a DATA statement, first split the line into arguments
        {                                                           // new block, the getargs macro must be the first executable stmt in a block
        getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");
        if((argc & 1) == 0) { CurrentLinePtr = lineptr; ERROR_SYNTAX; }
        // now step through the variables on the READ line and get their new values from the argument list
        // we set the line number to the number of the DATA stmt so that any errors are reported correctly
        while(vidx < vcnt) {
            // check that there is some data to read if not look for another DATA stmt
            if(NextData > argc) {
                skipline(p);
                NextData = 0;
                goto search_again;
            }
            // x = CurrentLinePtr;
            CurrentLinePtr = lineptr;
            if(vtype[vidx] & T_STR) {
                char *p1, *p2;
                if(*argv[NextData] == '"') {                               // if quoted string
                    for(len = 0, p1 = vtbl[vidx], p2 = argv[NextData] + 1; *p2 && *p2 != '"'; len++, p1++, p2++) {
                       *p1 = *p2;                                   // copy up to the quote
                    }
                } else {                                            // else if not quoted
                    for(len = 0, p1 = vtbl[vidx], p2 = argv[NextData]; *p2 && *p2 != '\'' ; len++, p1++, p2++) {
                        if(*p2 < 0x20 || *p2 >= 0x7f) ERROR_INVALID_CHARACTER;
                        *p1 = *p2;                                  // copy up to the comma
                    }
                }
                if (len > vsize[vidx]) ERROR_STRING_TOO_LONG;
                *p1 = 0;                                            // terminate the string
                CtoM(vtbl[vidx]);                                   // convert to a MMBasic string
            }
            else if(vtype[vidx] & T_INT)
                *((long long int *)vtbl[vidx]) = getinteger(argv[NextData]); // much easier if integer variable
            else
                *((MMFLOAT *)vtbl[vidx]) = getnumber(argv[NextData]);      // same for numeric variable

            vidx++;
            NextData += 2;
        }
    }
}

void cmd_read(void) {
    if (checkstring(cmdline, "SAVE")) {
        cmd_read_save();
    } else if (checkstring(cmdline, "RESTORE")) {
        cmd_read_restore();
    } else {
        cmd_read_data();
    }
}
