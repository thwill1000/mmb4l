/*-*****************************************************************************

MMBasic for Linux (MMB4L)

path.c

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <linux/limits.h> // PATH_MAX
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
    for (size_t i = 0; i < strlen(suffix); ++i) {
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

/**
 * @brief Searches through \p new_path backwards from \p pdst looking for a '/'
 *        or the start of \p new_path.
 *
 * @return  pointer to the first '/' encountered or to \p new_path if there
 *          were none.
 */
/*static*/ char *path_unwind(char *new_path, char *pdst) {
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
    }

    assert(false);
    return NULL;
}

MmResult path_munge(const char *original_path, char *new_path, size_t sz) {
    const char *psrc = original_path;
    bool absolute = original_path[0] == '\\' || original_path[0] == '/';

    // HACK! ignore any leading drive letter and colon in the 'original_path', e.g. "A:".
    size_t len = strlen(psrc);
    if (len >= 2 && isalpha(psrc[0]) && psrc[1] == ':') {
        psrc += 2;
        len -= 2;
        absolute = true;
    }

    // TODO: better checking for buffer overrun.
    if (sz <= len || sz <= 2) return kFilenameTooLong;

    char *pdst = new_path;
    *pdst = '\0';
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
                            if (!absolute) {
                                *pdst++ = '.';
                                *pdst++ = '.';
                            }
                        } else {
                            pdst = p;
                        }
                        break;
                    }
                    default:
                        break;
                }

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
                            if (!absolute) {
                                *pdst++ = '.';
                                *pdst++ = '.';
                            }
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

            case '~':
                if (state == kPathStateStart) {
                    psrc++;
                    if (*psrc == '\0' || *psrc == '\\' || *psrc == '/' ) {
                        errno = 0;
                        const char *home = getenv("HOME");
                        if (!home) return errno; // Probably never happens.
                        while (*home != '\0') {
                            *pdst++ = *home++;
                        }
                    } else {
                        *pdst++ = '~';
                    }
                    psrc--;
                    state = kPathStateDefault;
                    break;
                } else {
                    CASE_FALLTHROUGH;
                }

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

    if (!*new_path) {
        // Empty absolute path is '/' whereas empty relative path is '.'
        new_path[0] = absolute ? '/' : '.';
        new_path[1] = '\0';
    }

    return kOk;
}

/**
 * @brief Resolves symbolic links in \p src writing the resolved path into \p dst.
 *
 * @param src  source path.
 * @param dst  buffer for destination path.
 * @param sz   size of buffer.
 * @return     kOk on success.
 */
static MmResult path_resolve_symlinks(const char *src, char *dst, size_t sz) {

    // TODO: Handle 'dst' buffer overrun.

    size_t count = 0;
    const char *psrc = src;
    char *pdst = dst;
    char buf[PATH_MAX];
    MmResult result;
    do {
        if (*psrc == '/' || *psrc == '\0') {
try_again:
            *pdst = '\0';
            ssize_t num = readlink(dst, buf, PATH_MAX);
            if (num != -1) {
                // On success replace 'dst' with target of link.
                // On failure leave contents of 'dst' intact.

                if (buf[0] == '/') {
                    // Handle absolute symbolic link.
                    pdst = dst;
                } else {
                    // Handle relative symbolic link.
                    memcpy(pdst, "/../", 4);
                    pdst += 4;
                }

                count++;
                if (count >= 32) return kTooManySymbolicLinks;  // TODO: test this case

                memcpy(pdst, buf, num);
                pdst += num;
                *pdst = '\0';
                result = path_munge(dst, buf, PATH_MAX);
                if (FAILED(result)) return result;
                strcpy(dst, buf);
                pdst = dst + strlen(dst);

                goto try_again;  // Handle symbolic links to symbolic links.
            }

            // Handle edge case of a symbolic link to root.
            if (*psrc == '/' && *(pdst - 1) == '/') pdst--;
        }
        *pdst++ = *psrc;
    } while (*psrc++ != '\0');
    return kOk;
}

