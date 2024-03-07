/*-*****************************************************************************

MMBasic for Linux (MMB4L)

Commands.h

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



/**********************************************************************************
 All other tokens (keywords, functions, operators) should be inserted in this table
**********************************************************************************/
#ifdef INCLUDE_TOKEN_TABLE

  { "For",        T_NA,              0, op_invalid    },
  { "Else",       T_NA,              0, op_invalid    },
  { "GoSub",      T_NA,              0, op_invalid    },
  { "GoTo",       T_NA,              0, op_invalid    },
  { "Step",       T_NA,              0, op_invalid    },
  { "Then",       T_NA,              0, op_invalid    },
  { "To",         T_NA,              0, op_invalid    },
  { "Until",      T_NA,              0, op_invalid    },
  { "While",      T_NA,              0, op_invalid    },

#endif




#if !defined(INCLUDE_COMMAND_TABLE) && !defined(INCLUDE_TOKEN_TABLE)

struct s_forstack {
    const char *forptr;                     // pointer to the FOR command in program memory
    const char *nextptr;                    // pointer to the NEXT command in program memory
    void *var;                              // value of the FOR variable
    char vartype;                           // type of the variable
    char level;                             // the sub/function level that the loop was created
    union u_totype {
        MMFLOAT f;                          // the TO value if it is a MMFLOAT
        long long int i;                    // the TO value if it is an integer
    } tovalue;
    union u_steptype {
        MMFLOAT f;                          // the STEP value if it is a MMFLOAT
        long long int i;                    // the STEP value if it is an integer
    } stepvalue;
};

extern struct s_forstack forstack[MAXFORLOOPS + 1] ;
extern int forindex;

struct s_dostack {
    const char *evalptr;                    // pointer to the expression to be evaluated
    const char *loopptr;                    // pointer to the loop statement
    const char *doptr;                      // pointer to the DO statement
    char level;                             // the sub/function level that the loop was created
};

extern struct s_dostack dostack[MAXDOLOOPS];
extern int doindex;

extern const char *gosubstack[MAXGOSUB];
extern const char *errorstack[MAXGOSUB];
extern int gosubindex;
extern char DimUsed;

// char *GetFileName(char* CmdLinePtr, char *LastFilePtr);
// void mergefile(char *fname, char *MemPtr);
void ListProgram(char *p, int all);
char *llist(char *b, char *p);

#if !defined(__mmb4l__)
#define CONFIG_TITLE      0
#define CONFIG_LOWER      1
#define CONFIG_UPPER      2
#endif

extern unsigned int BusSpeed;
extern char *OnKeyGOSUB;
extern char EchoOption;

#endif
