/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmb4l.h

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

#if !defined(MMB4L_H)
#define MMB4L_H

#include "../core/funtbl.h"
#include "../core/MMBasic.h"
#include "../core/vartbl.h"
#include "error.h"
#include "features.h"
#include "memory.h"
#include "options.h"
#include "parse.h"

#define PI_VALUE  3.14159265358979323

#define TestStackOverflow()  {}

// Aliases for MMBasic's global variables, all prefixed with g_
#define g_current_var_idx  VarIndex
#define g_float_rtn        fret
#define g_integer_rtn      iret
#define g_rtn_type         targ
#define g_string_rtn       sret
#define g_subfun           subfun
#define g_var_tbl          vartbl

#define checkstring        parse_check_string

void CheckAbort(void);
const char *llist(char *b, const char *p);
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
extern Features mmb_features;

extern char *CFunctionFlash;
extern char DimUsed;
extern const char *errorstack[MAXGOSUB];
extern int gosubindex;
extern const char *gosubstack[MAXGOSUB];
extern int IgnorePIN;
extern int WatchdogSet;

typedef struct {
    uint32_t next_line_offset;
    uint32_t next_data;
} DataReadPointer;

// various debug macros
#if defined(DEBUGMODE)
    void dump(char *p, int nbr);   // defined in Main.c,  dump an area of memory in hex and ascii
    void DumpVarTbl(void);         // defined in MMBasic.c,  dump the variable table

    #define dp(...) {char s[140];sprintf(s,  __VA_ARGS__); display_puts(s); display_puts("\r\n");}

    #define db(i) {IntToStr(inpbuf, i, 10); display_puts(inpbuf); display_puts("\r\n");}
    #define db2(i1, i2) {IntToStr(inpbuf, i1, 10); display_puts(inpbuf); display_puts("  "); IntToStr(inpbuf, i2, 10); display_puts(inpbuf); display_puts("\r\n");}
    #define db3(i1, i2, i3) {IntToStr(inpbuf, i1, 10); display_puts(inpbuf); display_puts("  "); IntToStr(inpbuf, i2, 10); display_puts(inpbuf); display_puts("  "); IntToStr(inpbuf, i3, 10); display_puts(inpbuf); display_puts("\r\n");}

    #define ds(s) {display_puts(s); display_puts("\r\n");}
    #define ds2(s1, s2) {display_puts(s1); display_puts(s2); display_puts("\r\n");}
    #define ds3(s1, s2, s3) {display_puts(s1); display_puts(s2); display_puts(s3); display_puts("\r\n");}

#endif

#endif // #if !defined(MMB4L_H)
