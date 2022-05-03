/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmb4l.h

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#if !defined MMB4L_H
#define MMB4L_H

#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../../Version.h"
#include "../Configuration.h"
#include "../../MMBasic/MMBasic.h"
#include "../../MMBasic/VarTable.h"
#include "options.h"
#include "memory.h"

#define PI_VALUE  3.14159265358979323

#define ClearSavedVars()    {}
#define TestStackOverflow()  {}

// Aliases for MMBasic's global variables, all prefixed with g_
#define g_current_var_idx  VarIndex
#define g_float_rtn        fret
#define g_integer_rtn      iret
#define g_rtn_type         targ
#define g_string_rtn       sret
#define g_subfun           subfun
#define g_var_tbl          vartbl

void CheckAbort(void);
void FlashWriteInit(char *p, int nbr);
char *GetIntAddress(char *p);
void ListProgram(char *p, int all);
char *llist(char *b, char *p);
int MMgetchar(void);
void cmd_dummy(void);
void op_equal(void);

#define JMP_BREAK  1
#define JMP_END    2
#define JMP_ERROR  3
#define JMP_NEW    4
#define JMP_QUIT   5
#define JMP_UNEXPECTED  999

extern uint8_t mmb_exit_code;
extern Options mmb_options;

extern char *CFunctionFlash;
extern char DimUsed;
extern char *errorstack[MAXGOSUB];
extern int gosubindex;
extern char *gosubstack[MAXGOSUB];
extern int IgnorePIN;
extern int MMCharPos;
extern int WatchdogSet;

#endif
