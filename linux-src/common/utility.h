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