MmResult path_get_canonical(const char *path, char *canonical_path, size_t sz) {
    bool absolute = (path[0] == '\\' || path[0] == '/');

    const char *prefix = "";
    if ((path[0] == '~') && (path[1] == '\0' || path[1] == '\\' || path[1] == '/')) {

        // Replace '~' prefix with the user's HOME directory.
        errno = 0;
        prefix = getenv("HOME");
        if (!prefix) return errno; // Probably never happens.
        absolute = (prefix[0] == '\\' || prefix[0] == '/');
        path++; // Skip the '~'.

    } else if (isalpha(path[0]) && path[1] == ':') {

        // Replace DOS drive prefix with root dir;
        // Any repeated '/' will be dealt with by the later call to path_munge().
        prefix = "/";
        absolute = true;
        path += 2; // Skip the drive prefix.

    }

    char tmp_path[PATH_MAX] = { 0 };

    // If the 'path' is not absolute then copy the current working directory
    // into 'tmp_path'.
    if (!absolute) {
        errno = 0;
        if (!getcwd(tmp_path, PATH_MAX)) return errno;
        if (FAILED(cstring_cat(tmp_path, "/", PATH_MAX))) return kFilenameTooLong;
    }

    // Append 'prefix', which may be empty.
    if (FAILED(cstring_cat(tmp_path, prefix, PATH_MAX))) return kFilenameTooLong;

    // Append 'path'.
    if (FAILED(cstring_cat(tmp_path, path, PATH_MAX))) return kFilenameTooLong;

    // Munge 'tmp_path' into 'canonical_path' to deal with any
    // repeated slashes, slash-dots, slash-dot-dots, or back-slashes.
    MmResult result = path_munge(tmp_path, canonical_path, sz);
    if (FAILED(result)) return result;

    // Resolve symbolic links into 'tmp_path'.
    result = path_resolve_symlinks(canonical_path, tmp_path, PATH_MAX);
    if (FAILED(result)) return result;
    if (strlen(tmp_path) >= sz) return kFilenameTooLong;
    strcpy(canonical_path, tmp_path);

    return kOk;
}

bool path_is_absolute(const char *path) {
    return path[0] == '\\' || path[0] == '/';
}

MmResult path_get_parent(const char *path, char *parent_path, size_t sz) {
    bool absolute = path_is_absolute(path);
    MmResult result = path_munge(path, parent_path, PATH_MAX);
    if (FAILED(result)) return result;
    char *p = strrchr(parent_path, '/');
    if (!p) return kFileNotFound;
    *p = '\0';
    if (parent_path[0] == '\0' && absolute) {
        parent_path[0] = '/';
        parent_path[1] = '\0';
    }
    return kOk;
}

MmResult path_append(const char *head, const char *tail, char *result, size_t sz) {
    result[0] = '\0';
    if (FAILED(cstring_cat(result, head, sz))
            || FAILED(cstring_cat(result, "/", sz))
            || FAILED(cstring_cat(result, tail, sz))) return kFilenameTooLong;
    return kOk;
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
    MmResult result = path_munge(path, tmp_path, PATH_MAX);
    if (FAILED(result)) return result;

    // Make intermediate elements of the path.
    char *end = strchr(tmp_path, '/');
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

MmResult path_complete(const char *path, char *out, size_t sz) {
    // printf("path_complete: #%s#\n", path);
    char dir_path[PATH_MAX];
    MmResult result = path_munge(path, dir_path, PATH_MAX);
    if (FAILED(result)) return result;

    out[0] = '\0';
    if (dir_path[0] == '\0' || path_exists(dir_path)) return kOk;

    // Rewind to path-separator '/' or beginning of path,
    // that gives us the 'dir_path' to search
    // and the 'filename' prefix to match.
    char filename[NAME_MAX + 1];
    char *p = dir_path + strlen(dir_path);
    while (*p != '/' && p > dir_path) p--;
    if (*p == '/') {
        strcpy(filename, p + 1);
        if (p == dir_path) {
            strcpy(dir_path, "/");
        } else {
            *p = '\0';
        }
    } else {
        strcpy(filename, p);
        strcpy(dir_path, "./");
    }

    // printf("#%s#%s#\n", dir_path, filename);
    // printf("%d\n", path_exists(dir_path));
    // printf("%d\n", path_is_directory(dir_path));

    // If 'dir_path' doesn not exist or is not a directory then exit.
    if (!path_exists(dir_path)) return kFileNotFound;
    if (!path_is_directory(dir_path)) return kNotADirectory;

    // Open 'dir_path'.
    errno = 0;
    DIR *fd = opendir(dir_path);
    if (!fd) return errno;

    // Loop through files in 'dir_path' to identify a common completion 'out'.
    struct dirent* entry;
    size_t filename_len = strlen(filename);
    while ((entry = readdir(fd))) {
        if (strcmp(entry->d_name, ".") == 0
                 || strcmp(entry->d_name, "..") == 0) continue;
        p = strstr(entry->d_name, filename);
        if (p != entry->d_name) continue;
        // printf(" - %s\n", entry->d_name);
        if (out[0] == '\0') {
            cstring_cat(out, entry->d_name + filename_len, sz);
        } else {
            p = out;
            char *p2 = entry->d_name + filename_len;
            while (*p++ == *p2++);
            *--p = '\0';
        }
    }

    return kOk;
}
