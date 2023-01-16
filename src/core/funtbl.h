/*-*****************************************************************************

MMBasic for Linux (MMB4L)

funtbl.h

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

#if !defined(MMB4L_FUNTBL_H)
#define MMB4L_FUNTBL_H

#include "../Configuration.h"
#include "../common/hash.h"
#include "../common/mmresult.h"

typedef int16_t FunHashValue;

/** Structure of elements in the function table. */
struct s_funtbl {
    char name[MAXVARLEN];  // Function name canonically in UPPER-CASE; will not
                           // be \0 terminated if MAXVARLEN characters long.
    const char *addr;      // Pointer to function in the program memory.
    FunHashValue hash;     // Index of this function in funtbl_hashmap[].
};

/** Indexes into this table are hashes of the SUB/FUNCTION names. */
extern struct s_funtbl funtbl[MAXSUBFUN];

/**
 * @brief  Hashmap from a hash of the first 32 characters of the
 *         function name to the corresponding entry in the \p funtbl.
 *         Empty hash table entries will contain -1.
 */
extern FunHashValue funtbl_hashmap[FUN_HASHMAP_SIZE];

/**
 * @brief  The number of declared functions (entries in 'funtbl').
 */
extern size_t funtbl_count;

/**
 * @brief  Adds a function to the functions table.
 *
 * @param  name   Name for the function.
 *                @warning  This is case-sensitive, but MMB4L should always call
 *                          it with an UPPER-CASE name.
 *                @warning  Only the first 32 characters are used.
 *                @warning  Validity of the name is not checked.
 * @param  addr   Pointer to the function's declaration in the program memory.
 * @param[out]  fun_idx  On exit, the index of the new function,
 *                       or -1 on error.
 * @return        kOk                - on success.
 *                kTooManyFunctions  - if the function table is full.
 *                kDuplicateFunction - if a function of the same name is already
 *                                     in the table.
 *                kHashmapFull       - if the function hashmap is full, this
 *                                     should never happen because the hashmap
 *                                     is larger than the function table.
 */
MmResult funtbl_add(const char *name, const char *addr, int *fun_idx);

void funtbl_clear();
void funtbl_dump();

/**
 * @brief  Finds a function by name in the functions table.
 *
 * @param  name          Name of the function to find.
 *                       @warning  This is case-sensitive, but MMB4L should
 *                                 always call it with an UPPER-CASE name.
 *                       @warning  Only the first 32 characters are used.
 * @param[out]  fun_idx  On exit, the index of the matching function,
 *                       or -1 if not found.
 * @return               kOk               - if the function is found.
 *                       kFunctionNotFound - if the function is not in the
 *                                           table.
 */
MmResult funtbl_find(const char *name, int *fun_idx);

size_t funtbl_size();

#endif // #if !defined(MMB4L_FUNTBL_H)
