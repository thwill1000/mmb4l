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
#include <string.h>

bool variables_init_called = false;
int variables_free_idx = 0;
int varcnt = 0;

void variables_init() {
    assert(!variables_init_called);
    varcnt = 0;
    variables_free_idx = 0;
    memset(vartbl, 0, MAXVARS * sizeof(struct s_vartbl));
    variables_init_called = true;
}

int variables_add(
        const char *name,
        uint8_t type,
        uint8_t level,
        DIMTYPE* dims,
        uint8_t slen) {

    //printf("variables_add(%s, %d, %d, ...)\n", name, type, level);

    assert(variables_init_called);
    assert((type & T_STR) || (slen == 0));
    assert(!(type & T_STR) || (slen > 0));

    // The T_PTR flag is expected to be only set after a variable has
    // already been added to the table.
    assert(!(type & T_PTR));

    // Find a free slot.
    while (variables_free_idx < varcnt
            && vartbl[variables_free_idx].type != T_NOTYPE) {
        variables_free_idx++;
    }

    //printf("variables_free_idx = %d\n", variables_free_idx);

    if (variables_free_idx == MAXVARS) return -1;

    // In theory verify sufficient variable memory is available and/or allocate
    // more. However in practice MMB4L uses a fixed sized variable table so this
    // is unnecessary.
#if 0
    if (variables_free_idx == varcnt) {
        m_alloc(M_VAR, (varcnt + 1) * sizeof(struct s_vartbl));
    }
#endif

    int var_idx = variables_free_idx;

    // IMPORTANT! This code assumes that the slot in the variable table has
    //            already been zeroed out either during the initialisation
    //            of the table or when the variable previously occupying the
    //            slot was deleted.

    // First try to allocate any heap memory required by the variable;
    // this may report an out-of-memory error in which case we do not
    // want to have left a half constructed variable in the table.
    size_t heap_sz = 0;
    if (dims) {
        if (dims[0] != -1) {
            if (type & T_INT) {
                heap_sz = sizeof(MMINTEGER);
            } else if (type & T_NBR) {
                heap_sz = sizeof(MMFLOAT);
            } else if (type & T_STR) {
                heap_sz = (slen + 1);
            }

            for (int ii = 0; ii < MAXDIM && dims[ii] != 0; ++ii) {
                if (dims[ii] <= mmb_options.base) return -2;
                heap_sz *= (dims[ii] + 1 - mmb_options.base);
            }
        } else {
            // "Empty" array used for fun/sub parameter lists.
            // Don't allocate any heap memory.
        }
    } else if (type & T_STR) {
        heap_sz = slen + 1;
    }
    // Memory allocated by GetMemory will have been zeroed out.
    // TODO: At the moment GetMemory() will do a longjmp if there is an error,
    //       it would be better if it returned an error code which we could
    //       handle later.
    vartbl[var_idx].val.s = heap_sz == 0 ? NULL : GetMemory(heap_sz);

    // Copy a maximum of MAXVARLEN characters,
    // a maximum length stored name will not be '\0' terminated.
    strncpy(vartbl[var_idx].name, name, MAXVARLEN);

    vartbl[var_idx].type = type;
    vartbl[var_idx].level = level;
    vartbl[var_idx].size = slen;

    // Copy dimension data.
    if (dims) {
        memcpy(vartbl[var_idx].dims, dims, MAXDIM * sizeof(DIMTYPE));
    }

    // Increment because next time this is called there is no point in
    // looking in a slot we have just used.
    variables_free_idx++;

    // If we've used a new slot then we increment 'varcnt'.
    if (variables_free_idx > varcnt) varcnt++;

    return var_idx;
}

void variables_delete(int var_idx) {

    assert(variables_init_called);

    if (var_idx >= varcnt) return;

    //printf("variables_delete(%d = %s)\n", var_idx, vartbl[var_idx].name);

    // FreeMemory associated with string and array variables unless they are pointers.
    if (((vartbl[var_idx].type & T_STR) || vartbl[var_idx].dims[0] != 0)
            && !(vartbl[var_idx].type & T_PTR)) {
        FreeMemory(vartbl[var_idx].val.s); // Free any memory (if allocated).
    }
    memset(vartbl + var_idx, 0x0, sizeof(struct s_vartbl));
    if (var_idx == varcnt - 1) varcnt--;
    if (var_idx < variables_free_idx) variables_free_idx = var_idx;
}

void variables_delete_all(uint8_t level) {

    assert(variables_init_called);
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

MmResult variables_find(
        const char *name, uint8_t level, int *var_idx, int *global_idx) {

    assert(variables_init_called);

    //printf("variables_find(%s, %d, ...)\n", name, level);

    *var_idx = -1;
    int tmp;  // So we don't have to keep checking if global_idx is NULL or not.
    if (!global_idx) global_idx = &tmp;
    *global_idx = -1;
    for (int ii = 0; ii < varcnt; ++ii) {
        // Only compares first MAXVARLEN characters.
        if (strncmp(name, vartbl[ii].name, MAXVARLEN) == 0) {
            if (vartbl[ii].level == 0) {
                // Found a global.
                if (level == 0) {
                    // Looking for a global, we're done.
                    *var_idx = ii;
                    *global_idx = ii;
                    // if (vartbl[ii].type & T_STR) printf("\"%s\"\n", vartbl[ii].val.s + 1);
                    return kOk;
                } else {
                    // Looking for a local, store but keep looking.
                    *global_idx = ii;
                }
            } else if (vartbl[ii].level == level) {
                // Found the local we are looking for, we're done.
                *var_idx = ii;
                return kOk;
            }
        }
    }
    return kVariableNotFound;
}
