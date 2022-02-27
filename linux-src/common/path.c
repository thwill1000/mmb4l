#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cstring.h"
#include "path.h"
#include "utility.h"

bool path_exists(const char *path) {
    struct stat st;
    errno = 0;
    return stat(path, &st) == 0;
}

bool path_is_empty(const char *path) {
    struct stat st;
    errno = 0;
    stat(path, &st);
    return st.st_size == 0;
}

bool path_is_regular(const char *path) {
    struct stat st;
    errno = 0;
    return (stat(path, &st) == 0) && S_ISREG(st.st_mode) ? true : false;
}

bool path_has_suffix(
        const char *path, const char *suffix, bool case_insensitive) {
    int start = strlen(path) - strlen(suffix);
    if (start < 0) return 0;
    for (int i = 0; i < strlen(suffix); ++i) {
        if (case_insensitive) {
            if (toupper(path[i + start]) != toupper(suffix[i])) return false;
        } else {
            if (path[i + start] != suffix[i]) return false;
        }
    }
    return true;
}

char *path_munge(const char *original_path, char *new_path, size_t sz) {
    errno = 0;
    const char *psrc = original_path;

    // HACK! ignore any leading drive letter and colon in the 'original_path', e.g. "A:".
    size_t len = strlen(psrc);
    if (len >= 2 && isalpha(psrc[0]) && psrc[1] == ':') {
        psrc += 2;
        len -= 2;
    }

    if (sz <= len || sz <= 2) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    // Handle the case where 'original_path' was just a drive letter and colon.
    if (!*psrc) {
        new_path[0] = '/';
        new_path[1] = '\0';
        return new_path;
    }

    // Does the path begin with '~' ?
    char *pdst = new_path;
    if (*psrc == '~') {
        psrc++;

        const char *home = getenv("HOME");
        if (!home) return NULL; // Probably never happens.

        if (!*psrc) {
            // The path is just "~".
            strcpy(new_path, home);
            return new_path;
        } else if (*psrc == '\\' || *psrc == '/') {
            // The path begins "~/".
            strcpy(new_path, home);
            pdst += strlen(home);
        } else {
            // No special treatment for '~' in this case.
            *pdst++ = '~';
        }
    }

    // Copy from 'original_path' to 'new_path' converting '\' => '/'.
    for (;;) {
        *pdst = (*psrc == '\\') ? '/' : *psrc;
        if (*psrc == '\0') break;
        psrc++;
        pdst++;
    }

    return new_path;
}

char *path_get_canonical(const char *path, char *canonical_path, size_t sz) {
    errno = 0;

    if (!path_munge(path, canonical_path, sz)) return NULL;

    // Use realpath() to convert to canonical absolute form putting the result in 'tmp_path'.
    char tmp_path[PATH_MAX];
    if (!realpath(canonical_path, tmp_path)) {
        if (errno != ENOENT) return NULL;

        // Path didn't exist, try again with immediate parent.
        char *last = strrchr(canonical_path, '/');
        if (last) {
            *last = '\0';
            if (!realpath(canonical_path, tmp_path)) return NULL;
            *last = '/';
        } else {
            if (!realpath(".", tmp_path)) return NULL;
            cstring_cat(tmp_path, "/", PATH_MAX);
            last = canonical_path;
        }

        // Immediate parent existed, copy the final path element into the result.
        char *to = tmp_path + strlen(tmp_path);
        while (*last) *to++ = *last++;
        *to = '\0';
    }

    if (strlen(tmp_path) >= sz) {
        errno = ENAMETOOLONG;
        return NULL;
    } else {
        strcpy(canonical_path, tmp_path);
        errno = 0;
        return canonical_path;
    }
}

bool path_is_absolute(const char *path) {
    return path[0] == '\\' || path[0] == '/';
}

char *path_get_parent(const char *path, char *parent_path, size_t sz) {
    errno = 0;
    char *p = (char *) path + strlen(path) - 1;
    while ((p > path) && (*p == '\\' || *p == '/')) p--;
    while ((p > path) && (*p != '\\' && *p != '/')) p--;

    if (p <= path) {
        errno = ENOENT;
        return NULL;
    }

    if (p - path >= sz) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    memcpy(parent_path, path, p - path);
    parent_path[p - path] = '\0';

    return parent_path;
}

char *path_append(const char *head, const char *tail, char *result, size_t sz) {
    if (strlen(head) + strlen(tail) + 1 >= sz) {
        errno = ENAMETOOLONG;
        return NULL;
    }
    sprintf(result, "%s/%s", head, tail);
    return result;
}

const char *path_get_extension(const char *path) {
    char *p = strrchr(path, '.');
    return p ? p : path + strlen(path);
}
