/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_erase.c

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

#include "../common/mmb4l.h"
#include "../common/parse.h"
#include "../common/utility.h"
#include "../common/variables.h"

void cmd_erase(void) {
    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");                // getargs macro must be the first executable stmt in a block
    if ((argc & 0x01) == 0) ERROR_ARGUMENT_COUNT;

    const char *p;
    char name[MAXVARLEN + 1];
    MmResult result = kOk;

    for (int i = 0; i < argc; i += 2) {
        p = argv[i];
        result = parse_name(&p, name);
        if (FAILED(result)) error_throw(result);
        int var_idx = variables_find(name);
        if (var_idx == -1) error_throw_ex(kError, "Cannot find $", name);
        FreeMemory(vartbl[var_idx].val.s);                           // free the memory, note that FreeMemory() will ignore an invalid argument
        vartbl[var_idx].type = T_NOTYPE;                             // empty slot
        *vartbl[var_idx].name = 0;                                   // safety precaution
        for (int k = 0; k < MAXDIM; k++) vartbl[var_idx].dims[k] = 0;// and again
        if (var_idx == varcnt - 1) varcnt--;
    }
}
