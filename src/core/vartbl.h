/*-*****************************************************************************

MMBasic for Linux (MMB4L)

vartbl.h

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

#if !defined(MMB4L_VARTBL_H)
#define MMB4L_VARTBL_H

#include "VarTable.h"
#include "../common/mmresult.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define GLOBAL_VAR     0
#define UNUSED_HASH   -1
#define DELETED_HASH  -2

/**
 * @brief  Has vartbl_init() been called ?
 */
extern bool vartbl_init_called;

/**
 * @brief  Hashmap from a hash of the first 32 characters of the
 *         variable name to the corresponding entry in the \p vartbl.
 *         Empty hash table entries will contain -1.
 */
extern VarHashValue vartbl_hashmap[VARS_HASHMAP_SIZE];

/**
 * @brief  Index of the lowest index "potentially" free slot in the
 *         variables table.
 *
 * When a free slot is required (for a new variable) the code should
 * start looking at this index by checking if:
 *
 *     vartbl[vartbl_free_idx].type == T_NOTYPE
 *
 * and then proceed to increment \p vartbl_free_idx until a free slot
 * is found.
 */
extern int vartbl_free_idx;

/**
 * @brief  Largest index into the variables table.
 */
extern int varcnt;

/**
 * @brief  Initialises variables/structures for the variables table.
 */
void vartbl_init();

/**
 * @brief  Adds a variable to the variables table.
 *
 * Notes:
 *   1. This function makes no effort to detect and report duplicate
 *      or invalid variable names/levels.
 *
 * @param  name   Name for the variable. This is case-sensitive, but
 *                MMB4L should always call it with an UPPER-CASE name.
 *                Only the first 32 characters are used.
 * @param  type   Type of variable, a logical OR of one or more of:
 *                T_NBR, T_STR, T_INT, T_PTR, T_IMPLIED and T_CONST.
 * @param  level  Subroutine depth for a local variable,
 *                or 0 for a global variable.
 * @param  dims   Upper bounds for up to 8 dimensions. If dims[0] == 0
 *                then it is a scalar variable.
 * @param  slen   Maximum length for a string variable, only really relevant
 *                for arrays since scalar strings always require at least
 *                255 + 1 bytes of heap storage.
 * @return        Index of the new variable, or
 *                -1 if we would exceed the maximum number of variables.
 *                -2 if an array dimension is invalid.
 *                -3 if the variable hashmap is full; this should never happen
 *                   in production because the map size is larger than the
 *                   maximum number of variables.
 */
int vartbl_add(
        const char *name,
        uint8_t type,
        uint8_t level,
        DIMTYPE* dims,
        uint8_t slen);

/**
 * @brief  Deletes a variable from the variable table.
 *
 * @param  var_idx  Index of the variable to delete.
 */
void vartbl_delete(int var_idx);

/**
 * @brief  Deletes all variables of a given 'level' or above from the
 *         variable table.
 *
 * @param  level  variables of this level and above will be deleted.
 *                A value of 0 deletes all variables including globals
 *                (which by definition are of level 0).
 */
void vartbl_delete_all(uint8_t level);

/**
 * @brief  Finds a variable by name in the variables table.
 *
 * @param  name   Name of the variable to find. This is case-sensitive, but
 *                MMB4L should always call it with an UPPER-CASE name.
 * @param  level  Subroutine depth to find a local variable,
 *                or 0 to find a global variable.
 * @param[out]  var_idx
 *                On exit the index of the matching variable,
 *                or -1 if not found.
 * @param[out]  global_idx
 *                If non-NULL then on exit the index of a global
 *                variable with the same name, or -1 if not found.
 * @return        kOk if the variable was found at the given level.
 *                kVariableNotFound if the variable was not found at the
 *                given level. However if a local variable (level > 0) is
 *                not found but there is a corresponding global variable
 *                then \p global_idx will be set to the index of the
 *                global variable.
 */
MmResult vartbl_find(
        const char *name, uint8_t level, int *var_idx, int *global_idx);

#endif // #if !defined(MMB4L_VARTBL_H)
