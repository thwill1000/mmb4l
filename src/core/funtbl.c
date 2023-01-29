/*-*****************************************************************************

MMBasic for Linux (MMB4L)

funtbl.c

Copyright 2011-2023 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include "../Hardware_Includes.h"
#include "MMBasic_Includes.h"
#include "funtbl.h"

#include <stddef.h>

struct s_funtbl funtbl[MAXSUBFUN];
FunHashValue funtbl_hashmap[FUN_HASHMAP_SIZE];
size_t funtbl_count = 0;

MmResult funtbl_add(
        const char *name, FunType type, const char *addr, int *fun_idx) {
    *fun_idx = -1;
    if (funtbl_count == MAXSUBFUN) return kTooManyTargets;
    if (addr < ProgMemory || addr >= ProgMemory + PROG_FLASH_SIZE) return kInternalFault;

    // Record function in the hashmap.
    FunHashValue hash = hash_cstring(name, MAXVARLEN) % FUN_HASHMAP_SIZE;
    FunHashValue original_hash = hash;
    while (funtbl_hashmap[hash] >= 0) {
        if (strncmp(funtbl[funtbl_hashmap[hash]].name, name, MAXVARLEN) == 0) {
            // Functions and Subs share a namespace but Labels do not.
            switch (funtbl[funtbl_hashmap[hash]].type) {
                case kFunction:
#if !defined(__clang__)
                    [[fallthrough]];
#endif
                case kSub:
                    if (type == kFunction || type == kSub) return kDuplicateTarget;
                    break;
                case kLabel:
                    if (type == kLabel) return kDuplicateTarget;
                    break;
                default:
                    *fun_idx = -1;
                    return kInternalFault;
            }
        }
        hash = (hash + 1) % FUN_HASHMAP_SIZE;
        if (hash == original_hash) return kHashmapFull; // Should never happen in production because
                                                        // the hashmap is larger than the function
                                                        // table.
    }
    funtbl_hashmap[hash] = funtbl_count;

    // Copy a maximum of MAXVARLEN characters,
    // a maximum length stored name will not be '\0' terminated.
    strncpy(funtbl[funtbl_count].name, name, MAXVARLEN);
    funtbl[funtbl_count].type = type;
    funtbl[funtbl_count].hash = hash;
    funtbl[funtbl_count].addr = addr;

    *fun_idx = funtbl_count++;
    return kOk;
}

void funtbl_clear() {
    memset(funtbl, 0, sizeof(funtbl));
    memset(funtbl_hashmap, 0xFF, sizeof(funtbl_hashmap));
    funtbl_count = 0;
}

void funtbl_dump() {
    for (int ii = 0; ii < MAXSUBFUN; ++ii) {
        if (funtbl[ii].name[0]) printf(
                "[%d] %s, type = %d, hash = %d, addr = %ld\n",
                ii,
                funtbl[ii].name,
                funtbl[ii].type,
                funtbl[ii].hash,
                (uint64_t) funtbl[ii].addr);
    }
}

MmResult funtbl_find(const char *name, uint8_t type_mask, int *fun_idx) {
    FunHashValue hash = hash_cstring(name, MAXVARLEN) % FUN_HASHMAP_SIZE;
    FunHashValue original_hash = hash;
    int mismatch = -1;

    do {
        *fun_idx = funtbl_hashmap[hash];
        if (*fun_idx == -1) break;

        // Compare 'name' with referenced 'funtbl' entry.
        // Both names should be in upper-case, but if they are MAXVARLEN
        // chars long then they are not NULL terminated.
        if (strncmp(name, funtbl[*fun_idx].name, MAXVARLEN) == 0) {
            switch (funtbl[*fun_idx].type) {
                case kFunction:
                    if (type_mask & kFunction) return kOk;
                    mismatch = *fun_idx;
                    break;
                case kSub:
                    if (type_mask & kSub) return kOk;
                    mismatch = *fun_idx;
                    break;
                case kLabel:
                    if (type_mask & kLabel) return kOk;
                    // Where there is both a mismatched Function or Sub and a
                    // mismatched Label we preferably return the index of the
                    // former.
                    if (mismatch == -1) mismatch = *fun_idx;
                    break;
                default:
                    *fun_idx = -1;
                    return kInternalFault;
            }
        }

        hash = (hash + 1) % FUN_HASHMAP_SIZE;
    } while (hash != original_hash);

    *fun_idx = mismatch;
    return *fun_idx == -1 ? kTargetNotFound : kTargetTypeMismatch;
}
