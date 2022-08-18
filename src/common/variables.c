/*-*****************************************************************************

MMBasic for Linux (MMB4L)

variables.c

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

#include "variables.h"

#include "mmb4l.h"

#include <stdbool.h>

int variables_add(const char *name, size_t size) {
    // Copy a maximum of MAXVARLEN characters,
    // a maximum length stored name will not be '\0' terminated.
    strncpy(vartbl[varcnt].name, name, MAXVARLEN);
    char *mptr = size == 0 ? NULL : GetMemory(size);
    vartbl[varcnt].val.s = mptr;
    return varcnt++;
}

void variables_delete(int var_idx) {
    if (var_idx >= varcnt) return;

    // TODO: according to the MMBasic for DOS code FreeMemory() will ignore
    //       invalid addresses, I suspect this is bollocks and this code
    //       will do bad things if called for scalar variables.
    FreeMemory(vartbl[var_idx].val.s);
    memset(vartbl + var_idx, 0x0, sizeof(struct s_vartbl));
    if (var_idx == varcnt - 1) varcnt--;
}

int variables_find(const char *name) {
    for (int ii = 0; ii < varcnt; ++ii) {
        // Only compares first MAXVARLEN characters.
        if (strncmp(name, vartbl[ii].name, MAXVARLEN) == 0) return ii;
    }
    return -1;
}
