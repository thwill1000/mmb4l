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

#if !defined(FUNTBL_H)
#define FUNTBL_H

#include "../Configuration.h"
#include "../common/hash.h"
#include "../common/mmresult.h"

#include <stdbool.h>

struct s_funtbl {
    char name[MAXVARLEN + 1];  // SUB/FUNCTION name.
    uint32_t index;            // Index into subfun[].
};

/** Indexes into this table are hashes of the SUB/FUNCTION names. */
extern struct s_funtbl funtbl[MAXSUBFUN];

/** Table of pointers to SUBroutines and FUNCTIONs in the program memory. */
extern const char *subfun[MAXSUBFUN];

void funtbl_clear();
void funtbl_dump();

/**
 * @brief  Finds a FUNCTION/SUBroutine by name in the function table.
 *
 * @param[in]   p        pointer to start of function name.
 * @param[out]  fun_idx  on exit the index of the function subfun[],
 *                       or -1 on failure.
 * @return               kOk               - on success.
 *                       kNameTooLong      - if function name is too long.
 *                       kFunctionNotFound - if function not in table.
 */
MmResult funtbl_find(const char *p, int *fun_idx);

void funtbl_prepare(bool abort_on_error);
size_t funtbl_size();

/**
 * Calculates hash for a function table entry.
 *
 * @param[in]  p     function name is read from here.
 * @param[out] name  on exit the function name in upper-case,
 *                   a buffer of at least MAXVARLEN + 1 chars must be provided.
 * @param[out] hash  on exit the calculated hash value.
 * @return           0 on success, or
 *                   -1 if the name was too long in which case the value
 *                   copied to \p pname will be truncated.
 */
int funtbl_hash(const char *p, char *name, HashValue* hash);

#endif // #if !defined(FUNTBL_H)
