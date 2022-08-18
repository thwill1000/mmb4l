/*-*****************************************************************************

MMBasic for Linux (MMB4L)

variables.h

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

#if !defined(MMB4L_VARIABLES_H)
#define MMB4L_VARIABLES_H

#include <stddef.h>

/**
 * @brief  Adds a variable to the variables table.
 *
 * For the moment this is only for use by unit-tests.
 *
 * @param  name  Name for the variable. This is case-sensitive, but
 *               MMB4L should always call it with an UPPER-CASE name.
 * @param  size  Number of bytes of heap memory that should be allocated
 *               for this variable. If == 0 then no heap memory is allocatted.
 * @return       Index of the new variable.
 */
int variables_add(const char *name, size_t size);

/**
 * @brief  Deletes a variable from the variable table.
 *
 * @param  var_idx  Index of the variable to delete.
 */
void variables_delete(int var_idx);

/**
 * @brief  Finds a variable by name in the variables table.
 *
 * @param  name  Name of the variable to find. This is case-sensitive, but
 *               MMB4L should always call it with an UPPER-CASE name.
 * @return       Index of the named variable in the variables table,
 *               or -1 if not found.
 */
int variables_find(const char *name);

#endif // #if !defined(MMB4L_VARIABLES_H)
