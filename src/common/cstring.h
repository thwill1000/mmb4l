/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cstring.h

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#if !defined(MMMB4L_CSTRING_H)
#define MMMB4L_CSTRING_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief  Safely concatenates strings.
 *
 * @param dst     the string buffer to append to.
 * @param src     the string buffer to append.
 * @param dst_sz  the size of the \p dst buffer.
 * @return        0 on success, -1 if the 'dst' buffer was too small to hold
 *                the result without overrun. On failure the \p dst buffer will
 *                contain the result truncated to avoid an overrun whilst still
 *                allowing for a terminating '\0'.
 */
int cstring_cat(char *dst, const char *src, size_t dst_sz);

/**
 * @brief  Safely concatenates an integer with a string.
 *
 * @param dst     the string buffer to append to.
 * @param src     the integer to append.
 * @param dst_sz  the size of the \p dst buffer.
 * @return        0 on success, -1 if the 'dst' buffer was too small to hold
 *                the result without overrun. On failure the \p dst buffer will
 *                contain the result truncated to avoid an overrun whilst still
 *                allowing for a terminating '\0'.
 */
int cstring_cat_int64(char *dst, int64_t src, size_t dst_sz);

/**
 * @brief  Safely copies strings.
 *
 * @param dst     the string buffer to write to.
 * @param src     the string buffer to read from.
 * @param dst_sz  the size of the \p dst buffer.
 * @return        0 on success, -1 if the \p dst buffer was too small to hold
 *                the result without overrun. On failure the \p dst buffer will
 *                contain the \p src truncated to avoid an overrun whilst still
 *                allowing for a terminating '\0'.
*/
int cstring_cpy(char *dst, const char *src, size_t dst_sz);

/**
 * @brief Performs inplace double-quoting of a C-string.
 *
 * @param [in,out] s  The string to quote.
 * @param [in]     n  The size of the buffer allocated to @p s.
 * @return            0 on success, -1 if the string was NULL or the buffer too small.
 */
int cstring_enquote(char *s, size_t n);

/**
 * @brief Does a C-string have leading and trailing double-quotes.
 *
 * @param[in] s   The string to test.
 * @return true   If @p s has both leading and trailing double-quotes.
 * @return false  If @p s does not have both leading and trailing double-quotes.
 */
bool cstring_isquoted(const char *s);

#define cstring_quote(s, n)  cstring_enquote(s, n)

/**
 * @brief Replaces substrings in a C string.
 *
 * @param [in,out] target       string to apply replacements to.
 * @param [in]     needle       replace this ...
 * @param [in]     replacement  ... with this
 * @return                      0 on success, -1 on failure.
 */
int cstring_replace(char *target, const char *needle, const char *replacement);

/**
 * @brief Performs inplace conversion of a C-string to lower-case.
 *
 * @param [in,out] s  The string to convert.
 * @return            The value of @p s.
 */
char *cstring_tolower(char *s);

/**
 * @brief Performs inplace conversion of a C-string to upper-case.
 *
 * @param [in,out] s  The string to convert.
 * @return            The value of @p s.
 */
char *cstring_toupper(char *s);

/**
 * @brief Performs inplace trimming of leading and trailing whitespace
 *        (as determined by @code{isspace()}) from a C-string.
 *
 * @param [in,out] s  The string to trim.
 * @return            The value of @p s.
 */
char *cstring_trim(char *s);

/**
 * @brief Performs inplace removal of a single leading and trailing double-quote
 *        from a C-string.
 *
 * If both leading and trailing double-quotes are not present then does nothing.
 *
 * @param [in,out] s  The string to unquote.
 * @return            The value of @p s.
 */
char *cstring_unquote(char *s);

#endif
