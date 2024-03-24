/*-*****************************************************************************

MMBasic for Linux (MMB4L)

parse.h

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

#if !defined(MMB4L_PARSE_H)
#define MMB4L_PARSE_H

#include "mmresult.h"

#include <stdbool.h>

bool parse_is_end(const char *p);

/**
 * @brief Does the next text in an element (a basic statement) correspond to an alpha string.
 *
 * Leading whitespace is skipped and the string must be terminated by a non-name character.
 *
 * @param[in] p  Parse from this pointer.
 * @return       If found, then poiter to the next non-space character after the match,
 *               otherwise NULL.
 */
const char *parse_check_string(const char *p, const char *tkn);

bool parse_bool(const char *p);
int parse_colour(const char *p, bool allow_bright);
int parse_file_number(const char *p, bool allow_zero);

/**
 * @brief Does the string match the pattern for a LONGSTRING, i.e. name%() or name().
 */
bool parse_matches_longstring_pattern(const char *s);

/**
 * @brief Parses a name/identifier ignoring any leading space characters.
 *
 * @param[in,out] p     Parse from this pointer.
 *                      On exit points at the character following the name.
 * @param[out]    name  Buffer to store the UPPER-CASE name in, should be
 *                      at least MAXVARLEN + 1 chars.
 * @return              kOk on success.
 *                      kSyntax if the first non-space character is not valid to
 *                      start a name.
 *                      kNameTooLong if the name is more than MAXVARLEN chars.
 */
MmResult parse_name(const char **p, char *name);

/**
 * @brief Transforms star '*' and bang '!' commands in the input buffer
 *        to standard MMBasic commands.
 *
 * @param[in,out] The buffer to transform, usually called with the global 'inpbuf'.
 */
MmResult parse_transform_input_buffer(char *input);

#endif
