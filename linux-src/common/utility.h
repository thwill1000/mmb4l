#if !defined(UTILITY_H)
#define UTILITY_H

#include <stdbool.h>
#include <stdlib.h>

#define max(a,b) \
    ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })

#define min(a,b) \
    ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a < _b ? _a : _b; })

#define FAILED(x) (x != 0)
#define SUCCEEDED(x) (x == 0)

/** If 'str' has leading and trailing double quotes then strips them off. */
void unquote(char *str);

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

/** Converts C-string to upper-case inplace. */
char *strupr(char *s);

/**
 * @brief Replaces substrings in a C string.
 *
 * @param [in,out] target       string to apply replacements to.
 * @param [in]     needle       replace this ...
 * @param [in]     replacement  ... with this
 */
void str_replace(char *target, const char *needle, const char *replacement);

#endif
