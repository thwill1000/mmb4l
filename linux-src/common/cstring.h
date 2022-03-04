#if !defined(MMMB4L_CSTRING_H)
#define MMMB4L_CSTRING_H

#include <stdbool.h>

/**
 * @brief  Safely concatenates strings.
 *
 * @param dst     the string buffer to append to.
 * @param src     the string buffer to append.
 * @param dst_sz  the size of the 'dst' buffer.
 * @return        0 on success, -1 if the 'dst' buffer was too small to hold
 *                the result without overrun. On failure the 'dst' buffer will
 *                contain the result truncated to avoid an overrun whilst still
 *                allowing for a terminating '\0'.
 */
int cstring_cat(char *dst, const char* src, size_t dst_sz);

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
 */
void cstring_replace(char *target, const char *needle, const char *replacement);

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
