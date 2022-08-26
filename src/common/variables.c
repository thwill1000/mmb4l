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

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

int variables_free_idx = 0;

void variables_init() {
    varcnt = 0;
    variables_free_idx = 0;
}

int variables_add(const char *name, uint8_t level, size_t size) {
    // Find a free slot.
    while (variables_free_idx < varcnt
            && vartbl[variables_free_idx].type != T_NOTYPE) variables_free_idx++;

    if (variables_free_idx == MAXVARS) return -1;

    // Copy a maximum of MAXVARLEN characters,
    // a maximum length stored name will not be '\0' terminated.
    int var_idx = variables_free_idx;
    strncpy(vartbl[var_idx].name, name, MAXVARLEN);
    vartbl[var_idx].type = T_INT;  // Placeholder.
    vartbl[var_idx].level = level;
    char *mptr = size == 0 ? NULL : GetMemory(size);
    vartbl[var_idx].val.s = mptr;

    // Increment because next time this is called there is no point in
    // looking in a slot we have just used.
    variables_free_idx++;

    // If we've used a new slot then we increment 'varcnt'.
    if (variables_free_idx > varcnt) varcnt++;

    return var_idx;
}

void variables_delete(int var_idx) {
    if (var_idx >= varcnt) return;

    // TODO: according to the MMBasic for DOS code FreeMemory() will ignore
    //       invalid addresses, I suspect this is bollocks and this code
    //       can do bad things if called for scalar variables.
    FreeMemory(vartbl[var_idx].val.s);
    memset(vartbl + var_idx, 0x0, sizeof(struct s_vartbl));
    if (var_idx == varcnt - 1) varcnt--;
    if (var_idx < variables_free_idx) variables_free_idx = var_idx;
}

void variables_delete_all(uint8_t level) {
    assert(level >= 0);

    // We traverse the table in reverse so that 'varcnt' will be decremented
    // as much as possible.
    const int original_count = varcnt;
    for (int ii = original_count - 1; ii >= 0; --ii) {
        if (vartbl[ii].level >= level) {
            variables_delete(ii);
        }
    }
}

int variables_find(const char *name) {
    for (int ii = 0; ii < varcnt; ++ii) {
        // Only compares first MAXVARLEN characters.
        if (strncmp(name, vartbl[ii].name, MAXVARLEN) == 0) return ii;
    }
    return -1;
}
