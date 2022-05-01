#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cstring.h"
#include "path.h"
#include "utility.h"

bool path_exists(const char *path) {
    struct stat st;
    errno = 0;
    return stat(path, &st) == 0;
}

bool path_is_directory(const char *path) {
    struct stat st;
    errno = 0;
    return (stat(path, &st) == 0) && S_ISDIR(st.st_mode) ? true : false;
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

typedef enum {
    kPathStateStart,
    kPathStateStartDot,
    kPathStateStartDotDot,
    kPathStateDefault,
    kPathStateSlash,
    kPathStateSlashDot,
    kPathStateSlashDotDot,
} PathState;

static char *path_unwind(char *new_path, char *pdst) {
    if (pdst == new_path
        || ((pdst == new_path + 2) && memcmp(pdst - 2, "..", 2) == 0)
        || (memcmp(pdst - 3, "/..", 3) == 0)) {
        // Can't unwind any further.
        return pdst;
    } else {
        char *p = pdst;
        while (--p >= new_path) {
            if (*p == '/' || p == new_path) {
                return p;
            }
        }
        assert(false);
    }
}

char *path_munge(const char *original_path, char *new_path, size_t sz) {
    errno = 0;
    const char *psrc = original_path;
    bool absolute = original_path[0] == '\\' || original_path[0] == '/';

    // HACK! ignore any leading drive letter and colon in the 'original_path', e.g. "A:".
    size_t len = strlen(psrc);
    if (len >= 2 && isalpha(psrc[0]) && psrc[1] == ':') {
        psrc += 2;
        len -= 2;
        absolute = true;
    }

    if (sz <= len || sz <= 2) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    // Does the path begin with '~' ?
    char *pdst = new_path;
    *pdst = '\0';
    if (*psrc == '~') {
        psrc++;
        const char *home = getenv("HOME");
        if (!home) return NULL; // Probably never happens.
        if (!*psrc || *psrc == '\\' || *psrc == '/') {
            // The path is just '~' or begins "~/".
            strcpy(new_path, home);
            pdst += strlen(home);
        } else {
            // No special treatment for '~' in this case.
            *pdst++ = '~';
        }
        absolute = new_path[0] == '\\' || new_path[0] == '/';
    }

    PathState state = kPathStateStart;
    do {
        switch (*psrc) {

            case '\0':
                switch (state) {
                    case kPathStateStartDotDot:
                        *pdst++ = '.';
                        *pdst++ = '.';
                        break;
                    case kPathStateSlashDotDot: {
                        char *p = path_unwind(new_path, pdst);
                        if (p == pdst) {
                            *pdst++ = '/';
                            *pdst++ = '.';
                            *pdst++ = '.';
                        } else {
                            pdst = p;
                        }
                        break;
                    }
                    default:
                        break;
                }

                // Empty absolute path is '/'
                if (absolute && pdst == new_path) *pdst++ = '/';

                *pdst++ = *psrc; // Copies the '/0'
                break;

            case '.':
                switch (state) {
                    case kPathStateStart:
                        state = kPathStateStartDot;
                        break;
                    case kPathStateStartDot:
                        state = kPathStateStartDotDot;
                        break;
                    case kPathStateSlash:
                        state = kPathStateSlashDot;
                        break;
                    case kPathStateSlashDot:
                        state = kPathStateSlashDotDot;
                        break;
                    default:
                        *pdst++ = '.';
                        break;
                }
                break;

            case '\\':
            case '/':
                switch (state) {
                    case kPathStateStartDot:
                        // Ignore ./
                        state = kPathStateDefault;
                        break;
                    case kPathStateStartDotDot:
                        *pdst++ = '.';
                        *pdst++ = '.';
                        state = kPathStateSlash;
                        break;
                    case kPathStateSlash:
                        // Ignore repeated slashes.
                        break;
                    case kPathStateSlashDot:
                        // Treat /./ as /
                        state = kPathStateSlash;
                        break;
                    case kPathStateSlashDotDot: {
                        char *p = path_unwind(new_path, pdst);
                        if (p == pdst) {
                            *pdst++ = '/';
                            *pdst++ = '.';
                            *pdst++ = '.';
                            state = kPathStateSlash;
                        } else {
                            state = *p == '/' ? kPathStateSlash : kPathStateDefault;
                            pdst = p;
                        }
                        break;
                    }
                    default:
                        state = kPathStateSlash;
                        break;
                }
                break;

            default:
                switch (state) {
                    case kPathStateStartDot:
                        *pdst++ = '.';
                        break;
                    case kPathStateStartDotDot:
                        *pdst++ = '.';
                        *pdst++ = '.';
                        break;
                    case kPathStateSlash:
                        *pdst++ = '/';
                        break;
                    case kPathStateSlashDot:
                        *pdst++ = '/';
                        *pdst++ = '.';
                        break;
                    case kPathStateSlashDotDot:
                        *pdst++ = '/';
                        *pdst++ = '.';
                        *pdst++ = '.';
                        break;
                    default:
                        break;
                }
                state = kPathStateDefault;
                *pdst++ = *psrc;
                break;

        } // select

    } while (*psrc++);

    return new_path;
}

char *path_get_canonical(const char *path, char *canonical_path, size_t sz) {
    errno = 0;
    char tmp_path[PATH_MAX];
    if (!path_munge(path, tmp_path, PATH_MAX)) return NULL;

    if (tmp_path[0] == '/') {
        if (strlen(tmp_path) >= sz) {
            errno = ENAMETOOLONG;
            return NULL;
        }
        strcpy(canonical_path, tmp_path);
        errno = 0;
        return canonical_path;
    }

    errno = 0;
    if (!getcwd(canonical_path, sz)) {
        assert(errno != 0);
        return NULL;
    }

    if (tmp_path[0] == '\0') return canonical_path;

    if (FAILED(cstring_cat(canonical_path, "/", sz))) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    if (FAILED(cstring_cat(canonical_path, tmp_path, sz))) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    return canonical_path;
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

static MmResult path_mkdir_internal(const char *path) {
    if (path[0] == '\0') return kOk;

    if (path_exists(path)) {
        return path_is_directory(path) ? kOk : kNotADirectory;
    }

    errno = 0;
    if (FAILED(mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))) return errno;

    return kOk;
}

MmResult path_mkdir(const char *path) {
    char tmp_path[PATH_MAX];
    if (!path_munge(path, tmp_path, PATH_MAX)) return errno;

    // Make intermediate elements of the path.
    char *end = strchr(tmp_path, '/');
    MmResult result = kOk;
    while (end) {
        *end = '\0';
        result = path_mkdir_internal(tmp_path);
        if (FAILED(result)) return result;
        *end = '/';
        end = strchr(end + 1, '/');
    }

    // Make final element of the path.
    result = path_mkdir_internal(tmp_path);
    return result == kNotADirectory ? kFileExists : result;
}
