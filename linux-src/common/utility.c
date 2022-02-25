#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "utility.h"

char *munge_path(const char *original_path, char *new_path, size_t sz) {
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

char *canonicalize_path(const char *path, char *canonical_path, size_t sz) {
    errno = 0;

    if (!munge_path(path, canonical_path, sz)) return NULL;

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
            strcat(tmp_path, "/");
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

bool is_absolute_path(const char *path) {
    return path[0] == '\\' || path[0] == '/';
}

char *get_parent_path(const char *path, char *parent_path, size_t sz) {
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

char *append_path(const char *head, const char *tail, char *result, size_t sz) {
    if (strlen(head) + strlen(tail) + 1 >= sz) {
        errno = ENAMETOOLONG;
        return NULL;
    }
    sprintf(result, "%s/%s", head, tail);
    return result;
}

void unquote(char *str) {
    if (str[0] == '\"' && str[strlen(str) - 1] == '\"') {
        int len = strlen(str);
        for (int i = 0; i < len - 2; ++i) {
            str[i] = str[i + 1];
        }
        str[len - 2] = '\0';
    }
}

char *strupr(char *s) {
    char *p = s;
    while (*p != '\0') {
        *p = toupper(*p);
        p++;
    }
    return s;
}

// TODO: be smart and safe about not overrunning target.
void str_replace(char *target, const char *needle, const char *replacement) {
    char buffer[288] = {0};
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);

    while (1) {
        const char *p = strstr(tmp, needle);

        // walked past last occurrence of needle; copy remaining part
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }

        // copy part before needle
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;

        // copy replacement string
        memcpy(insert_point, replacement, repl_len);
        insert_point += repl_len;

        // adjust pointers, move on
        tmp = p + needle_len;
    }

    // write altered string back to target
    strcpy(target, buffer);
}
