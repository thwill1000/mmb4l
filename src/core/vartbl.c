/*-*****************************************************************************

MMBasic for Linux (MMB4L)

vartbl.c

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

#include "vartbl.h"
#include "../common/hash.h"
#include "../common/mmb4l.h"

#include <assert.h>
#include <string.h>

bool vartbl_init_called = false;
VarHashValue vartbl_hashmap[VARS_HASHMAP_SIZE];
int vartbl_free_idx = 0;
int varcnt = 0;

void vartbl_init() {
    assert(!vartbl_init_called);
    varcnt = 0;
    vartbl_free_idx = 0;
    memset(vartbl, 0, MAXVARS * sizeof(struct s_vartbl));
    memset(vartbl_hashmap, 0xFF, sizeof(vartbl_hashmap));
    vartbl_init_called = true;
}

int vartbl_add(
        const char *name,
        uint8_t type,
        uint8_t level,
        DIMTYPE* dims,
        uint8_t slen) {

    //printf("vartbl_add(%s, %d, %d, ...)\n", name, type, level);

    assert(vartbl_init_called);
    assert((type & T_STR) || (slen == 0));
    assert(!(type & T_STR) || (slen > 0));

    // The T_PTR flag is expected to be only set after a variable has
    // already been added to the table.
    assert(!(type & T_PTR));

    // Find a free slot.
    while (vartbl_free_idx < varcnt
            && vartbl[vartbl_free_idx].type != T_NOTYPE) {
        vartbl_free_idx++;
    }

    //printf("vartbl_free_idx = %d\n", vartbl_free_idx);

    if (vartbl_free_idx == MAXVARS) return -1;

    // In theory verify sufficient variable memory is available and/or allocate
    // more. However in practice MMB4L uses a fixed sized variable table so this
    // is unnecessary.
#if 0
    if (vartbl_free_idx == varcnt) {
        m_alloc(M_VAR, (varcnt + 1) * sizeof(struct s_vartbl));
    }
#endif

    int var_idx = vartbl_free_idx;

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
    vartbl_free_idx++;

    // If we have used a new slot then we increment 'varcnt'.
    if (vartbl_free_idx > varcnt) varcnt++;

    // Record variable in the hashmap.
    VarHashValue hash = hash_cstring(name, MAXVARLEN) % VARS_HASHMAP_SIZE;
    VarHashValue original_hash = hash;
    while (vartbl_hashmap[hash] >= 0) {
        hash = (hash + 1) % VARS_HASHMAP_SIZE;
        if (hash == original_hash) return -3; // Should never happen in production because the map
                                              // size is larger than the maximum numer of variables.
    }
    vartbl_hashmap[hash] = var_idx;
    vartbl[var_idx].hash = hash;

    return var_idx;
}

void vartbl_delete(int var_idx) {

    assert(vartbl_init_called);
    assert(var_idx >= 0);

    if (var_idx >= varcnt) return;

    //printf("vartbl_delete(%d = %s)\n", var_idx, vartbl[var_idx].name);

    vartbl_hashmap[vartbl[var_idx].hash] = DELETED_HASH;

    // FreeMemory associated with string and array variables unless they are pointers.
    if (((vartbl[var_idx].type & T_STR) || vartbl[var_idx].dims[0] != 0)
            && !(vartbl[var_idx].type & T_PTR)) {
        FreeMemory(vartbl[var_idx].val.s); // Free any memory (if allocated).
    }
    memset(vartbl + var_idx, 0x0, sizeof(struct s_vartbl));
    if (var_idx == varcnt - 1) varcnt--;
    if (var_idx < vartbl_free_idx) vartbl_free_idx = var_idx;
}

void vartbl_delete_all(uint8_t level) {

    assert(vartbl_init_called);
    assert(level >= 0);

    // We traverse the table in reverse so that 'varcnt' will be decremented
    // as much as possible.
    const int original_count = varcnt;
    for (int ii = original_count - 1; ii >= 0; --ii) {
        if (vartbl[ii].level >= level) {
            vartbl_delete(ii);
        }
    }

    // When deleting ALL variables including globals then mark all the hashmap
    // slots as UNUSED and not just DELETED.
    if (level == 0) {
        assert(varcnt == 0);
        memset(vartbl_hashmap, 0xFF, sizeof(vartbl_hashmap));
    }
}

MmResult vartbl_find(
        const char *name, uint8_t level, int *var_idx, int *global_idx) {

    assert(vartbl_init_called);

//    printf("vartbl_find(\"%s\", %d, ...)\n", name, level);

    *var_idx = -1;
    int tmp;  // So we don't have to keep checking if global_idx is NULL or not.
    if (!global_idx) global_idx = &tmp;
    *global_idx = -1;

    MmResult result = kVariableNotFound;
    VarHashValue hash = hash_cstring(name, MAXVARLEN) % VARS_HASHMAP_SIZE;
    VarHashValue original_hash = hash;

    do {
        *var_idx = vartbl_hashmap[hash];
        if (*var_idx == UNUSED_HASH) break;

        if (*var_idx != DELETED_HASH) {
            // TODO: check 'vartbl' entry is valid.
            assert(vartbl[*var_idx].type != T_NOTYPE);

            // Compare 'name' with referenced 'vartbl' entry.
            // Both names should be in upper-case, but if they are MAXVARLEN
            // chars long then they are not NULL terminated.
            if (strncmp(name, vartbl[*var_idx].name, MAXVARLEN) == 0) {
                if (vartbl[*var_idx].level == 0) *global_idx = *var_idx;
                if (vartbl[*var_idx].level == level) {
                    result = kOk;
                    break;
                }
            }
        }

        hash = (hash + 1) % VARS_HASHMAP_SIZE;
    } while (hash != original_hash);

    return result;
}
