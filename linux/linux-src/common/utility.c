#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "utility.h"

int ErrorCheck(void); // file_io.c
void error(char *, ...); // MMBasic.c

char *munge_path(const char *original_path, char *new_path, size_t new_path_sz) {
    const char *psrc = original_path;

    // HACK! ignore any leading drive letter and colon in the 'original_path', e.g. "A:".
    size_t len = strlen(psrc);
    if (len >= 2 && isalpha(psrc[0]) && psrc[1] == ':') {
        psrc += 2;
        len -= 2;
    }

    if (new_path_sz <= len || new_path_sz <= 2) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    // Handle the case where 'original_path' was just a drive letter and colon.
    if (!*psrc) {
        new_path[0] = '/';
        new_path[1] = '\0';
        return new_path;
    }

    // Copy from 'original_path' to 'new_path' converting '\' => '/'.
    char *pdst = new_path;
    for (;;) {
        *pdst = (*psrc == '\\') ? '/' : *psrc;
        if (*psrc == '\0') break;
        psrc++;
        pdst++;
    }

    return new_path;
}

char *canonicalize_path(const char *path, char *canonical_path, size_t max_len) {

    //printf("Before: *%s*\n", path);

    const char *psrc = path;
    char *pdst = canonical_path;

    // HACK! strip any leading drive letter and colon, e.g. "A:".
    if (strlen(path) >= 2 && isalpha(path[0]) && path[1] == ':') {
        psrc += 2;
    }

    // Convert '\' => '/', put result in 'canonical'.
    for (;;) {
        *pdst = (*psrc == '\\') ? '/' : *psrc;
        if (*psrc == 0) break;
        psrc++;
        pdst++;
    }

    //printf("*%s*\n", canonical_path);

    // Convert to absolute/canonical form, put result in 'tmp'.
    //errno = 0;
    char tmp[PATH_MAX + 1];
    if (!realpath(canonical_path, tmp)) {
        //printf("%s\n", canonical_path);
        ErrorCheck();
    }

    if (strlen(tmp) > max_len) {
        error("Path too long");
    }

    // Copy result into 'canonical'.
    strcpy(canonical_path, tmp);

    //printf("After: %s\n", canonical_path);

    return canonical_path;
}

int is_absolute_path(const char *path) {
    return path[0] == '\\' || path[0] == '/';
}

char *get_parent_path(const char *path, char *parent_path, size_t max_len) {
    char *p = (char *) path + strlen(path) - 1;
    while ((p > path) && (*p == '\\' || *p == '/')) p--;
    while ((p > path) && (*p != '\\' && *p != '/')) p--;

    if (p <= path) error("No parent path");
    if (p - path > max_len) error("Path too long");

    memcpy(parent_path, path, p - path);
    parent_path[p - path] = '\0';

    return parent_path;
}

char *append_path(const char *head, const char *tail, char *result, size_t max_len) {
    if (strlen(head) + strlen(tail) + 1 > max_len) error("Path too long");
    sprintf(result, "%s/%s", head, tail);
    return result;
}
