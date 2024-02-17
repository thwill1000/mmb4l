/*-*****************************************************************************

MMBasic for Linux (MMB4L)

MMBasic.c

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

// Provides the core functions used in MMBasic. These include parsing the command line and converting the key
// words into tokens, storage and management of the program in memory, storage and management of variables,
// the expression execution engine and other useful functions.

#include "../Hardware_Includes.h"
#include "MMBasic.h"
#include "Commands.h"
#include "commandtbl.h"
#include "funtbl.h"
#include "tokentbl.h"
#include "vartbl.h"
#include "../common/graphics.h"
#include "../common/parse.h"
#include "../common/utility.h"

#include <assert.h>

extern int ListCnt;
extern int MMCharPos;

// these are initialised at startup
int CommandTableSize, TokenTableSize;

int VarIndex;                                                       // Global set by findvar after a variable has been created or found
int LocalIndex;                                                     // used to track the level of local variables
#if !defined(__mmb4l__)
char OptionExplicit;                                                // used to force the declaration of variables before their use
char DefaultType;                                                   // the default type if a variable is not specifically typed
#endif

                                                                    // require extra byte to store optional type suffix
char CurrentSubFunName[MAXVARLEN + 2];                              // the name of the current sub or fun
char CurrentInterruptName[MAXVARLEN + 2];                           // the name of the current interrupt function

jmp_buf mark;                                                       // longjump to recover from an error and abort
jmp_buf ErrNext;                                                    // longjump to recover from an error and continue
char inpbuf[INPBUF_SIZE];                                           // used to store user keystrokes until we have a line
char tknbuf[TKNBUF_SIZE];                                           // used to store the tokenised representation of the users input line

const char DIGIT_CHARS[256] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x10
        0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0, //0x20
        1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0, //0x30
        0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0, //0x40
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x50
        0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0, //0x60
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x70
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x80
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0x90
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xA0
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xB0
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xC0
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xD0
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0xE0
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  //0xF0
};

int NextData;                                                       // used to track the next item to read in DATA & READ stmts
const char *NextDataLine;                                           // used to track the next line to read in DATA & READ stmts
#if !defined(__mmb4l__)
int OptionBase;                                                     // track the state of OPTION BASE
#endif



///////////////////////////////////////////////////////////////////////////////////////////////
// Global information used by operators and functions
//
int targ;                                                           // the type of the returned value
MMFLOAT farg1, farg2, fret;                                         // the two float arguments and returned value
MMINTEGER iarg1, iarg2, iret;                                       // the two integer arguments and returned value
char *sarg1, *sarg2, *sret;                                         // the two string arguments and returned value

////////////////////////////////////////////////////////////////////////////////////////////////
// Global information used by functions
// functions use targ, fret and sret as defined for operators (above)
const char *ep;                                                     // pointer to the argument to the function terminated with a zero byte.
                                                                    // it is NOT trimmed of spaces

////////////////////////////////////////////////////////////////////////////////////////////////
// Global information used by commands
//
CommandToken cmdtoken;                                              // Token number of the command
const char *cmdline;                                                // Command line terminated with a zero char and trimmed of spaces
const char *nextstmt;                                               // Pointer to the next statement to be executed.
const char *CurrentLinePtr;                                         // Pointer to the current line (used in error reporting)
const char *ContinuePoint;                                          // Where to continue from if using the continue statement


/////////////////////////////////////////////////////////////////////////////////////////////////
// Functions only used within MMBasic.c
//
void getexpr(char *);
void checktype(int *, int);
const char *getvalue(const char *p, MMFLOAT *fa, MMINTEGER *ia, char **sa, int *oo, int *ta);


/********************************************************************************************************************************************
 Program management
 Includes the routines to initialise MMBasic, start running the interpreter, and to run a program in memory
*********************************************************************************************************************************************/

// Initialise MMBasic
void InitBasic(void) {
    DefaultType = T_NBR;
    commandtbl_init();
    tokentbl_init();
    vartbl_init();
    ClearProgram();
}



// run a program
// this will continuously execute a program until the end (marked by TWO zero chars)
// the argument p must point to the first line to be executed
// We need to suppress a spurious(?) warning about 'p' being clobbered by setjmp().
#pragma GCC diagnostic push
#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wclobbered"
#endif
void ExecuteProgram(const char *p) {
    int i;
    int SaveLocalIndex = 0;
    jmp_buf SaveErrNext;                                            // we call ExecuteProgram() recursively so we need
    memcpy(SaveErrNext, ErrNext, sizeof(jmp_buf));                  // to store/restore old jump buffer between calls
    skipspace(p);                                                   // just in case, skip any whitespace
    while(1) {
        if(*p == 0) p++;                                            // step over the zero byte marking the beginning of a new element
        if(*p == T_NEWLINE) {
            CurrentLinePtr = p;                                     // and pointer to the line for error reporting
#if !defined(MX170)
            TraceBuff[TraceBuffIndex] = p;                          // used by TRACE LIST
            if(++TraceBuffIndex >= TRACE_BUFF_SIZE) TraceBuffIndex = 0;
#endif
            assert(p < (char *) (ProgMemory + PROG_FLASH_SIZE));
            if (TraceOn) {
#if defined(__mmb4l__)
                // Copied from the CMM2,
                // looks like it has duplication with cmd_trace.c#TraceLines()
                char buf[STRINGSIZE], buff[10];
                MMPrintString("[");
                memcpy(buf, p, STRINGSIZE);
                char *ename, *cpos = NULL;
                i = 0;
                while (!(buf[i] == 0 && buf[i + 1] == 0)) i++;
                while (i > 0) {
                    if (buf[i] == '|') cpos = &buf[i];
                    i--;
                }
                if (cpos != NULL) {
                    if ((ename = strchr(cpos, ',')) != NULL) {
                        *ename = 0;
                        cpos++;
                        ename++;
                        if (*cpos == '\'') cpos++;
                        MMPrintString(cpos);
                        MMPrintString(":");
                        MMPrintString(ename);
                    } else {
                        cpos++;
                        IntToStr(buff, atoi(cpos), 10);
                        MMPrintString(buff);
                    }
                }
                MMPrintString("]");
#else
                inpbuf[0] = '[';
                IntToStr(inpbuf + 1, CountLines(p), 10);
                strcat(inpbuf, "]");
                MMPrintString(inpbuf);
#endif
                uSec(1000);
            }
            p++;                                                    // and step over the token
        }
        if(*p == T_LINENBR) p += 3;                                 // and step over the number
        skipspace(p);                                               // and skip any trailing white space
        if(p[0] == T_LABEL) {                                       // got a label
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }

        if(*p) {                                                    // if p is pointing to a command
            nextstmt = cmdline = p + sizeof(CommandToken);
            skipspace(cmdline);
            skipelement(nextstmt);
            if(*p && *p != '\'') {                                  // ignore a comment line
                SaveLocalIndex = LocalIndex;                        // save this if we need to cleanup after an error

                if (setjmp(ErrNext) == 0) {                         // return to the else leg of this if error and OPTION ERROR SKIP/IGNORE is in effect
                    if (p[0] >= C_BASETOKEN && p[1] >= C_BASETOKEN) {
                        cmdtoken = commandtbl_decode(p);
                        targ = T_CMD;
                        commandtbl[cmdtoken].fptr();                // execute the command
                    } else {
                        if (!isnamestart(*p)) error("Invalid character: %", (int)(*p));
                        i = FindSubFun(p, kSub);                    // it could be a defined command (subroutine)
                        if(i >= 0) {                                // >= 0 means it is a user defined command
                            DefinedSubFun(false, p, i, NULL, NULL, NULL, NULL);
                        }
                        else
                            error("Unknown command");
                    }
                } else {
                    LocalIndex = SaveLocalIndex;                    // restore so that we can clean up any memory leaks
                    ClearTempMemory();
                }

                if(OptionErrorSkip > 0) OptionErrorSkip--;          // if OPTION ERROR SKIP decrement the count - we do not error if it is greater than zero
                if(TempMemoryIsChanged) ClearTempMemory();          // at the end of each command we need to clear any temporary string vars
                CheckAbort();
                check_interrupt();                                  // check for an MMBasic interrupt and handle it
            }
            p = nextstmt;
        }

        if ((p[0] == 0 && p[1] == 0) || (p[0] == 0xff && p[1] == 0xff)) {
            break;                                                  // the end of the program is marked by TWO zero chars, empty flash by two 0xff
        }
    }

    memcpy(ErrNext, SaveErrNext, sizeof(jmp_buf));                  // restore jump buffer
}
#pragma GCC diagnostic pop


/********************************************************************************************************************************************
 Code associated with processing user defined subroutines and functions
********************************************************************************************************************************************/

/**
 * @brief  Formats error message from a call to AddFunction().
 */
static const char *FormatAddFunctionError(MmResult result, FunType type) {
    switch (result) {
        case kDuplicateFunction:
            if (type == kLabel) return "Duplicate label";
            break;
        case kInvalidName:
            switch (type) {
                case kFunction:
                    return "Invalid function name";
                case kLabel:
                    // Does not seem to be possible;
                    // the tokeniser doesn't recognise it as a label.
                    return "Invalid label";
                default:
                    return "Invalid subroutine name";
            }
            break;
        case kNameTooLong:
            switch (type) {
                case kFunction:
                    return "Function name too long";
                case kLabel:
                    // Does not seem to be possible;
                    // the tokeniser doesn't recognise it as a label.
                    return "Label too long";
                default:
                    return "Subroutine name too long";
            }
            break;
        default:
            break;
    }

    return mmresult_to_string(result);
}

/**
 * @brief  Adds a function, label or subroutine to the function table.
 */
static MmResult AddFunction(const char **p, FunType type, const char *addr) {
    char name[MAXVARLEN + 1];
    MmResult result = parse_name(p, name);
    if (SUCCEEDED(result)) {
        int fun_idx;
        result = funtbl_add(name, type, addr, &fun_idx);
    }
    return result;
}

/**
 * @brief  Populates the function table by searching ProgMemory for functions,
 *         labels and subroutines.
 */
static void PrepareFunctionTable(bool abort_on_error) {
    const char *p = ProgMemory;
    CurrentLinePtr = NULL;

    funtbl_clear();

    for (;;) {

        // If we are not at the start of a line then look for the '/0' marking
        // the start of a statement and step over it.
        if (*p != T_NEWLINE) {
            while(*p) p++;
            p++;
        }

        if (*p == 0) break;                         // The end of the program.
        if (*p == T_NEWLINE) CurrentLinePtr = p++;  // Record newline and step over token.
        if (*p == T_LINENBR) p += 3;                // Step over token and 2-byte line number

        skipspace(p);

        // Have we found a label ?
        if (*p == T_LABEL) {
            p += 2; // Step over the token and the length byte.
            MmResult result = AddFunction(&p, kLabel, CurrentLinePtr);
            if (FAILED(result) && abort_on_error) {
                error_throw_ex(result, FormatAddFunctionError(result, kLabel));
            }
            skipspace(p);
        }

        // Have we found a function or subroutine ?
        const CommandToken cmd = commandtbl_decode(p);
        if (cmd == cmdSUB || cmd == cmdCSUB || cmd == cmdFUN || cmd == cmdCFUN) {
            int type = (cmd == cmdSUB || cmd == cmdCSUB) ? kSub : kFunction;
            const char *addr = p;
            p += sizeof(CommandToken); // Step over the token.
            MmResult result = AddFunction(&p, type, addr);
            if (FAILED(result) && abort_on_error) {
                error_throw_ex(result, FormatAddFunctionError(result, type));
            }
        }
    }
}

