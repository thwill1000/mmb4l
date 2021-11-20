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

/**
 * Returns an absolute/canonical path, replacing any '\' with '/'.
 *
 * @param  path            original path to be converted.
 * @param  canonical_path  canonical path is returned in this buffer.
 * @param  sz              size of the 'canonical_path' buffer.
 * @return                 the value of 'canonical_path' on success,
 *                         otherwise sets 'errno' and returns NULL.
 */
char *canonicalize_path(const char *path, char *canonical_path, size_t sz);

/**
 * Is the path absolute?
 *
 * @param  path  path to check.
 * @return       true if the path is absolute, otherwise false.
 */
bool is_absolute_path(const char *path);

/**
 * Gets the parent of the given path.
 *
 * @param  path         original path to get the parent of.
 * @param  parent_path  parent path is returned in this buffer.
 * @param  sz           size of the 'parent_path' buffer.
 * @return              the value of 'parent_path' on success,
 *                      otherwise sets 'errno' and returns NULL.
 */
char *get_parent_path(const char *path, char *parent_path, size_t sz);

/**
 * Appends one path to another.
 *
 * @param  head    path being appended to.
 * @param  tail    path being appended.
 * @param  result  result is returned in this buffer.
 * @param  sz      size of the 'result' buffer.
 * @return         the value of 'result' on success,
 *                 otherwise sets 'errno' and returns NULL.
 */
char *append_path(const char *head, const char *tail, char *result, size_t sz);

/**
 * Transforms path by:
 *  - removing any DOS style drive specified, e.g. A:
 *  - replacing any '\' with '/'
 *
 * @param  original_path  path to be transformed.
 * @param  new_path       transformed path is returned in this buffer.
 * @param  sz             size of the 'new_path' buffer.
 * @return                the value of 'new_path' on success,
 *                        otherwise sets 'errno' and returns NULL.
 */
char *munge_path(const char *original_path, char *new_path, size_t sz);

/** If 'str' has leading and trailing double quotes then strips them off. */
void unquote(char *str);

/** Converts C-string to upper-case inplace. */
char *strupr(char *s);

#endif
