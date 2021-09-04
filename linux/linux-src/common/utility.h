#if !defined(UTILITY_H)
#define UTILITY_H

#include <stdlib.h>

#define ERROR_UNIMPLEMENTED(s)  error("Unimplemented: " s)

/**
 * Returns an absolute/canonical path, replacing any '\' with '/'.
 *
 * @param  path            the original path to be converted.
 * @param  canonical_path  the canonical path is returned in this buffer.
 * @param  max_len         the length of the 'canonical_path' buffer.
 * @return                 the value of 'canonical_path'.
 */
char *canonicalize_path(const char *path, char *canonical_path, size_t max_len);

/**
 * Is the path absolute?
 *
 * @param  path  the path to check.
 * @return       1 if the path is absolute, otherwise 0.
 */
int is_absolute_path(const char *path);

/**
 * Gets the parent of the given path.
 *
 * @param  path         the original path to get the parent of.
 * @param  parent_path  the parent path.
 * @param  max_len      the length of the 'parent_path' buffer.
 * @return              the value of 'parent_path'.
 */
char *get_parent_path(const char *path, char *parent_path, size_t max_len);

/**
 * Appends one path to another.
 *
 * @param  head     the path being appended to.
 * @param  tail     the path being appended.
 * @param  result   the result buffer.
 * @param  max_len  the length of the result buffer.
 * @return          the value of 'new_path'.
 */
char *append_path(const char *head, const char *tail, char *result, size_t max_len);

/**
 * Transforms path by:
 *  - removing any DOS style drive specified, e.g. A:
 *  - replacing any '\' with '/'
 *
 * @param  original_path  the path to be transformed.
 * @param  new_path       buffer to holdthe transformed path.
 * @param  new_path_sz    size of the buffer.
 * @return                'new_path' on success,
 *                        NULL on error, see 'errno' for details.
 */
char *munge_path(const char *original_path, char *new_path, size_t new_path_sz);

#endif