// Scan through the program loaded in flash and build a table pointing to the definition of all user defined subroutines and functions.
// This pre processing speeds up the program when using defined subroutines and functions
// this routine also looks for embedded fonts and adds them to the font table
void PrepareProgram(int ErrAbort) {
    // Clear the font table.
    for (int i = FONT_BUILTIN_NBR; i < FONT_TABLE_SIZE; i++) {
        FontTable[i] = NULL;
    }

#if !defined(__mmb4l__)
    CFunctionFlash = CFunctionLibrary = NULL;
#endif

    PrepareFunctionTable(ErrAbort);

#if defined(MICROMITE)
    while(*p == 0) p++;                                             // the end of the program can have multiple zeros
    p++;                                                            // step over the terminating 0xff
    *CFunPtr = (unsigned char *)(((unsigned int)p + 0b11) & ~0b11); // CFunction flash (if it exists) starts on the next word address after the program in flash
    if(i < MAXSUBFUN) subfun[i] = NULL;
    CurrentLinePtr = NULL;

    // now, step through the CFunction area looking for fonts to add to the font table
    unsigned int cfp = *(unsigned int **)CFunPtr;
    while(*cfp != 0xffffffff) {
        if(*cfp < FONT_TABLE_SIZE)
            FontTable[*cfp] = (unsigned char *)(cfp + 2);
        cfp++;
        cfp += (*cfp + 4) / sizeof(unsigned int);
    }
#endif
}

/**
 * @brief  Finds a FUNCTION/SUBroutine by name in the function table.
 *
 * @param[in]  p          pointer to start of function name.
 * @param[in]  type_mask  kSub             - to find a subroutine.
 *                        kFunction        - to find a function.
 *                        kSub | kFunction - to find a subroutine or function.
 * @return                the index of the function in funtbl[],
 *                        or -1 if the function could not be found.
 */
