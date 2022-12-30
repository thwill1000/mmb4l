/*-*****************************************************************************

MMBasic for Linux (MMB4L)

funtbl.c

Copyright 2011-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

// FNV hash parameters for 32-bit hashes (https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function)
#define FNV_PRIME         16777619
#define FNV_OFFSET_BASIS  2166136261

#define MAXSUBHASH        MAXSUBFUN

struct s_funtbl funtbl[MAXSUBFUN];
const char *subfun[MAXSUBFUN];

void funtbl_clear() {
    memset(funtbl, 0, sizeof(funtbl));
}

void funtbl_prepare(bool abort_on_error) {
    char name[MAXVARLEN + 1];

    funtbl_clear();

    for (size_t ii = 0; ii < MAXSUBFUN && subfun[ii]; ++ii) {
        const char *p = subfun[ii];  // p is pointing at the SUB/FUNCTION keyword.
        p++;
        skipspace(p);                // p1 is pointing at the beginning of the SUB/FUNCTION name.

        HashValue hash;
        if (funtbl_hash(p, name, &hash) != 0 && abort_on_error) {
            error("SUB/FUNCTION name too long");
        }

        while (funtbl[hash].name[0]) {
            if (strcmp(funtbl[hash].name, name) == 0) break;
            hash++;
            if (hash == MAXSUBFUN) hash = 0;
        }

        if (strcmp(funtbl[hash].name, name) == 0) {
            if (abort_on_error) error("Duplicate SUB/FUNCTION declaration");
        } else {
            funtbl[hash].index = ii;
            strcpy(funtbl[hash].name, name);
        }
    }
}

void funtbl_dump() {
    for (int ii = 0; ii < MAXSUBFUN; ++ii) {
        if (funtbl[ii].name[0]) printf("[%d] %s, %d\n", ii, funtbl[ii].name, funtbl[ii].index);
    }
}

size_t funtbl_size() {
    size_t sz = 0;
    for (int ii = 0; ii < MAXSUBFUN; ++ii) {
        if (funtbl[ii].name[0]) sz++;
    }
    return sz;
}

int funtbl_find(const char *p) {
    char name[MAXVARLEN + 1] = { 0 };
    HashValue hash;
    if (funtbl_hash(p, name, &hash) != 0) {
        error("SUB/FUNCTION name too long");
        return -1;
    }

    while (funtbl[hash].name[0]) {
        if (strcmp(funtbl[hash].name, name) == 0) return funtbl[hash].index;
        hash = (hash + 1) % MAXSUBFUN;
    }

    return -1;
}

int funtbl_hash(const char *p, char *name, HashValue* hash) {
    int namelen = 0;
    *hash = FNV_OFFSET_BASIS;
    while (isnamechar(*p)) {
        *name = toupper(*p);
        *hash ^= *name;
        *hash *= FNV_PRIME;
        if (++namelen > MAXVARLEN) break; // Which will truncate the name.
        p++;
        name++;
    }
    *name = '\0';
    *hash %= MAXSUBHASH; // Scale hash to size of function table.
    return namelen > MAXVARLEN ? -1 : 0;
}
