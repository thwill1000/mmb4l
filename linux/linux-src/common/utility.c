#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "utility.h"

int ErrorCheck(void); // file_io.c
void error(char *, ...); // MMBasic.c

char *canonicalize_path(const char *path, char *canonical_path, size_t max_len) {

    //printf("Before: *%s*\n", path);

    // Convert '\' => '/', put result in 'canonical'.
    const char *psrc = path;
    char *pdst = canonical_path;
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
