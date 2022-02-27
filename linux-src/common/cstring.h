#if !defined(MMMB4L_CSTRING_H)
#define MMMB4L_CSTRING_H

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
 * @brief Replaces substrings in a C string.
 *
 * @param [in,out] target       string to apply replacements to.
 * @param [in]     needle       replace this ...
 * @param [in]     replacement  ... with this
 */
void cstring_replace(char *target, const char *needle, const char *replacement);

/** Converts C-string to upper-case inplace. */
char *cstring_toupper(char *s);

/** If 'str' has leading and trailing double quotes then strips them off. */
void cstring_unquote(char *str);

#endif
