/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmb4l_hardware_includes.h

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

// Definitions required to build the MMBasic "core" for MMB4L.

#if !defined(MMB4L_HARDWARE_INCLUDES_H)
#define MMB4L_HARDWARE_INCLUDES_H

#include <errno.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "Version.h"
#include "Configuration.h"
#include "common/console.h"
#include "common/error.h"
#include "common/file.h"
#include "common/interrupt.h"
#include "common/memory.h"
#include "common/options.h"

// Redefine the standard float routines used in MMBasic to their double versions
#define powf pow
#define log10f log10
#define floorf floor
#define fabsf fabs
#define atanf atan
#define cosf cos
#define expf exp
#define logf log
#define sinf sin
#define sqrtf sqrt
#define tanf tan

// Global variables used in MMBasic but must be maintained outside of the interpreter
extern int IgnorePIN;

// Aliases for functions using "legacy" names in MMBasic core:
#define error              error_throw_legacy

// Aliases for identifiers using "legacy" names in MMBasic core:
#define Autorun          autorun
#define DefaultType      mmb_options.default_type
#define error_file       mmb_error_state_ptr->file
#define error_line       mmb_error_state_ptr->line
#define MMErrMsg         mmb_error_state_ptr->message
#define MMerrno          mmb_error_state_ptr->code
#define Option           mmb_options
#define OptionBase       mmb_options.base
#define OptionErrorSkip  mmb_error_state_ptr->skip
#define OptionExplicit   mmb_options.explicit_type

// Functions not used in MMB4L ... so make them go away
#define uSec(a)  {}
#define TestStackOverflow()  {}

#define ClearExternalIO()         // same

void CheckAbort(void);
int MMgetchar(void);

extern char *CFunctionFlash;
extern char *CFunctionLibrary;

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

#endif
