/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_mid.c

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
#include "../core/tokentbl.h"

#include <string.h>

#define ERROR_NOT_A_STRING              error_throw_ex(kError, "Not a string")
#define ERROR_SELECTION_EXCEEDS_LENGTH  error_throw_ex(kError, "Selection exceeds length of string")
#define ERROR_STRING_TOO_SHORT          error_throw_ex(kError, "Supplied string too short")

void cmd_mid(void){
    getargs(&cmdline, 5, ",");
    findvar(argv[0], V_NOFIND_ERR);
    if (vartbl[VarIndex].type & T_CONST) ERROR_CANNOT_CHANGE_A_CONSTANT;
    if (!(vartbl[VarIndex].type & T_STR)) ERROR_NOT_A_STRING;
    char *sourcestring = getstring(argv[0]);
    int start = getint(argv[2], 1, sourcestring[0]);
    int num = 0;
    if (argc == 5) num = getint(argv[4], 1, sourcestring[0]);
    if (start + num - 1 > sourcestring[0]) ERROR_SELECTION_EXCEEDS_LENGTH;
    while (*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
    if (!*cmdline) ERROR_SYNTAX;
    ++cmdline;
    if (!*cmdline) ERROR_SYNTAX;
    char *value = getstring(cmdline);
    if (num == 0) num = value[0];
    if (num > value[0]) ERROR_STRING_TOO_SHORT;
    char *p = &value[1];
    memcpy(&sourcestring[start], p, num);
}