int FindSubFun(const char *p, uint8_t type_mask) {
    assert(type_mask >= 1 || type_mask <= 3);

    char name[MAXVARLEN + 1];
    MmResult result = parse_name(&p, name);

    int fun_idx;
    if (SUCCEEDED(result)) result = funtbl_find(name, type_mask, &fun_idx);

    const char *msg = NULL;
    switch (result) {
        case kOk:
            CASE_FALLTHROUGH;
        case kFunctionNotFound:
            return fun_idx;
        case kNameTooLong:
            switch (type_mask) {
                case kFunction:
                    msg = "Function name too long";
                    break;
                case kSub:
                    msg = "Subroutine name too long";
                    break;
                case kFunction | kSub:
                    msg = "Function/subroutine name too long";
                    break;
                default:
                    break;
            }
            break;
        case kFunctionTypeMismatch:
            if (funtbl[fun_idx].type == kLabel) return -1;
            switch (type_mask) {
                case kSub:
                    msg = "Not a subroutine";
                    break;
                case kFunction | kSub:
                    // I don't think this is currently possible.
                    msg = "Not a function/subroutine";
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    error_throw_ex(result, msg ? msg : mmresult_to_string(result));
    return -1;
}



// This function is responsible for executing a defined subroutine or function.
// As these two are similar they are processed in the one lump of code.
//
// The arguments when called are:
//   isfun    = true if we are executing a function
//   cmd      = pointer to the command name used by the caller (in program memory)
//   index    = index into funtbl[i] which points to the definition of the sub or funct
//   fa, i64a, sa and typ are pointers to where the return value is to be stored (used by functions only)
void DefinedSubFun(int isfun, const char *cmd, int index, MMFLOAT *fa, MMINTEGER *i64a, char **sa, int *typ) {
    const char *p;
    const char *ttp;
    char *s;
    const char *CallersLinePtr, *SubLinePtr = NULL;
    char *argbuf1; char **argv1; int argc1;
    char *argbuf2; char **argv2; int argc2;
    char fun_name[MAXVARLEN + 1];
    int i;
    int ArgType, FunType;
    int *argtype;
    union u_argval {
        MMFLOAT f;                                                  // the value if it is a float
        MMINTEGER i;                                                // the value if it is an integer
        MMFLOAT *fa;                                                // pointer to the allocated memory if it is an array of floats
        MMINTEGER *ia;                                              // pointer to the allocated memory if it is an array of integers
        char *s;                                                    // pointer to the allocated memory if it is a string
    } *argval;
    int *argVarIndex;

    CallersLinePtr = CurrentLinePtr;
    SubLinePtr = funtbl[index].addr;                                // used for error reporting
    p =  SubLinePtr + sizeof(CommandToken);                         // point to the sub or function definition
    skipspace(p);
    ttp = p;

    // copy the sub/fun name from the definition into temp storage and terminate
    // p is left pointing to the end of the name (ie, start of the argument list in the definition)
    CurrentLinePtr = SubLinePtr;                                    // report errors at the definition
    {
        char *tp = fun_name;
        *tp++ = *p++; while(isnamechar(*p)) *tp++ = *p++;
        if(*p == '$' || *p == '%' || *p == '!') {
            if(!isfun) {
                error("Type specification is invalid: @", (int)(*p));
            }
            *tp++ = *p++;
        }
        *tp = 0;
    }

    if(isfun && *p != '(' && (*SubLinePtr != cmdCFUN)) error("Function definition");

    // find the end of the caller's identifier, tp is left pointing to the start of the caller's argument list
    CurrentLinePtr = CallersLinePtr;                                // report errors at the caller
    const char *tp = cmd + 1;
    while(isnamechar(*tp)) tp++;
    if(*tp == '$' || *tp == '%' || *tp == '!') {
        if(!isfun) error("Type specification");
        tp++;
    }
    if(toupper(*(p-1)) != toupper(*(tp-1))) error("Inconsistent type suffix");

    // if this is a function we check to find if the function's type has been specified with AS <type> and save it
    CurrentLinePtr = SubLinePtr;                                    // report errors at the definition
    FunType = T_NOTYPE;
    if(isfun) {
        ttp = skipvar(ttp, false);                                  // point to after the function name and bracketed arguments
        skipspace(ttp);
        if(*ttp == tokenAS) {                                       // are we using Microsoft syntax (eg, AS INTEGER)?
            ttp++;                                                  // step over the AS token
            ttp = CheckIfTypeSpecified(ttp, &FunType, true);        // get the type
            if(!(FunType & T_IMPLIED)) error("Variable type");
        }
        FunType |= (V_FIND | V_DIM_VAR | V_LOCAL | V_EMPTY_OK);
    }


    // from now on
    // tp  = the caller's argument list
    // p   = the argument list for the definition
    skipspace(tp); skipspace(p);

#if defined(MICROMITE)
    const CommandToken cmd = commandtbl_decode(SubLinePtr);

    // if this is a CFUNCTION we can skip all the rest and just execute the CFUNCTION and return its value
    if (cmd == cmdCFUN) {
        skipspace(p);
        if(*p != '(')
            *typ = T_INT;
        else {                                                      // find the type
            char *pp = p;
            while(*pp != ')' && *pp != 0) pp++;
            if(*pp == 0) error("Syntax");
            pp++; skipspace(pp);
            CheckIfTypeSpecified(pp, typ, false);
            *typ &= ~T_IMPLIED;
        }
        switch(*typ) {                                              // return the correct type of value
            union {
                float ftmp;
                int itmp;
            } u;
            case T_INT:  *i64a = CallCFunction(SubLinePtr, tp, p, CallersLinePtr); break;
            case T_NBR:  u.itmp = (int)CallCFunction(SubLinePtr, tp, p, CallersLinePtr);
                         *fa = u.ftmp;
                         #if !defined(MX170)
                           RoundDoubleFloat(fa);
                         #endif
                         break;
            case T_STR:  *sa = (char *)((unsigned int)CallCFunction(SubLinePtr, tp, p, CallersLinePtr)); break;
        }
        TempMemoryIsChanged = true;                                 // signal that temporary memory should be checked
        return;
    }

    // similar if this is a CSUB
    if (cmd == cmdCSUB) {
        CallCFunction(SubLinePtr, tp, p, CallersLinePtr);           // run the CSUB
        TempMemoryIsChanged = true;                                 // signal that temporary memory should be checked
        return;
    }
#endif

    // from now on we have a user defined sub or function (not a C routine)

    if(gosubindex >= MAXGOSUB) error("Too many nested SUB/FUN");
    errorstack[gosubindex] = CallersLinePtr;
    gosubstack[gosubindex++] = isfun ? NULL : nextstmt;             // NULL signifies that this is returned to by ending ExecuteProgram()

    // allocate memory for processing the arguments
    argval = GetTempMemory(MAX_ARG_COUNT * sizeof(union u_argval));
    argtype = GetTempMemory(MAX_ARG_COUNT * sizeof(int));
    argVarIndex = GetTempMemory(MAX_ARG_COUNT * sizeof(int));
    argbuf1 = GetTempMemory(STRINGSIZE); argv1 = GetTempMemory(MAX_ARG_COUNT * sizeof(char *));  // these are for the caller
    argbuf2 = GetTempMemory(STRINGSIZE); argv2 = GetTempMemory(MAX_ARG_COUNT * sizeof(char *));  // and these for the definition of the sub or function

    // now split up the arguments in the caller
    CurrentLinePtr = CallersLinePtr;                                // report errors at the caller
    argc1 = 0;
    if(*tp) makeargs(&tp, MAX_ARG_COUNT, argbuf1, argv1, &argc1, (*tp == '(') ? "(," : ",");

    // split up the arguments in the definition
    CurrentLinePtr = SubLinePtr;                                    // any errors must be at the definition
    argc2 = 0;
    if(*p) makeargs(&p, MAX_ARG_COUNT, argbuf2, argv2, &argc2, (*p == '(') ? "(," : ",");

    // error checking
    if(argc2 && (argc2 & 1) == 0) error("Argument list");
    CurrentLinePtr = CallersLinePtr;                                // report errors at the caller
    if(argc1 > argc2 || (argc1 && (argc1 & 1) == 0)) error("Argument list");

    // step through the arguments supplied by the caller and get the value supplied
    // these can be:
    //    - missing (ie, caller did not supply that parameter)
    //    - a variable, in which case we need to get a pointer to that variable's data and save its index so later we can get its type
    //    - an expression, in which case we evaluate the expression and get its value and type
    for(i = 0; i < argc2; i += 2) {                                 // count through the arguments in the definition of the sub/fun
        if(i < argc1 && *argv1[i]) {
            // check if the argument is a valid variable
            if(i < argc1 && isnamestart(*argv1[i]) && *skipvar(argv1[i], false) == 0) {
                // yes, it is a variable (or perhaps a user defined function which looks the same)?
                if(!(FindSubFun(argv1[i], kFunction) >= 0 && strchr(argv1[i], '(') != NULL)) {
                    // yes, this is a valid variable.  set argvalue to point to the variable's data and argtype to its type
                    argval[i].s = findvar(argv1[i], V_FIND | V_EMPTY_OK);        // get a pointer to the variable's data
                    argtype[i] = vartbl[VarIndex].type;                          // and the variable's type
                    argVarIndex[i] = VarIndex;
                    if(argtype[i] & T_CONST) {
                        argtype[i] = 0;                                          // we don't want to point to a constant
                    } else {
                        argtype[i] |= T_PTR;                                     // flag this as a pointer
                    }
                }
            }

            // if argument is present and is not a pointer to a variable then evaluate it as an expression
            if(argtype[i] == 0) {
                MMINTEGER ia;
                evaluate(argv1[i], &argval[i].f, &ia, &s, &argtype[i], false);   // get the value and type of the argument
                if(argtype[i] & T_INT)
                    argval[i].i = ia;
                else if(argtype[i] & T_STR) {
                    argval[i].s = GetTempStrMemory();
                    Mstrcpy(argval[i].s, s);
                }
            }
        }
    }

    // now we step through the parameters in the definition of the sub/fun
    // for each one we create the local variable and compare its type to that supplied in the callers list
    CurrentLinePtr = SubLinePtr;                                    // any errors must be at the definition
    LocalIndex++;
    char *tp2;                                                      // temporary non-const char *
                                                                    // it will be pointing into the items of argv2[] which we know
                                                                    // are not constants so we can cast away const-ness as necessary
                                                                    // to remove warnings.
    for(i = 0; i < argc2; i += 2) {                                 // count through the arguments in the definition of the sub/fun
        ArgType = T_NOTYPE;
        tp2 = (char *) skipvar(argv2[i], false);                    // point to after the variable
        skipspace(tp2);
        if (*tp2 == tokenAS) {                                      // are we using Microsoft syntax (eg, AS INTEGER)?
            *tp2++ = '\0';                                          // terminate the string and step over the AS token
            tp2 = (char *) CheckIfTypeSpecified(tp2, &ArgType, true);  // and get the type
            if(!(ArgType & T_IMPLIED)) error("Variable type");
        }
        ArgType |= (V_FIND | V_DIM_VAR | V_LOCAL | V_EMPTY_OK);
        (void) findvar(argv2[i], ArgType);                          // declare the local variable
        if(vartbl[VarIndex].dims[0] > 0) error("Argument list");    // if it is an array it must be an empty array

        CurrentLinePtr = CallersLinePtr;                            // report errors at the caller

        // if the definition called for an array, special processing and checking will be required
        if(vartbl[VarIndex].dims[0] == -1) {
            int j;
            if(vartbl[argVarIndex[i]].dims[0] == 0) error("Expected an array");
            if(TypeMask(vartbl[VarIndex].type) != TypeMask(argtype[i])) error("Incompatible type: $", argv1[i]);
            vartbl[VarIndex].val.s = NULL;
            for(j = 0; j < MAXDIM; j++)                             // copy the dimensions of the supplied variable into our local variable
                vartbl[VarIndex].dims[j] = vartbl[argVarIndex[i]].dims[j];
        }

        // if this is a pointer check and the type is NOT the same as that requested in the sub/fun definition
        if((argtype[i] & T_PTR) && TypeMask(vartbl[VarIndex].type) != TypeMask(argtype[i])) {
            if((TypeMask(vartbl[VarIndex].type) & T_STR) || (TypeMask(argtype[i]) & T_STR))
                error("Incompatible type: $", argv1[i]);
            // make this into an ordinary argument
            if(vartbl[argVarIndex[i]].type & T_PTR) {
                argval[i].i = *vartbl[argVarIndex[i]].val.ia;       // get the value if the supplied argument is a pointer
            } else {
                argval[i].i = *(MMINTEGER *)argval[i].s;            // get the value if the supplied argument is an ordinary variable
            }
            argtype[i] &= ~T_PTR;                                   // and remove the pointer flag
        }

        // if this is a pointer (note: at this point the caller type and the required type must be the same)
        if(argtype[i] & T_PTR) {
            // the argument supplied was a variable so we must setup the local variable as a pointer
            if((vartbl[VarIndex].type & T_STR) && vartbl[VarIndex].val.s != NULL) {
                FreeMemory(vartbl[VarIndex].val.s);                            // free up the local variable's memory if it is a pointer to a string
            }
            vartbl[VarIndex].val.s = argval[i].s;                              // point to the data of the variable supplied as an argument
            vartbl[VarIndex].type |= T_PTR;                                    // set the type to a pointer
            vartbl[VarIndex].size = vartbl[argVarIndex[i]].size;               // just in case it is a string copy the size
        // this is not a pointer
        } else if(argtype[i] != 0) {                                           // in getting the memory argtype[] is initialised to zero
            // the parameter was an expression or a just straight variables with different types (therefore not a pointer))
            if((vartbl[VarIndex].type & T_STR) && (argtype[i] & T_STR)) {      // both are a string
                Mstrcpy(vartbl[VarIndex].val.s, argval[i].s);
                ClearSpecificTempMemory(argval[i].s);
            } else if((vartbl[VarIndex].type & T_NBR) && (argtype[i] & T_NBR)) // both are a float
                vartbl[VarIndex].val.f = argval[i].f;
            else if((vartbl[VarIndex].type & T_NBR) && (argtype[i] & T_INT))   // need a float but supplied an integer
                vartbl[VarIndex].val.f = argval[i].i;
            else if((vartbl[VarIndex].type & T_INT) && (argtype[i] & T_INT))   // both are integers
                vartbl[VarIndex].val.i = argval[i].i;
            else if((vartbl[VarIndex].type & T_INT) && (argtype[i] & T_NBR))   // need an integer but was supplied with a MMFLOAT
                vartbl[VarIndex].val.i = FloatToInt64(argval[i].f);
            else
                error("Incompatible type: $", argv1[i]);
        }
    }

    // temp memory used in setting up the arguments can be deleted now
    ClearSpecificTempMemory(argval);
    ClearSpecificTempMemory(argtype);
    ClearSpecificTempMemory(argVarIndex);
    ClearSpecificTempMemory(argbuf1);
    ClearSpecificTempMemory(argv1);
    ClearSpecificTempMemory(argbuf2);
    ClearSpecificTempMemory(argv2);

    // set the CurrentSubFunName which is used to create static variables
    strcpy(CurrentSubFunName, fun_name);

    // if it is a defined command we simply point to the first statement in our command and allow ExecuteProgram() to carry on as before
    // exit from the sub is via cmd_return which will decrement LocalIndex
    if(!isfun) {
        skipelement(p);
        nextstmt = p;                                               // point to the body of the subroutine
        return;
    }

    // if it is a defined function we have a lot more work to do.  We must:
    //   - Create a local variable for the function's name
    //   - Save the globals being used by the current command that caused the function to be called
    //   - Invoke another instance of ExecuteProgram() to execute the body of the function
    //   - When that returns we need to restore the global variables
    //   - Get the variable's value and save that in the return value globals (fret or sret)
    //   - Return to the expression parser
    char *pvar = findvar(fun_name, FunType | V_FUNCT);              // declare the local variable
    FunType = vartbl[VarIndex].type;
    if(FunType & T_STR) {
        FreeMemory(vartbl[VarIndex].val.s);                         // free the memory if it is a string
        vartbl[VarIndex].type |= T_PTR;
        LocalIndex--;                                               // allocate the memory at the previous level
        vartbl[VarIndex].val.s = GetTempMemory(STRINGSIZE);         // and use our own memory
        pvar = vartbl[VarIndex].val.s;
        LocalIndex++;
    }
    skipelement(p);                                                 // point to the body of the function

    const char *cached_nextstmt = nextstmt;                         // save the globals used by commands
    CommandToken cached_cmdtoken = cmdtoken;
    const char *cached_cmdline = cmdline;

    ExecuteProgram(p);                                              // execute the function's code
    CurrentLinePtr = CallersLinePtr;                                // report errors at the caller

    cmdline = cached_cmdline;                                       // restore the globals
    cmdtoken = cached_cmdtoken;
    nextstmt = cached_nextstmt;

    // return the value of the function's variable to the caller
    if(FunType & T_NBR)
        *fa = *(MMFLOAT *) pvar;
    else if(FunType & T_INT)
        *i64a = *(MMINTEGER *) pvar;
    else
        *sa = pvar;                                                 // for a string we just need to return the local memory
    *typ = FunType;                                                 // save the function type for the caller
    ClearVars(LocalIndex--);                                        // delete any local variables
    TempMemoryIsChanged = true;                                     // signal that temporary memory should be checked
    gosubindex--;
}



/********************************************************************************************************************************************
 take an input line and turn it into a line with tokens suitable for saving into memory
********************************************************************************************************************************************/

//take an input string in inpbuf[] and copy it to tknbuf[] and:
// - convert the line number to a binary number
// - convert a label to the token format
// - convert keywords to tokens
// - convert the colon to a zero char
// the result in tknbuf[] is terminated with MMFLOAT zero chars
// if the arg console is true then do not add a line number

void tokenise(int console) {
    char *p, *op;
    int i;
    int firstnonwhite;
    int labelvalid;

    // first, make sure that only printable characters are in the line
    p = inpbuf;
    while(*p) {
        *p = *p & 0x7f;
        if(*p < ' ' || *p == 0x7f)  *p = ' ';
        p++;
    }

    // setup the input and output buffers
    p = inpbuf;
    op = tknbuf;
    if(!console) *op++ = T_NEWLINE;

    // get the line number if it exists
    {
        char *tp = p;
        skipspace(tp);
        for(i = 0; i < 8; i++) if(!isxdigit(tp[i])) break;  // test if this is eight hex digits
        if(isdigit(*tp) && i < 8) {                         // if it a digit and not an 8 digit hex number (ie, it is CFUNCTION data) then try for a line number
            char *endptr;
            i = strtol(tp, &endptr, 10);
            tp = endptr;
            if(!console && i > 0 && i <= MAXLINENBR) {
                *op++ = T_LINENBR;
                *op++ = (i>>8);
                *op++ = (i & 0xff);
            }
            p = tp;
        }
    }

    // process the rest of the line
    firstnonwhite = true;
    labelvalid = true;
    while(*p) {

        // just copy a space
        if(*p == ' ') {
            *op++ = *p++;
            continue;
        }

        // first look for quoted text and copy it across
        // this will also accept a string without the closing quote and it will add the quote in
        if(*p == '"') {
            do {
                *op++ = *p++;
            } while(*p != '"' && *p);
            *op++ = '"';
            if(*p == '"') p++;
            continue;
        }

        // copy anything after a comment (')
        if(*p == '\'') {
            char t;
            do {
                t = *p++;
                *op++ = t;
            } while(*p);
            if (t == '\'') *op++ = 32;
            continue;
        }

        // check for multiline separator (colon) and replace with a zero char
        if(*p == ':') {
            *op++ = 0;
            p++;
            while(*p == ':') {                                      // insert a space between consecutive colons
                *op++ = ' ';
                *op++ = 0;
                p++;
            }
            firstnonwhite = true;
            continue;
        }

        // not whitespace or string or comment - try a number
        if(isdigit(*p) || *p == '.') {                              // valid chars at the start of a number
            while(isdigit(*p) || *p == '.' || *p == 'E' || *p == 'e')
                if (*p == 'E' || *p == 'e') {   // check for '+' or '-' as part of the exponent
                    *op++ = *p++;                                   // copy the number
                    if (*p == '+' || *p == '-') {                   // BUGFIX by Gerard Sexton
                        *op++ = *p++;                               // copy the '+' or '-'
                    }
                } else {
                    *op++ = *p++;                                   // copy the number
                }
            firstnonwhite = false;
            continue;
        }

        // not whitespace or string or comment or number - see if we can find a label or a token identifier
        if(firstnonwhite) {                                         // first entry on the line must be a command
            // these variables are only used in the search for a command code
            char *tp2, *match_p = NULL;
            ssize_t match_i = -1, match_l = 0;
            // first test if it is a print shortcut char (?) - this needs special treatment
            if(*p == '?') {
                match_i = cmdPRINT;
                if(*++p == ' ') p++;                                // eat a trailing space
                match_p = p;
            } else {
                // now try for a command in the command table
                // this works by scanning the entire table looking for the match with the longest command name
                // this is needed because we need to differentiate between END and END SUB for example.
                // without looking for the longest match we might think that we have a match when we found just END.
                const char *tp;
                for(i = 0 ; i < CommandTableSize - 1; i++) {
                    tp2 = p;
                    tp = commandtbl[i].name;
                    while(toupper(*tp2) == toupper(*tp) && *tp != 0) {
                        if(*tp == ' ')
                            skipspace(tp2);                         // eat up any extra spaces between keywords
                        else
                            tp2++;
                        tp++;
                        if(*tp == '(') skipspace(tp2);              // eat up space between a keyword and bracket
                    }
                    // we have a match
                    if(*tp == 0 && (!isnamechar(*tp2) || (commandtbl[i].type & T_FUN))) {
                        if(*(tp - 1) != '(' && isnamechar(*tp2)) continue;   // skip if not the function
                        // save the details if it is the longest command found so far
                        if((ssize_t) strlen(commandtbl[i].name) > match_l) {
                            match_p = tp2;
                            match_l = strlen(commandtbl[i].name);
                            match_i = i;
                        }
                    }
                }
            }

            if (match_i > -1) {
                // we have found a command
                commandtbl_encode(&op, match_i);
                p = match_p;                                        // step over the command in the source
                if (match_i == cmdREM)                              // check if it is a REM command
                    while(*p) *op++ = *p++;                         // and in that case just copy everything
                else {
                    if(isalpha(*(p-1)) && *p == ' ')                // if the command is followed by a space
                        p++;                                        // skip over it (llist will restore the space)
                }
                firstnonwhite = false;
                labelvalid = false;                                 // we do not want any labels after this
                continue;
            }

            // next test if it is a label
            if(labelvalid && isnamestart(*p)) {
                const char *tp;
                for(i = 0, tp = p + 1; i < MAXVARLEN - 1; i++, tp++)
                    if(!isnamechar(*tp)) break;                     // search for the first invalid char
                if(*tp == ':') {                                    // Yes !!  It is a label
                    labelvalid = false;                             // we do not want any more labels
                    *op++ = T_LABEL;                                // insert the token
                    *op++ = tp - p;                                 // insert the length of the label
                    for(i = tp - p; i > 0; i--) *op++ = *p++;       // copy the label
                    p++;                                            // step over the terminating colon
                    continue;
                }
            }
        } else {
            // check to see if it is a function or keyword
            const char *tp;
            char *tp2 = NULL;
            for(i = 0 ; i < TokenTableSize - 1; i++) {
                tp2 = p;
                tp = tokentbl[i].name;
                // check this entry
                while(toupper(*tp2) == toupper(*tp) && *tp != 0) {
                    tp++; tp2++;
                    if(*tp == '(') skipspace(tp2);
                }
                if(*tp == 0 && (!isnameend(*(tp - 1)) || !isnamechar(*tp2))) break;
            }
            if(i != TokenTableSize - 1) {
                // we have a  match
                i += C_BASETOKEN;
                *op++ = i;                                          // insert the token found
                p = tp2;                                            // and step over it in the source text
//                if(isalpha(*(p-1)) && *p == ' ') {                  // if the token is an alpha string followed by a space
//                    p++;                                            // skip over it (llist will restore the space)
//                }
                if(i == tokenTHEN || i == tokenELSE)
                    firstnonwhite = true;                           // a command is valid after a THEN or ELSE
                else
                    firstnonwhite = false;
                continue;
            }
        }

        // not whitespace or string or comment or token identifier or number
        // try for a variable name which could be a user defined subroutine or an implied let
        if(isnamestart(*p)) {                                       // valid chars at the start of a variable name
            if (firstnonwhite) {                                    // first entry on the line?
                const char *tp = skipvar(p, true);                  // find the char after the variable
                skipspace(tp);
                if (*tp == '=') {
                    commandtbl_encode(&op, cmdLET);
                }
            }
            while(isnamechar(*p)) *op++ = *p++;                     // copy the variable name
            firstnonwhite = false;
            labelvalid = false;                                     // we do not want any labels after this
            continue;
        }

        // special case where the character to copy is an opening parenthesis
        // we search back to see if the previous non space char was the end of an identifier and, if it is, we remove any spaces following the identifier
        // this enables the programmer to put spaces after a function name or array identifier without causing a confusing error
        if(*p == '(') {
            char *tp = op - 1;
            if(*tp == ' ') {
                while(*tp == ' ') tp--;
                if(isnameend(*tp)) op = tp + 1;
            }
        }

        // something else, so just copy the one character
        *op++ = *p++;
       labelvalid = false;                                          // we do not want any labels after this
       firstnonwhite = false;

    }
    // end of loop, trim any trailing blanks (but not part of a line number)
    while(*(op - 1) == ' ' && op > tknbuf + 3) *--op = 0;
    // make sure that it is terminated properly
    *op++ = 0;  *op++ = 0;  *op++ = 0;                              // terminate with  zero chars
}




/********************************************************************************************************************************************
 routines for evaluating expressions
 the main functions are getnumber(), getinteger() and getstring()
********************************************************************************************************************************************/



// A convenient way of evaluating an expression
// it takes two arguments:
//     p = pointer to the expression in memory (leading spaces will be skipped)
//     t = pointer to the type
//         if *t = T_STR or T_NBR or T_INT will throw an error if the result is not the correct type
//         if *t = T_NOTYPE it will not throw an error and will return the type found in *t
// it returns with a void pointer to a float, integer or string depending on the value returned in *t
// this will check that the expression is terminated correctly and throw an error if not
void *DoExpression(const char *p, int *t) {
    static MMFLOAT f;
    static MMINTEGER i64;
    static char *s;

    evaluate(p, &f, &i64, &s, t, false);
    if(*t & T_INT) return &i64;
    if(*t & T_NBR) return &f;
    if(*t & T_STR) return s;

    error("Internal fault (sorry)");
    return NULL;                                                    // to keep the compiler happy
}



// evaluate an expression.  p points to the start of the expression in memory
// returns either the float or string in the pointer arguments
// *t points to an integer which holds the type of variable we are looking for
//  if *t = T_STR or T_NBR or T_INT will throw an error if the result is not the correct type
//  if *t = T_NOTYPE it will not throw an error and will return the type found in *t
// this will check that the expression is terminated correctly and throw an error if not.  flags & E_NOERROR will suppress that check
const char *evaluate(const char *p, MMFLOAT *fa, MMINTEGER *ia, char **sa, int *ta, int flags) {
    int o;
    int t = *ta;
    char *s;

    p = getvalue(p, fa, ia, &s, &o, &t);                            // get the left hand side of the expression, the operator is returned in o
    while(o != E_END) p = doexpr(p, fa, ia, &s, &o, &t);            // get the right hand side of the expression and evaluate the operator in o

    // check that the types match and convert them if we can
    if((*ta & T_NBR || *ta & T_INT) && t & T_STR) error("Expected a number");
    if(*ta & T_STR && (t & T_NBR || t & T_INT)) {
        error("Expected a string");
    }
    if(o != E_END) error("Argument count");
    if((*ta & T_NBR) && (t & T_INT)) *fa = *ia;
    if((*ta & T_INT) && (t & T_NBR)) *ia = FloatToInt64(*fa);
    *ta = t;
    *sa = s;

    // check that the expression is terminated correctly
    if(!(flags & E_NOERROR)) {
        skipspace(p);
        if(!(*p == 0 || *p == ',' || *p == ')' || *p == '\''))  error("Expression syntax");
    }
    return p;
}


// evaluate an expression to get a number
MMFLOAT getnumber(const char *p) {
    int t = T_NBR;
    MMFLOAT f;
    MMINTEGER i64;
    char *s;

    evaluate(p, &f, &i64, &s, &t, false);
    if(t & T_INT) return (MMFLOAT)i64;
    return f;
}


// evaluate an expression and return a 64 bit integer
MMINTEGER getinteger(const char *p) {
    int t = T_INT;
    MMFLOAT f;
    MMINTEGER i64;
    char *s;

    evaluate(p, &f, &i64, &s, &t, false);
    if(t & T_NBR) return FloatToInt64(f);
    return i64;
}



// evaluate an expression and return an integer
// this will throw an error is the integer is outside a specified range
// this will correctly round the number if it is a fraction of an integer
int getint(const char *p, int min, int max) {
    MMINTEGER i;
    i = getinteger(p);
    if(i < min || i > max) error("% is invalid (valid is % to %)", (int)i, min, max);
    return i;
}



// evaluate an expression to get a string
char *getstring(const char *p) {
    int t = T_STR;
    MMFLOAT f;
    MMINTEGER i64;
    char *s;

    evaluate(p, &f, &i64, &s, &t, false);
    return s;
}



// evaluate an expression to get a string using the C style for a string
// as against the MMBasic style returned by getstring()
char *getCstring(const char *p) {
    char *tp;
    tp = GetTempStrMemory();                                        // this will last for the life of the command
    Mstrcpy(tp, getstring(p));                                      // get the string and save in a temp place
    MtoC(tp);                                                       // convert to a C style string
    return tp;
}



// recursively evaluate an expression observing the rules of operator precedence
const char *doexpr(const char *p, MMFLOAT *fa, MMINTEGER *ia, char **sa, int *oo, int *ta) {
    MMFLOAT fa1, fa2;
    MMINTEGER ia1, ia2;
    int o1, o2;
    int t1, t2;
    char *sa1, *sa2;

    TestStackOverflow();                                            // throw an error if we have overflowed the PIC32's stack

    fa1 = *fa;
    ia1 = *ia;
    sa1 = *sa;
    t1 = TypeMask(*ta);
    o1 = *oo;
    p = getvalue(p, &fa2, &ia2, &sa2, &o2, &t2);
    while(1) {
        if(o2 == E_END || tokentbl[o1].precedence <= tokentbl[o2].precedence) {
            if((t1 & T_STR) != (t2 & T_STR)) error("Incompatible types in expression");
            targ = tokentbl[o1].type & (T_NBR | T_INT);
            if(targ == T_NBR) {                                     // if the operator does not work with ints convert the args to floats
                if(t1 & T_INT) { fa1 = ia1; t1 = T_NBR; }           // at this time the only example of this is op_div (/)
                if(t2 & T_INT) { fa2 = ia2; t2 = T_NBR; }
            }
            if(targ == T_INT) {                                     // if the operator does not work with floats convert the args to ints
                if(t1 & T_NBR) { ia1 = FloatToInt64(fa1); t1 = T_INT; }
                if(t2 & T_NBR) { ia2 = FloatToInt64(fa2); t2 = T_INT; }
            }
            if(targ == (T_NBR | T_INT)) {                               // if the operator will work with both floats and ints
                if(t1 & T_NBR && t2 & T_INT) { fa2 = ia2; t2 = T_NBR; } // if one arg is float convert the other to a float
                if(t1 & T_INT && t2 & T_NBR) { fa1 = ia1; t1 = T_NBR; }
            }
            if(!(tokentbl[o1].type & T_OPER) || !(tokentbl[o1].type & t1)) {
                error("Invalid operator");
            }
            farg1 = fa1; farg2 = fa2;                               // setup the float args (incase it is a float)
            sarg1 = sa1; sarg2 = sa2;                               // ditto string args
            iarg1 = ia1; iarg2 = ia2;                               // ditto integer args
            targ = t1;                                              // this is what both args are
            tokentbl[o1].fptr();                                    // call the operator function
            *fa = fret;
            *ia = iret;
            *sa = sret;
            *oo = o2;
            *ta = targ;
            return p;
        }
        // the next operator has a higher precedence, recursive call to evaluate it
        else
            p = doexpr(p, &fa2, &ia2, &sa2, &o2, &t2);
    }
}



// get a value, either from a constant, function or variable
// also returns the next operator to the right of the value or E_END if no operator
const char *getvalue(const char* p, MMFLOAT* fa, MMINTEGER* ia, char** sa, int* oo, int* ta) {
    MMFLOAT f = 0;
    MMINTEGER i64 = 0;
    char *s = NULL;
    int t = T_NOTYPE;
    const char *tp;
    int i;

    skipspace(p);
    if (*p >= C_BASETOKEN) { //don't waste time if not a built-in function
        // special processing for the NOT operator
        // just get the next value and invert its logical value
        if (tokenfunction(*p) == op_not) {
            int ro;
            p++; t = T_NOTYPE;
            p = getvalue(p, &f, &i64, &s, &ro, &t);                     // get the next value
            if (t & T_NBR)
                f = (MMFLOAT)((f != 0) ? 0 : 1);                        // invert the value returned
            else if (t & T_INT)
                i64 = ((i64 != 0) ? 0 : 1);
            else
                error("Expected a number");
            skipspace(p);
            *fa = f;                                                    // save what we have
            *ia = i64;
            *sa = s;
            *ta = t;
            *oo = ro;
            return p;                                                   // return straight away as we already have the next operator
        }

        if (tokenfunction(*p) == op_inv) {
            int ro;
            uint64_t ut;
            p++; t = T_NOTYPE;
            p = getvalue(p, &f, &i64, &s, &ro, &t);                     // get the next value
            if (t & T_NBR)
                i64 = FloatToInt64(f);
            else if (!(t & T_INT))
                error("Expected a number");
            ut = ~(uint64_t)i64;
            i64 = (int64_t)ut;
            t = T_INT;
            skipspace(p);
            *fa = f;                                                    // save what we have
            *ia = i64;
            *sa = s;
            *ta = t;
            *oo = ro;
            return p;                                                   // return straight away as we already have the next operator
        }

        // special processing for the unary - operator
        // just get the next value and negate it
        if (tokenfunction(*p) == op_subtract) {
            int ro;
            p++; t = T_NOTYPE;
            p = getvalue(p, &f, &i64, &s, &ro, &t);                     // get the next value
            if (t & T_NBR)
                f = -f;                                                 // negate the MMFLOAT returned
            else if (t & T_INT)
                i64 = -i64;                                             // negate the integer returned
            else
                error("Expected a number");
            skipspace(p);
            *fa = f;                                                    // save what we have
            *ia = i64;
            *sa = s;
            *ta = t;
            *oo = ro;
            return p;                                                   // return straight away as we already have the next operator
        }

        if (tokenfunction(*p) == op_add) {
            int ro;
            p++; t = T_NOTYPE;
            p = getvalue(p, &f, &i64, &s, &ro, &t);                     // get the next value
            skipspace(p);
            *fa = f;                                                    // save what we have
            *ia = i64;
            *sa = s;
            *ta = t;
            *oo = ro;
            return p;                                                   // return straight away as we already have the next operator
        }

        // if a function execute it and save the result
        if (tokentype(*p) & (T_FUN | T_FNA)) {
            int tmp;
            tp = p;
            // if it is a function with arguments we need to locate the closing bracket and copy the argument to
            // a temporary variable so that functions like getarg() will work.
            if (tokentype(*p) & T_FUN) {
                const char *p1 = p + 1;
                p = getclosebracket(p);                                 // find the closing bracket
                char *p2 = (char *) GetTempMemory(STRINGSIZE);          // this will last for the life of the command
                ep = p2;
                while (p1 != p) *p2++ = *p1++;
            }
            p++;                                                        // point to after the function (without argument) or after the closing bracket
            targ = TypeMask(tokentype(*tp));                            // set the type of the function (which might need to know this)
            tmp = targ;
            tokenfunction(*tp)();                                       // execute the function
            if ((tmp & targ) == 0) error("Internal fault (sorry)");     // as a safety check the function must return a type the same as set in the header
            t = targ;                                                   // save the type of the function
            f = fret; i64 = iret; s = sret;                             // save the result
        }
    }
    else {
        // if it is a variable or a defined function, find it and get its value
        if (isnamestart(*p)) {
            // first check if it is terminated with a bracket
            tp = p + 1;
            while (isnamechar(*tp)) tp++;                               // search for the end of the identifier
            if (*tp == '$' || *tp == '%' || *tp == '!') tp++;
            i = -1;
            if (*tp == '(') i = FindSubFun(p, kFunction);               // if terminated with a bracket it could be a function
            if (i >= 0) {                                               // >= 0 means it is a user defined function
                const char *SaveCurrentLinePtr = CurrentLinePtr;        // in case the code in DefinedSubFun messes with this
                DefinedSubFun(true, p, i, &f, &i64, &s, &t);
                CurrentLinePtr = SaveCurrentLinePtr;
            }
            else {
                s = (char *) findvar(p, V_FIND);                        // if it is a string then the string pointer is automatically set
                t = TypeMask(vartbl[VarIndex].type);
                if (t & T_NBR) f = (*(MMFLOAT *)s);
                if (t & T_INT) i64 = (*(MMINTEGER *)s);
            }
            p = skipvar(p, false);
        }
        // is it an ordinary numeric constant?  get its value if yes
        // a leading + or - might have been converted to a token so we need to check for them also
        else if (isdigit(*p) || *p == '.') {
            char ts[31], *tsp;
            int isi64 = true;
            tsp = ts;
            int isf = true;
            MMINTEGER scale = 0;
            // copy the first digit of the string to a temporary place
            if (*p == '.') {
                isi64 = false;
                scale = 1;
            }
            else if (isdigit(*p)) {
                i64 = (*p - '0');
            }
            *tsp++ = *p++;

            // now concatenate the remaining digits
            while ((DIGIT_CHARS[(uint8_t)*p]) && (tsp - ts) < 30) {
                if (*p >= '0' && *p <= '9') {
                    i64 = i64 * 10 + (*p - '0');
                    if (scale)scale *= 10;
                }
                else {
                    if ((*p) == '.') {
                        isi64 = false;
                        scale = 1;
                    }
                    else {
                        if (toupper(*p) == 'E' || *p == '-' || *p == '+') {
                            isi64 = false;
                            isf = false;
                        }
                    }
                }
                *tsp++ = *p++;                                          // copy the string to a temporary place
            }
            *tsp = 0;                                                   // terminate it
            if (isi64) {
                t = T_INT;
            }
            else if (isf && (tsp - ts) < 18) {
                f = (MMFLOAT)i64 / (MMFLOAT)scale;
                t = T_NBR;
            }
            else {
                f = (MMFLOAT)strtod(ts, &tsp);                          // and convert to a MMFLOAT
                t = T_NBR;
            }
        }


        // if it is a numeric constant starting with the & character then get its base and convert to an integer
        else if (*p == '&') {
            p++; i64 = 0;
            switch (toupper(*p++)) {
                case 'H':
                    while (isxdigit(*p)) {
                        i64 = (i64 << 4) | ((toupper(*p) >= 'A') ? toupper(*p) - 'A' + 10 : *p - '0');
                        p++;
                    }
                    break;
                case 'O':
                    while (*p >= '0' && *p <= '7') {
                        i64 = (i64 << 3) | (*p++ - '0');
                    }
                    break;
                case 'B':
                    while (*p == '0' || *p == '1') {
                        i64 = (i64 << 1) | (*p++ - '0');
                    }
                    break;
                default:
                    error("Type prefix");
            }
            t = T_INT;
        }
        // if opening bracket then first evaluate the contents of the bracket
        else if (*p == '(') {
            p++;                                                        // step over the bracket
            p = evaluate(p, &f, &i64, &s, &t, true);                    // recursively get the contents
            if (*p != ')') error("No closing bracket");
            ++p;                                                        // step over the closing bracket
        }
        // if it is a string constant, return a pointer to that.  Note: tokenise() guarantees that strings end with a quote
        else if (*p == '"') {
            p++;                                                        // step over the quote
            char *p1 = s = (char *) GetTempMemory(STRINGSIZE);          // this will last for the life of the command
            tp = strchr(p, '"');
            while (p != tp) *p1++ = *p++;
            p++;
            CtoM(s);                                                    // convert to a MMBasic string
            t = T_STR;
        }
        else
            error("Syntax");
    }
    skipspace(p);
    *fa = f;                                                            // save what we have
    *ia = i64;
    *sa = s;
    *ta = t;

    // get the next operator, if there is not an operator set the operator to end of expression (E_END)
    if (tokentype(*p) & T_OPER)
        *oo = *p++ - C_BASETOKEN;
    else
        *oo = E_END;

    return p;
}



// search through program memory looking for a line number. Stops when it has a matching or larger number
// returns a pointer to the T_NEWLINE token or a pointer to the two zero characters representing the end of the program
char *findline(int nbr, int mustfind) {
    int i;
    char *p = ProgMemory;

    while(1) {
        if(p[0] == 0 && p[1] == 0) {
            i = MAXLINENBR;
            break;
        }

        if(p[0] == T_NEWLINE) {
            p++;
            continue;
        }

        if(p[0] == T_LINENBR) {
            i = (p[1] << 8) | p[2];
            if(mustfind) {
                if(i == nbr) break;
            } else {
                if(i >= nbr) break;
            }
            p += 3;
            continue;
        }

        if(p[0] == T_LABEL) {
            p += p[1] + 2;
            continue;
        }

        p++;
    }
    if(mustfind && i != nbr)
        error("Line number");
    return p;
}


// search through program memory looking for a label.
// returns a pointer to the T_NEWLINE token or throws an error if not found
// non cached version
const char *findlabel(const char *labelptr) {
    char name[MAXVARLEN + 1];
    MmResult result = parse_name(&labelptr, name);

    int fun_idx;
    if (SUCCEEDED(result)) result = funtbl_find(name, kLabel, &fun_idx);

    switch (result) {
        case kOk:
            return funtbl[fun_idx].addr;
        case kFunctionNotFound:
            error_throw_ex(result, "Label not found");
            return NULL;
        case kFunctionTypeMismatch:
            error_throw_ex(result, "Not a label");
            return NULL;
        case kNameTooLong:
            error_throw_ex(result, "Label too long");
            return NULL;
        default:
            error_throw(result);
            return NULL;
    }
}


// returns true if 'line' is a valid line in the program
int IsValidLine(int nbr) {
    char *p;
    p = findline(nbr, false);
    if(*p == T_NEWLINE) p++;
    if(*p == T_LINENBR) {
        if(((p[1] << 8) | p[2]) == nbr) return true;
    }
    return false;
}


// count the number of lines up to and including the line pointed to by the argument
// used for error reporting in programs that do not use line numbers
int CountLines(const char *target) {
    char *p;
    int cnt;

    p = ProgMemory;
    cnt = 0;

    while(1) {
        if(*p == 0xff || (p[0] == 0 && p[1] == 0))                  // end of the program
            return cnt;

        if(*p == T_NEWLINE) {
            p++;                                                    // and step over the line number
            cnt++;
            if(p >= target) return cnt;
            continue;
        }

        if(*p == T_LINENBR) {
            p += 3;                                                 // and step over the line number
            continue;
        }

        if(*p == T_LABEL) {
            p += p[0] + 2;                                          // still looking! skip over the label
            continue;
        }

        if(p++ > target) return cnt;

    }
}



/********************************************************************************************************************************************
routines for storing and manipulating variables
********************************************************************************************************************************************/


// find or create a variable
// the action parameter can be the following (these can be ORed together)
// - V_FIND    a straight forward find, if the variable is not found it is created and set to zero
// - V_NOFIND_ERR    throw an error if not found
// - V_NOFIND_NULL   return a null pointer if not found
// - V_DIM_VAR       dimension an array
// - V_EMPTY_OK      empty array dimension, e.g. foo() allowed
// - V_FUNCT         define the name of a function
// - V_LOCAL         create a local variable
//
// there are four types of variable:
//  - T_NOTYPE a free slot that was used but is now free for reuse
//  - T_STR string variable
//  - T_NBR holds a float
//  - T_INT integer variable
//
// A variable can have a number of characteristics
//  - T_PTR the variable points to another variable's data
//  - T_IMPLIED  the variables type does not have to be specified with a suffix
//  - T_CONST the contents of this variable cannot be changed
//
// storage of the variable's data:
//      if it is type T_NBR or T_INT the value is held in the variable slot
//      for T_STR a block of memory of MAXSTRLEN size (or size determined by the LENGTH keyword) will be malloc'ed and the pointer stored in the variable slot.
void *findvar(const char *p, int action) {

    TestStackOverflow();  // Test if we have overflowed the PIC32's stack.

    // Get the name.
    char name[MAXVARLEN + 1] = {0};
    MmResult result = parse_name(&p, name);
    switch (result) {
        case kOk:
            break;
        case kInvalidName:
            error_throw_ex(result, "Invalid variable name");
            return NULL;
        case kNameTooLong:
            error_throw_ex(result, "Variable name too long");
            return NULL;
        default:
            error_throw(result);
            return NULL;
    }

    // Check the terminating character to determine the variable type.
    int vtype = T_NOTYPE;
    if (*p == '$') {
        if ((action & T_IMPLIED) && !(action & T_STR)) {
            error("Conflicting variable type");
            return NULL;
        }
        vtype = T_STR;
        p++;
    } else if (*p == '%') {
        if ((action & T_IMPLIED) && !(action & T_INT)) {
            error("Conflicting variable type");
            return NULL;
        }
        vtype = T_INT;
        p++;
    } else if (*p == '!') {
        if ((action & T_IMPLIED) && !(action & T_NBR)) {
            error("Conflicting variable type");
            return NULL;
        }
        vtype = T_NBR;
        p++;
    } else if ((action & V_DIM_VAR) && DefaultType == T_NOTYPE && !(action & T_IMPLIED)) {
        error("Variable type not specified");
        return NULL;
    }

    // Check if this is an array.
    int dnbr = 0;  // Number of dimensions
                   // -1 : empty array used in fun/sub calls
                   //  0 : scalar.
    DIMTYPE dim[MAXDIM] = {0};
    if (*p == '(') {
        const char *pp = p + 1;
        skipspace(pp);
        if (action & V_EMPTY_OK && *pp == ')') {                    // if this is an empty array.  eg  ()
            dnbr = -1;                                              // flag this
        } else {                                                    // else, get the dimensions
            // start a new block - getargs macro must be the first executable stmt in a block
            // split the argument into individual elements
            // find the value of each dimension and store in dims[]
            // the bracket in "(," is a signal to getargs that the list is in brackets
            getargs(&p, MAXDIM * 2, "(,");
            if ((argc & 0x01) == 0) {
                error_throw(kInvalidArrayDimensions);
                return NULL;
            }
            dnbr = argc / 2 + 1;
            if (dnbr > MAXDIM) {
                error_throw(kInvalidArrayDimensions);
                return NULL;
            }
            for (int i = 0; i < argc; i += 2) {
                MMFLOAT f;
                MMINTEGER in;
                char *s;
                int targ = T_NOTYPE;
                evaluate(argv[i], &f, &in, &s, &targ, false);       // get the value and type of the argument
                if (targ == T_STR) dnbr = MAXDIM;                   // force an error to be thrown later (with the correct message)
                if (targ == T_NBR) in = FloatToInt32(f);
                if (in > DIMTYPE_MAX) {
                    error("Array bound exceeds maximum: %", DIMTYPE_MAX);
                    return NULL;
                }
                dim[i / 2] = in;
                if (dim[i / 2] < OptionBase) {
                    error_throw(kInvalidArrayDimensions);
                    return NULL;
                }
            }
        }
    }

    // Check for an existing variable.
    int var_idx = -1;
    {
        int global_idx = -1;
        result = vartbl_find(name, LocalIndex, &var_idx, &global_idx);
        switch (result) {
            case kOk:
                break;
            case kVariableNotFound:
                // If we are not explicitly declaring a LOCAL variable then use a
                // GLOBAL variable if we found one.
                if (!(action & V_LOCAL)) var_idx = global_idx;
                break;
            default:
                error_throw(result);
                return NULL;
        }
    }

    if (var_idx >= 0 && var_idx < varcnt) {  // We found an existing variable.

        // Are we trying to explicitly declare the same variable (DIM or LOCAL)
        // at the same scope ?
        if ((action & V_LOCAL) || (action & V_DIM_VAR)) {
            error("$ already declared", name);
            return NULL;
        }

        VarIndex = var_idx;  // Set the global VarIndex.

        // Check that the dimensions match.
        {
            int i;
            for (i = 0; i < MAXDIM && vartbl[var_idx].dims[i] != 0; i++) ;
            if (dnbr == -1) {
                if (i == 0) {
                    error("Array dimensions");
                    return NULL;
                }
            } else {
                if (i != dnbr) {
                    error("Array dimensions");
                    return NULL;
                }
            }
        }

        if (vtype == T_NOTYPE) {
            if (!(vartbl[var_idx].type & (DefaultType | T_IMPLIED))) {
                error("$ already declared", name);
                return NULL;
            }
        } else {
            if (!(vartbl[var_idx].type & vtype)) {
                error("$ already declared", name);
                return NULL;
            }
        }

        // if it is a non arrayed variable or an empty array it is easy, just calculate and return a pointer to the value
        if (dnbr == -1 || vartbl[var_idx].dims[0] == 0) {
            if (dnbr == -1 || vartbl[var_idx].type & (T_PTR | T_STR)) {
                return vartbl[var_idx].val.s;                        // if it is a string or pointer just return the pointer to the data
            } else if (vartbl[var_idx].type & (T_INT)) {
                return &(vartbl[var_idx].val.i);                     // must be an integer, point to its value
            } else {
                return &(vartbl[var_idx].val.f);                     // must be a straight number (float), point to its value
            }
        }

        // if we reached this point it must be a reference to an existing array
        // check that we are not using DIM and that all parameters are within the dimensions
        if (action & V_DIM_VAR) {
            // THW 15-Aug-2022: I do not think there is any code-path to reach
            // this that does not throw "$ already declared" first.
            error("Cannot re dimension array");
            return NULL;
        }
        for (int i = 0; i < dnbr; i++) {
            if (dim[i] > vartbl[var_idx].dims[i] || dim[i] < OptionBase) {
                error("Index out of bounds");
                return NULL;
            }
        }

        // then calculate the index into the array.  Bug fix by Gerard Sexton.
        int nbr = dim[0] - OptionBase;
        int j = 1;
        for (int i = 1; i < dnbr; i++) {
            j *= (vartbl[var_idx].dims[i - 1] + 1 - OptionBase);
            nbr += (dim[i] - OptionBase) * j;
        }
        // finally return a pointer to the value
        if (vartbl[var_idx].type & T_NBR) {
            return vartbl[var_idx].val.s + (nbr * sizeof(MMFLOAT));
        } else if (vartbl[var_idx].type & T_INT) {
            return vartbl[var_idx].val.s + (nbr * sizeof(MMINTEGER));
        } else {
            return vartbl[var_idx].val.s + (nbr * (vartbl[var_idx].size + 1));
        }
    }

    // We reached this point if no existing variable has been found.
    if (action & V_NOFIND_ERR) {
        error("Cannot find $", name);
        return NULL;
    }
    if (action & V_NOFIND_NULL) return NULL;
    if ((OptionExplicit || dnbr != 0) && !(action & V_DIM_VAR)) {
        error("$ is not declared", name);
        return NULL;
    }
    if (vtype == T_NOTYPE) {
        if (action & T_IMPLIED)
            vtype = (action & (T_NBR | T_INT | T_STR));
        else
            vtype = DefaultType;
    }

    // Check the sub/fun table to make sure that there is not a sub/fun with the same name.
    // Don't do this if we are defining the local variable for a function name.
    if (!(action & V_FUNCT)) {
        int fun_idx = -1;
        MmResult result = funtbl_find(name, kFunction | kSub, &fun_idx);
        switch (result) {
            case kOk:
                error_throw_legacy("A function/subroutine has the same name: $", name);
                return NULL;
            case kFunctionTypeMismatch:
                // Found a label.
                CASE_FALLTHROUGH;
            case kFunctionNotFound:
                break;
            default:
                error_throw(result);
                return NULL;
        }
    }

    // If we are declaring a new array then the bound of the first dimension must
    // be greater than OPTION BASE. This isn't caught inside vartbl_add()
    // because it uses a value of dim[0] == 0 to mean a scalar variable.
    for (int i = 0; i < dnbr; ++i) {
        if (dim[i] <= mmb_options.base) {
            error_throw(kInvalidArrayDimensions);
            return NULL;
        }
    }

    // If it is an array we must be dimensioning it.
    // If it is a string array we skip over the dimension values and look for
    // the LENGTH keyword and if found set 'slen'.
    uint8_t slen = MAXSTRLEN;
    if (action & V_DIM_VAR) {
        if (vtype & T_STR) {
            int i = 0;
            if (*p == '(') {
                do {
                    if (*p == '(') i++;
                    if (tokentype(*p) & T_FUN) i++;
                    if (*p == ')') i--;
                    p++;
                } while (i);
            }
            skipspace(p);
            const char *p2;
            if ((p2 = checkstring(p, "LENGTH")) != NULL) {
                slen = getint(p2, 1, MAXSTRLEN);
                if (slen == 0) return NULL;
            } else {
                if (!(*p == ',' || *p == 0 || tokenfunction(*p) == op_equal || tokenfunction(*p) == op_invalid)) {
                    error("Unexpected text: $", p);
                    return NULL;
                }
            }
        }
    }

    if (dnbr == -1) dim[0] = -1;  // "empty" array for fun/sub parameters.
    result = vartbl_add(
            name,
            vtype | (action & (T_IMPLIED | T_CONST)),
            (action & V_LOCAL) ? LocalIndex : 0,
            dnbr == 0 ? NULL : dim,
            (vtype & T_STR) ? slen : 0,
            &var_idx);
    VarIndex = var_idx;
    switch (result) {
        case kOk:
            if (vartbl[var_idx].dims[0] != 0) {
                return vartbl[var_idx].val.s;
            } else if (vartbl[var_idx].type & T_INT) {
                return &(vartbl[var_idx].val.i);
            } else if (vartbl[var_idx].type & T_NBR) {
                return &(vartbl[var_idx].val.f);
            } else {
                return vartbl[var_idx].val.s;
            }
        default:
            error_throw(result);
            return NULL;
    }
}



/********************************************************************************************************************************************
 utility routines
 these routines form a library of functions that any command or function can use when dealing with its arguments
 by centralising these routines it is hoped that bugs can be more easily found and corrected (unlike bwBasic !)
*********************************************************************************************************************************************/

// take a line of basic code and split it into arguments
// this function should always be called via the macro getargs
//
// a new argument is created by any of the chars in the string delim (not in brackets or quotes)
// with this function commands have much less work to do to evaluate the arguments
//
// The arguments are:
//   pointer to a pointer which points to the string to be broken into arguments.
//   the maximum number of arguments that are expected.  an error will be thrown if more than this are found.
//   buffer where the returned strings are to be stored
//   pointer to an array of strings that will contain (after the function has returned) the values of each argument
//   pointer to an integer that will contain (after the function has returned) the number of arguments found
//   pointer to a string that contains the characters to be used in spliting up the line.  If the first char of that
//   string is an opening bracket '(' this function will expect the arg list to be enclosed in brackets.
void makeargs(const char **p, int maxargs, char *argbuf, char *argv[], int *argc, const char *delim) {
    TestStackOverflow();                                            // throw an error if we have overflowed the PIC32's stack

    const char *tp = *p;
    char *op = argbuf;
    *argc = 0;
    bool inarg = false;
    bool expect_cmd = false;
    bool expect_bracket = false;

    // skip leading spaces
    while(*tp == ' ') tp++;

    // check if we are processing a list enclosed in brackets and if so
    //  - skip the opening bracket
    //  - flag that a closing bracket should be found
    if (*delim == '(') {
        if (*tp != '(') ERROR_SYNTAX;
        expect_bracket = true;
        delim++;
        tp++;
    }

    // the main processing loop
    while(*tp) {

        if(expect_bracket == true && *tp == ')') break;

        // comment char causes the rest of the line to be skipped
        if(*tp == '\'') {
            break;
        }

        // the special characters that cause the line to be split up are in the string delim
        // any other chars form part of the one argument
        if(strchr(delim, *tp) != NULL && !expect_cmd) {
            if(*tp == tokenTHEN || *tp == tokenELSE) expect_cmd = true;
            if(inarg) {                                             // if we have been processing an argument
                while(op > argbuf && *(op - 1) == ' ') op--;        // trim trailing spaces
                *op++ = 0;                                          // terminate it
            } else if(*argc) {                                      // otherwise we have two delimiters in a row (except for the first argument)
                argv[(*argc)++] = op;                               // create a null argument to go between the two delimiters
                *op++ = 0;                                          // and terminate it
            }

            inarg = false;
            if (*argc >= maxargs) ERROR_SYNTAX;
            argv[(*argc)++] = op;                                   // save the pointer for this delimiter
            *op++ = *tp++;                                          // copy the token or char (always one)
            *op++ = 0;                                              // terminate it
            continue;
        }

        // check if we have a THEN or ELSE token and if so flag that a command should be next
        if(*tp == tokenTHEN || *tp == tokenELSE) expect_cmd = true;


        // remove all spaces (outside of quoted text and bracketed text)
        if(!inarg && *tp == ' ') {
            tp++;
            continue;
        }

        // not a special char so we must start a new argument
        if(!inarg) {
            if (*argc >= maxargs) ERROR_SYNTAX;
            argv[(*argc)++] = op;                                   // save the pointer for this arg
            inarg = true;
        }

        // if an opening bracket '(' copy everything until we hit the matching closing bracket
        // this includes special characters such as , and ; and keeps track of any nested brackets
        if(*tp == '(' || ((tokentype(*tp) & T_FUN) && !expect_cmd)) {
            int x;
            x = (getclosebracket(tp) - tp) + 1;
            memcpy(op, tp, x);
            op += x; tp += x;
            continue;
        }

        // if quote mark (") copy everything until the closing quote
        // this includes special characters such as , and ;
        // the tokenise() function will have ensured that the closing quote is always there
        if(*tp == '"') {
            do {
                *op++ = *tp++;
                if(*tp == 0) ERROR_SYNTAX;
            } while(*tp != '"');
            *op++ = *tp++;
            continue;
        }

        // anything else is just copied into the argument
        *op++ = *tp++;
        if (expect_cmd) {
            *op++ = *tp++;
            expect_cmd = false;
        }
    }
    if (expect_bracket && *tp != ')') ERROR_SYNTAX;
    while(op - 1 > argbuf && *(op-1) == ' ') --op;                  // trim any trailing spaces on the last argument
    *op = 0;                                                        // terminate the last argument
}



/**********************************************************************************************
 Routines to convert floats and integers to formatted strings
 These replace the sprintf() libraries with much less flash usage
**********************************************************************************************/

#define IntToStrBufSize 65

// convert a integer to a string.
// sstr is a buffer where the chars are to be written to
// sum is the number to be converted
// base is the numbers base radix (10 = decimal, 16 = hex, etc)
// if base 10 the number will be signed otherwise it will be unsigned
void IntToStr(char *strr, MMINTEGER nbr, unsigned int base) {
    int i, negative;
    unsigned char digit;
    UNSIGNED_MMINTEGER sum;

    unsigned char str[IntToStrBufSize];

    if(nbr < 0 && base == 10) {                                     // we can have negative numbers in base 10 only
        nbr = llabs(nbr);
        negative = true;
    } else
        negative = false;

    // this generates the digits in reverse order
    sum = (UNSIGNED_MMINTEGER) nbr;
    i = 0;
    do {
        digit = sum % base;
        if (digit < 0xA)
            str[i++] = '0' + digit;
        else
            str[i++] = 'A' + digit - 0xA;
        sum /= base;
    } while (sum && i < IntToStrBufSize);

    if(negative) *strr++ = '-';

    // we now need to reverse the digits into their correct order
    for(i--; i >= 0; i--) *strr++ = str[i];
    *strr = 0;
}


// convert an integer to a string padded with a leading character
// p is a pointer to the destination
// nbr is the number to convert (can be signed in which case the number is preceeded by '-')
// padch is the leading padding char (usually a space)
// maxch is the desired width of the resultant string (incl padding chars)
// radix is the base of the number.  Base 10 is signed, all others are unsigned
// Special case (used by FloatToStr() only):
//     if padch is negative and nbr is zero prefix the number with the - sign
void IntToStrPad(char *p, MMINTEGER nbr, signed char padch, int maxch, int radix) {
    char sign = 0;
    if ((nbr < 0 && radix == 10 && (uint64_t) nbr != 0x8000000000000000) || padch < 0) {  // if the number is negative or we are forced to use a - symbol
        sign = '-';                                                 // set the sign
        nbr *= -1;                                                  // convert to a positive nbr
        padch = abs(padch);
    }
    else {
        if (nbr >= 0 && maxch < 0 && radix == 10)                   // should we display the + sign?
            sign = '+';
    }

    char buf[IntToStrBufSize];
    IntToStr(buf, nbr, radix);
    int j = abs(maxch) - strlen(buf);                               // calc padding required
    if (j <= 0) j = 0;
    else memset(p, padch, abs(maxch));                              // fill the buffer with the padding char
    if (sign != 0) {                                                // if we need a sign
        if (j == 0) j = 1;                                          // make space if necessary
        if (padch == '0')
            p[0] = sign;                                            // for 0 padding the sign is before the padding
        else
            p[j - 1] = sign;                                        // for anything else the padding is before the sign
    }
    strcpy(&p[j], buf);
}


// convert a float to a string including scientific notation if necessary
// p is the buffer to store the string
// f is the number
// m is the nbr of chars before the decimal point (if negative print the + sign)
// n is the nbr chars after the point
//     if n == STR_AUTO_PRECISION we should automatically determine the precision
//     if n is negative always use exponential format
// ch is the leading pad char
void FloatToStr(char *p, MMFLOAT f, int m, int n, unsigned char ch) {
    int exp, trim = false, digit;
    MMFLOAT rounding;
    char *pp;

    ch &= 0x7f;                                                     // make sure that ch is an ASCII char
    if(f == 0)
        exp = 0;
    else
        exp = floorf(log10f(fabsf(f)));                             // get the exponent part
    if(((fabsf(f) < 0.0001 || fabsf(f) >= 1000000) && f != 0 && n == STR_AUTO_PRECISION) || n < 0) {
        // we must use scientific notation
        f /= powf(10, exp);                                         // scale the number to 1.2345
        if(f >= 10) { f /= 10; exp++; }
        if(n < 0) n = -n;                                           // negative indicates always use exponantial format
        FloatToStr(p, f, m, n, ch);                                 // recursively call ourself to convert that to a string
        p = p + strlen(p);
        *p++ = 'e';                                                 // add the exponent
        if(exp >= 0) {
            *p++ = '+';
            IntToStrPad(p, exp, '0', 2, 10);                        // add a positive exponent
        } else {
            *p++ = '-';
            IntToStrPad(p, exp * -1, '0', 2, 10);                   // add a negative exponent
        }
    } else {
        // we can treat it as a normal number

        // first figure out how many decimal places we want.
        // n == STR_AUTO_PRECISION means that we should automatically determine the precision
        if(n == STR_AUTO_PRECISION) {
            trim = true;
            n = STR_SIG_DIGITS - exp;
            if(n < 0) n = 0;
        }

        // calculate rounding to hide the vagaries of floating point
        if(n > 0)
            rounding = 0.5/powf(10, n);
        else
            rounding = 0.5;
        if(f > 0) f += rounding;                                    // add rounding for positive numbers
        if(f < 0) f -= rounding;                                    // add rounding for negative numbers

        // convert the digits before the decimal point
        if((int)f == 0 && f < 0)
            IntToStrPad(p, 0, -ch, m, 10);                          // convert -0 incl padding if necessary
        else
            IntToStrPad(p, f, ch, m, 10);                           // convert the integer incl padding if necessary
        p += strlen(p);                                             // point to the end of the integer
        pp = p;

        // convert the digits after the decimal point
        if(f < 0) f = -f;                                           // make the number positive
        if(n > 0) {                                                 // if we need to have a decimal point and following digits
            *pp++ = '.';                                            // add the decimal point
            f -= floorf(f);                                         // get just the fractional part
            while(n--) {
                f *= 10;
                digit = floorf(f);                                  // get the next digit for the string
                f -= digit;
                *pp++ = digit + '0';
            }

            // if we do not have a fixed number of decimal places step backwards removing trailing zeros and the decimal point if necessary
            while(trim && pp > p) {
                pp--;
                if(*pp == '.') break;
                if(*pp != '0') { pp++; break; }
            }
        }
        *pp = 0;
    }
}



/**********************************************************************************************
Various routines to clear memory or the interpreter's state
**********************************************************************************************/


// clear (or delete) variables
// if level is not zero it will only delete local variables at that level or greater
// if level is zero to will delete all variables and reset global settings
void ClearVars(int level) {

    vartbl_delete_all(level);

    // Step through the for...next table and remove any loops at the level or greater
    for (int i = 0; i < forindex; i++) {
        if(forstack[i].level >= level) {
            forindex = i;
            break;
        }
    }

    // Step through the do...loop table and remove any loops at the level or greater
    for (int i = 0; i < doindex; i++) {
        if(dostack[i].level >= level) {
            doindex = i;
            break;
        }
    }

    if (level != 0) return;

    assert(varcnt == 0);
    assert(vartbl_free_idx == 0);

    forindex = 0;
    doindex = 0;
    LocalIndex = 0;                                                 // signal that all space is to be cleared
    ClearTempMemory();                                              // clear temp string space

    OptionBase = 0;
    DimUsed = false;
}


// clear all stack pointers (eg, FOR/NEXT stack, DO/LOOP stack, GOSUB stack, etc)
// this is done at the command prompt or at any break
void ClearStack(void) {
    NextData = 0;
    NextDataLine = ProgMemory;
    forindex = 0;
    doindex = 0;
    gosubindex = 0;
    LocalIndex = 0;
    TempMemoryIsChanged = true;                                     // signal that temporary memory should be checked
#if defined(__mmb4l__)
    extern void cmd_read_clear_cache(void);
    cmd_read_clear_cache();
    interrupt_clear();
#else
    InterruptReturn = NULL;
#endif
}


// clear the runtime (eg, variables, external I/O, etc) includes ClearStack() and ClearVars()
// this is done before running a program
void ClearRuntime(void) {
    graphics_term();
#if defined(MX470)
    //have to stop audio before we clear variables to avoid exception
    CloseAudio();
    vol_left = 100; vol_right = 100;
#endif
#if defined(MX470) || defined(__386__)
    OptionFileErrorAbort = true;
#endif
    ClearStack();
//    ClearVars(0);
    OptionExplicit = false;
    DefaultType = T_NBR;
#if defined(__mmb4l__)
    mmb_options.resolution = kCharacter;
    mmb_options.codepage = NULL;
#endif
#if defined(MICROMITE) && !defined(LITE)
    ds18b20Timers = NULL;                                           // InitHeap() will recover the memory allocated to this array
#endif
    CloseAllFiles();
    ClearExternalIO();                                              // this MUST come before InitHeap()
#if defined(__mmb4l__)
    mmb_error_state_ptr = &mmb_normal_error_state;
    error_init(mmb_error_state_ptr);
#else
    OptionErrorSkip = 0;
    MMerrno = 0;                                                    // clear the error flags
    *MMErrMsg = 0;
#endif
    InitHeap();
    ClearVars(0);
    CurrentLinePtr = ContinuePoint = NULL;
    funtbl_clear();
}



// clear everything including program memory (includes ClearStack() and ClearRuntime())
// this is used before loading a program
void ClearProgram(void) {
    ClearRuntime();
#if defined(__mmb4l__)
    memset(error_file, 0, STRINGSIZE);
    error_line = -1;
#else
    StartEditPoint = NULL;
    StartEditChar = 0;
#endif
    TraceOn = false;
}



#if defined(__mmb4l__)
int32_t FloatToInt32(MMFLOAT x) {
    if (x < (MMFLOAT) LONG_MIN - 0.5 || x > (MMFLOAT) LONG_MAX + 0.5)
        error("Number too large");
    return x >= 0 ? (int32_t)(x + 0.5) : (int32_t)(x - 0.5);
}

int64_t FloatToInt64(MMFLOAT x) {
    if (x < (MMFLOAT) LLONG_MIN - 0.5 || x > (MMFLOAT) LLONG_MAX + 0.5)
        error("Number too large");
    if ((x < -FLOAT_ROUNDING_LIMIT) || (x > FLOAT_ROUNDING_LIMIT))
        return (int64_t) x;
    else
        return x >= 0 ? (int64_t)(x + 0.5) : (int64_t)(x - 0.5);
}

#else
// round a float to an integer
int FloatToInt32(MMFLOAT x) {
    if(x < LONG_MIN - 0.5 || x > LONG_MAX + 0.5)
        error("Number too large");
    return (x >= 0 ? (int)(x + 0.5) : (int)(x - 0.5)) ;
}



MMINTEGER FloatToInt64(MMFLOAT x) {
    if(x < (-(0x7fffffffffffffffLL) -1) - 0.5 || x > 0x7fffffffffffffffLL + 0.5)
        error("Number too large");
   if ((x < -FLOAT_ROUNDING_LIMIT) || (x > FLOAT_ROUNDING_LIMIT))
       return (MMINTEGER)(x);
   else
       return (x >= 0 ? (MMINTEGER)(x + 0.5) : (MMINTEGER)(x - 0.5)) ;
}
#endif



// make a string uppercase
void makeupper(char *p) {
    while(*p) {
        *p = toupper(*p);
        p++;
    }
}



// skip to the end of a variable
const char *skipvar(const char *p, int noerror) {
    const char *pp;
    const char *tp;
    int i;
    int inquote = false;

    tp = p;
    // check the first char for a legal variable name
    skipspace(p);
    if(!isnamestart(*p)) return tp;

    do {
        p++;
    } while(isnamechar(*p));

    if(p - tp > MAXVARLEN) {
        if(noerror) return p;
        error("Variable name too long");
        return NULL;
    }

    // check the terminating char.
    if(*p == '$' || *p == '%' || *p == '!') p++;

    // THW: 22-Jan-2025: moved above the check for the terminating char.
    // if(p - tp > MAXVARLEN) {
    //     if(noerror) return p;
    //     error("A Variable name too long");
    // }

    pp = p; skipspace(pp); if(*pp == '(') p = pp;
    if(*p == '(') {
        // this is an array - THW: actually might be a function.

        p++;

        // THW: 22-Jan-2025: removed, I don't think it makes sense, and
        //                   why is it including the opening bracket in
        //                   the variable name length?
        // if(p - tp > MAXVARLEN) {
        //     if(noerror) return p;
        //     error("Variable name too long");
        // }

        // step over the parameters keeping track of nested brackets
        i = 1;
        while(1) {
            if(*p == '\"') inquote = !inquote;
            if(*p == 0) {
                if(noerror) return p;
                error("Expected closing bracket");
                return NULL;
            }
            if(!inquote) {
                if(*p == ')') if(--i == 0) break;
                if(*p == '(' || (tokentype(*p) & T_FUN)) i++;
            }
            p++;
        }
        p++;        // step over the closing bracket
    }
    return p;
}



// skip to the end of an expression (terminates on null, comma, comment or unpaired ')'
const char *skipexpression(const char *p) {
    int i, inquote;

    for(i = inquote = 0; *p; p++) {
        if(*p == '\"') inquote = !inquote;
        if(!inquote) {
            if(*p == ')') i--;
            if(*p == '(' || (tokentype(*p) & T_FUN)) i++;
        }
        if(i < 0 || (i == 0 && (*p == ',' || *p == '\''))) break;
    }
    return p;
}


// find the next command in the program
// this contains the logic for stepping over a line number and label (if present)
// p is the current place in the program to start the search from
// CLine is a pointer to a char pointer which in turn points to the start of the current line for error reporting (if NULL it will be ignored)
// EOFMsg is the error message to use if the end of the program is reached
// returns a pointer to the next command
const char *GetNextCommand(const char *p, const char **CLine, const char *EOFMsg) {
    do {
        if(*p != T_NEWLINE) {                                       // if we are not already at the start of a line
            while(*p) p++;                                          // look for the zero marking the start of an element
            p++;                                                    // step over the zero
        }
        if(*p == 0) {
            if(EOFMsg == NULL) return p;
            error(EOFMsg);
        }
        if(*p == T_NEWLINE) {
            if(CLine) *CLine = p;                                   // and a pointer to the line also for error reporting
            p++;
        }
        if(*p == T_LINENBR) p += 3;

        skipspace(p);
        if(p[0] == T_LABEL) {                                       // got a label
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }
    } while(*p < C_BASETOKEN);
    return p;
}


// scans text looking for the matching closing bracket
// it will handle nested strings, brackets and functions
// it expects to be called pointing at the opening bracket or a function token
const char *getclosebracket(const char *p) {
    int i = 0;
    int inquote = false;

    do {
        if(*p == 0) error("Expected closing bracket");
        if(*p == '\"') inquote = !inquote;
        if(!inquote) {
            if(*p == ')') i--;
            if(*p == '(' || (tokentype(*p) & T_FUN)) i++;
        }
        p++;
    } while(i);
    return p - 1;
}


// check that there is no excess text following an element
// will skip spaces and abort if a zero char is not found
void checkend(const char *p) {
    skipspace(p);
    if(*p == '\'') return;
    if(*p)
        error("Unexpected text: $", p);
}


// count the length of a program line excluding the terminating zero byte
// the pointer p must be pointing to the T_NEWLINE token at the start of the line
int GetLineLength(char *p) {
    char *start;
    start = p;
    p++;                                                            // step over the newline token
    if(*p == T_LINENBR) p += 3;                                     // step over a line number
    while(!(p[0] == 0 && (p[1] == 0 || p[1] == T_NEWLINE))) p++;
    return (p - start);
}



/********************************************************************************************************************************************
A couple of I/O routines that do not belong anywhere else
*********************************************************************************************************************************************/


// print a string to the console interfaces
#if !defined(__mmb4l__)
void MMPrintString(const char* s) {
  while(*s) {
      MMputchar(*s);
      s++;
  }
}
#endif

// output a string to a file
// the string must be a MMBasic string
#if !defined(__mmb4l__)
void MMfputs(const char *p, int filenbr) {
  int i;
  i = *p++;
  while(i--) MMfputc(*p++, filenbr);
}
#endif




/********************************************************************************************************************************************
 string routines
 these routines form a library of functions for manipulating MMBasic strings.  These strings differ from ordinary C strings in that the length
 of the string is stored in the first byte and the string is NOT terminated with a zero valued byte.  This type of string can store the full
 range of binary values (0x00 to 0xff) in each character.
*********************************************************************************************************************************************/

// convert a MMBasic string to a C style string
// if the MMstr contains a null byte that byte is skipped and not copied
char *MtoC(char *p) {
    int i;
    char *p1, *p2;
    i = *p;
    p1 = p + 1; p2 = p;
    while(i) {
        if(p1) *p2++ = *p1;
        p1++;
        i--;
    }
    *p2 = 0;
    return p;
}


// convert a c style string to a MMBasic string
char *CtoM(char *p) {
    int len, i;
    char *p1, *p2;
    len = i = strlen(p);
    if(len > MAXSTRLEN) error("String too long");
    p1 = p + len; p2 = p + len - 1;
    while(i--) *p1-- = *p2--;
    *p = len;
    return p;
}


// copy a MMBasic string to a new location
void Mstrcpy(char *dest, const char *src) {
    int i;
    i = *src + 1;
    while(i--) *dest++ = *src++;
}



// concatenate two MMBasic strings
void Mstrcat(char *dest, const char *src) {
    int i;
    i = *src;
    *dest += i;
    dest += *dest + 1 - i; src++;
    while(i--) *dest++ = *src++;
}



// compare two MMBasic style strings
// returns 1 if s1 > s2  or  0 if s1 = s2  or  -1 if s1 < s2
int Mstrcmp(const char *s1, const char *s2) {
    register int i;
    register const char *p1, *p2;

    // get the smaller length
    i = *s1 < *s2 ? *s1 : *s2;

    // skip the length byte and point to the char array
    p1 = s1 + 1; p2 = s2 + 1;

    // compare each char
    while(i--) {
        if(*p1 > *p2) return 1;
        if(*p1 < *p2) return -1;
        p1++; p2++;
    }
    // up to this point the strings matched - make the decision based on which one is shorter
    if(*s1 > *s2) return 1;
    if(*s1 < *s2) return -1;
    return 0;
}


// utility routine used by DoDim() and other places in the interpreter
// checks if the type has been explicitly specified as in DIM FLOAT A, B, ... etc
const char *CheckIfTypeSpecified(const char *p, int *type, int AllowDefaultType) {
    const char *tp;

    if((tp = checkstring(p, "INTEGER")) != NULL)
        *type = T_INT | T_IMPLIED;
    else if((tp = checkstring(p, "STRING")) != NULL)
        *type = T_STR | T_IMPLIED;
    else if((tp = checkstring(p, "FLOAT")) != NULL)
        *type = T_NBR | T_IMPLIED;
    else {
        if(!AllowDefaultType) error("Variable type");
        tp = p;
        *type = DefaultType;                                        // if the type is not specified use the default
    }
    return tp;
}



/**
 * Gets the address for a MMBasic interrupt.
 *
 * This will handle a line number, a label or a subroutine,
 * all areas of MMBasic that can generate an interrupt use this function.
 */
const char *GetIntAddress(const char *p) {
    if (isnamestart(*p)) {          // if it starts with a valid name char
        char name[MAXVARLEN + 1];
        MmResult result = parse_name(&p, name);

        int fun_idx;
        if (SUCCEEDED(result)) result = funtbl_find(name, kLabel | kSub, &fun_idx);
        switch (result) {
            case kOk:
                return funtbl[fun_idx].addr;
            case kNameTooLong:
                error_throw_ex(result, "Label/subroutine name too long");
                return NULL;
            case kFunctionNotFound:
                error_throw_ex(result, "Label/subroutine not found");
                return NULL;
            case kFunctionTypeMismatch:
                error_throw_ex(result, "Not a label/subroutine");
                return NULL;
            default:
                error_throw(result);
                return NULL;
        }
    }

    return findline(getinteger(p), true);  // otherwise try for a line number
}


#if defined(DEBUGMODE)

// debug,  dump the variable table
void DumpVarTbl(void) {
    int i;
    dp("idx  name        type level");
    for(i = 0; i < varcnt; i++) {
        char c;
        c = vartbl[i].name[10]; vartbl[i].name[10] = 0;
        dp("%2d: %-10s  %4X  %2d", i, vartbl[i].name, vartbl[i].type, vartbl[i].level);
        vartbl[i].name[10] = c;
    }
}
#endif

