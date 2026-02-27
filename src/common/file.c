/*-*****************************************************************************

MMBasic for Linux (MMB4L)

file.c

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed on the console at startup (additional copyright messages may
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
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "cstring.h"
#include "error.h"
#include "file.h"
#include "file_private.h"
#include "logger.h"
#include "mmb4l.h"
#include "path.h"
#include "utility.h"

// We don't use the 0'th entry, but it makes things simpler since MMBasic
// indexes file numbers from 1.
FileEntry file_table[MAXOPENFILES + 1] = { 0 };

// Forward declaration of real function implementations
static MmResult file_get_config_dir_impl(char *buf, size_t size);

// Pointers to functions we want to override in unit-tests
MmResult (*file_get_config_dir)(char *, size_t) = file_get_config_dir_impl;

MmResult file_basename(const char *path, char *buf, size_t buf_sz) {
    CHECK_PARAM(path != NULL);
    CHECK_PARAM(buf != NULL);

    // Empty string -> "."
    if (*path == '\0') {
        if (buf_sz < 2) return kFilenameTooLong;
        strcpy(buf, ".");
        return kOk;
    }

    // Find end of string, then strip trailing separators
    const char *end = path + strlen(path) - 1;
    while (end > path && file_is_separator(*end)) end--;

    // Root-only path e.g. "/" or "\\"
    if (end == path && file_is_separator(*end)) {
        if (buf_sz < 2) return kFilenameTooLong;
        buf[0] = *end;
        buf[1] = '\0';
        return kOk;
    }

    // Find the start of the last component
    const char *start = end;
    while (start > path && !file_is_separator(*(start - 1))) start--;

    size_t len = end - start + 1;
    if (len >= buf_sz) return kFilenameTooLong;

    memcpy(buf, start, len);
    buf[len] = '\0';
    return kOk;
}

MmResult file_close(int fnbr) {
    errno = 0;
    int result = fclose(file_table[fnbr].file_ptr);
    file_table[fnbr].type = fet_closed;
    file_table[fnbr].file_ptr = NULL;
    RETURN_RESULT(SUCCEEDED(result) ? kOk : errno);
}

bool file_compare_path(const char *path1, const char *path2) {
    // If the pointers are identical then the paths are the same
    if (path1 == path2) return true;

    // If the paths are different lengths then the paths are not the same
    const size_t len1 = strlen(path1);
    const size_t len2 = strlen(path2);
    if (len1 != len2) return false;

    // Iterate through the paths comparing character by character,
    // treat backslashes and forward slashes as equivalent,
    // and on Windows compare case-insensitively.
    for (size_t i = 0; i < len1; i++) {
        const char c1 = path1[i];
        const char c2 = path2[i];

        if (file_is_separator(c1) && file_is_separator(c2)) {
            continue; // Treat separators as equivalent
        }
#if defined(_WIN32)
        if (tolower(c1) != tolower(c2)) {
            return false; // Case-insensitive comparison on Windows
        }
#else
        if (c1 != c2) {
            return false; // Case-sensitive comparison on non-Windows
        }
#endif
    }

    return true;
}

// TODO: Reconcile with path_get_parent()
MmResult file_dirname(const char *path, char *buf, size_t buf_sz) {
    CHECK_PARAM(path != NULL);
    CHECK_PARAM(buf != NULL);

    // Empty string -> "."
    if (*path == '\0') {
        if (buf_sz < 2) return kFilenameTooLong;
        strcpy(buf, ".");
        return kOk;
    }

    // Find end of string, then strip trailing separators
    const char *end = path + strlen(path) - 1;
    while (end > path && file_is_separator(*end)) end--;

    // Strip the last component
    while (end > path && !file_is_separator(*end)) end--;

    // Strip any separators before the last component
    while (end > path && file_is_separator(*end)) end--;

    // Nothing left - either no directory component, or root
    if (end == path) {
        if (file_is_separator(*path)) {
            // Root path e.g. "/file.txt" -> "/"
            if (buf_sz < 2) return kFilenameTooLong;
            buf[0] = *path;
            buf[1] = '\0';
        } else {
            // No directory component e.g. "file.txt" -> "."
            if (buf_sz < 2) return kFilenameTooLong;
            strcpy(buf, ".");
        }
        return kOk;
    }

    size_t len = end - path + 1;
    if (len >= buf_sz) return kFilenameTooLong;

    memcpy(buf, path, len);
    buf[len] = '\0';
    return kOk;
}

MmResult file_fnmatch(const char *pattern, const char *str, bool *match) {
    CHECK_PARAM(pattern != NULL);
    CHECK_PARAM(str != NULL);
    CHECK_PARAM(match != NULL);

    const char *p = pattern;
    const char *s = str;
    const char *star_p = NULL;  // Position in pattern after last '*'
    const char *star_s = NULL;  // Position in str where last '*' was tried

    while (*s) {
        if (*p == '*') {
            // Record position and advance pattern only, not str
            star_p = ++p;
            star_s = s;
        } else if (*p == '?') {
            p++;
            s++;
        } else if (*p == '[') {
            p++; // Skip '['

            bool negate = false;
            if (*p == '!') {
                negate = true;
                p++;
            }

            bool found = false;
            const char *class_start = p;
            while (*p && (*p != ']' || p == class_start)) {
                if (*(p + 1) == '-' && *(p + 2) && *(p + 2) != ']') {
                    if (*s >= *p && *s <= *(p + 2)) found = true;
                    p += 3;
                } else {
                    if (*s == *p) found = true;
                    p++;
                }
            }
            if (*p == ']') p++;

            if (found == negate) goto backtrack;
            s++;
        } else if (*p == *s) {
            p++;
            s++;
        } else {
            goto backtrack;
        }
        continue;

backtrack:
        if (!star_p) {
            *match = false;
            return kOk;
        }
        // Retry the star match starting one character further in str
        p = star_p;
        s = ++star_s;
    }

    // Skip any trailing stars in pattern
    while (*p == '*') p++;

    *match = (*p == '\0');
    return kOk;
}

/**
 * Comparison function for qsort to sort by filename.
 */
static int compare_by_name(const void *a, const void *b) {
    const FileMatch *file_a = (const FileMatch *)a;
    const FileMatch *file_b = (const FileMatch *)b;

    return strcmp(file_a->name, file_b->name);
}

/**
 * Comparison functions for qsort to sort by file size.
 */
static int compare_by_size(const void *a, const void *b) {
    const FileMatch *file_a = (const FileMatch *)a;
    const FileMatch *file_b = (const FileMatch *)b;

    if (file_a->info.size < file_b->info.size) return -1;
    if (file_a->info.size > file_b->info.size) return 1;
    return strcmp(file_a->name, file_b->name); // Secondary sort by name
}

/**
 * Comparison functions for qsort to sort by file modification time.
 */
static int compare_by_time(const void *a, const void *b) {
    const FileMatch *file_a = (const FileMatch *)a;
    const FileMatch *file_b = (const FileMatch *)b;

    if (file_a->info.mtime < file_b->info.mtime) return -1;
    if (file_a->info.mtime > file_b->info.mtime) return 1;
    return strcmp(file_a->name, file_b->name); // Secondary sort by name
}

/**
 * Comparison functions for qsort to sort by file extension.
 */
static int compare_by_extension(const void *a, const void *b) {
    const FileMatch *file_a = (const FileMatch *)a;
    const FileMatch *file_b = (const FileMatch *)b;

    // Sort by extension, then by name
    const char *ext_a = strrchr(file_a->name, '.');
    const char *ext_b = strrchr(file_b->name, '.');

    // Files without extensions sort before files with extensions
    if (!ext_a && !ext_b) return strcmp(file_a->name, file_b->name);
    if (!ext_a) return -1;
    if (!ext_b) return 1;

    int ext_cmp = strcmp(ext_a, ext_b);
    if (ext_cmp != 0) return ext_cmp;
    return strcmp(file_a->name, file_b->name); // Same extension, sort by name
}

/**
 * Helper function to extract directory and pattern from file specification
 */
MmResult file_parse_fspec(const char *fspec, char *dirname, char *pattern) {
    LOG_FN_ENTRY("fspec=%s", fspec);

    CHECK_PARAM(fspec != NULL);
    CHECK_PARAM(dirname != NULL);
    CHECK_PARAM(pattern != NULL);

    ON_FAILURE_RETURN(path_get_canonical(fspec, dirname, PATH_MAX));

    // If the fspec is just a directory name then return all files
    LOG_DEBUG("dirname = %s", dirname);
    if (file_exists_dir(dirname)) {
        strcpy(pattern, "*");
        RETURN_RESULT(kOk);
    }

    // Find the last slash to separate directory from pattern
    char *last_slash = strrchr(dirname, '/');
    if (!last_slash) RETURN_RESULT(INTERNAL_FAULT);
    if (FAILED(cstring_cpy(pattern, last_slash + 1, STRINGSIZE))) {
        RETURN_RESULT(kStringTooLong);
    }

    // Omit pattern from directory
    *last_slash = '\0';

    RETURN_RESULT(kOk);
}

MmResult file_append_path(char *parent, const char *element, size_t size) {
    CHECK_PARAM(parent != NULL);
    CHECK_PARAM(element != NULL);

    size_t parent_len = strlen(parent);
    size_t element_len = strlen(element);

    // Check if element is empty
    if (element_len == 0) {
        return kOk;
    }

    // Append a separator if necessary
    if (parent_len > 0) {
        const char last_char = parent[parent_len - 1];
        if (!file_is_separator(last_char)) {
            if (FAILED(cstring_cat(parent, PATH_SEPARATOR_STR, size))) {
                return kFilenameTooLong;
            }
        }
    }

    // Skip leading separator in element if parent ends with a separator
    if (parent_len > 0 && file_is_separator(parent[parent_len - 1]) && file_is_separator(element[0])) {
        element++;
        element_len--;
    }

    // Append element
    if (FAILED(cstring_cat(parent, element, size))) {
        return kFilenameTooLong;
    }

    return kOk;
}

MmResult file_list(const char *fspec, FileSort sort, FileList *list) {
    LOG_FN_ENTRY("fspec=%s, sort=%d, list=%p", fspec, sort, list);
    CHECK_PARAM(fspec != NULL);
    CHECK_PARAM(list != NULL);

    // Initialize the list
    memset(list, 0, sizeof(FileList));
    list->count = 0;
    list->buf_full = false;

    char pattern[STRINGSIZE];

    // Parse the file specification
    ON_FAILURE_RETURN(file_parse_fspec(fspec, list->directory, pattern));
    LOG_DEBUG("Pattern: [%s]", pattern);

    // Store the remaining free space in the list
    MmResult result = file_get_free_space(list->directory, &(list->free_space));
    if (FAILED(result)) list->free_space = 0;

    // Open the directory
    DirStream *stream = NULL;
    ON_FAILURE_RETURN(file_opendir(list->directory, &stream));

    char *buf_ptr = list->buf;
    size_t buf_remaining = sizeof(list->buf);
    size_t files_added = 0;

    DirEntry *entry;
    while (true) {
        MmResult result = file_readdir(stream, &entry);
        if (FAILED(result)) {
            file_closedir(stream);
            return result;
        }

        if (!entry) break; // End of directory
        LOG_DEBUG("name: [%s]", entry->name);

        // Skip if the filename does not match the pattern
        bool match = false;
        ON_FAILURE_RETURN(file_fnmatch(pattern, entry->name, &match));
        if (!match) continue;

        // Skip if we've reached the maximum number of files
        if (files_added >= FILE_LIST_MAX) {
            list->count++;
            continue;
        }

        // Check if we have enough buffer space for the filename
        size_t name_len = strlen(entry->name) + 1; // +1 for null terminator
        if (name_len > buf_remaining) {
            list->buf_full = true;
            list->count++;
            continue;
        }

        // Get file statistics
        char full_path[PATH_MAX];
        snprintf_nowarn(full_path, sizeof(full_path), "%s/%s", list->directory, entry->name);

        FileInfo info;
        result = file_info(full_path, &info);
        if (FAILED(result)) {
            file_closedir(stream);
            return result;
        }

        // Add the file to our list
        FileMatch *fmatch = &list->files[files_added];
        fmatch->info = info;

        // Copy the filename to the buffer
        strcpy(buf_ptr, entry->name);
        fmatch->name = buf_ptr;

        // Update buffer pointer and remaining space
        buf_ptr += name_len;
        buf_remaining -= name_len;
        files_added++;
        list->count++;
    }

    file_closedir(stream);

    // Sort the files we successfully added
    if (files_added > 1) {
        int (*compare_func)(const void *, const void *);

        switch (sort) {
            case kFileSortByName:
                compare_func = compare_by_name;
                break;
            case kFileSortBySize:
                compare_func = compare_by_size;
                break;
            case kFileSortByTime:
                compare_func = compare_by_time;
                break;
            case kFileSortByExtension:
                compare_func = compare_by_extension;
                break;
            default:
                compare_func = compare_by_name;
                break;
        }

        qsort(list->files, files_added, sizeof(FileMatch), compare_func);
    }

    return kOk;
}

bool file_exists_regular(const char *path) {
    if (!path) return false;

    FileInfo info;
    if (SUCCEEDED(file_info(path, &info))) {
        return info.exists && (info.type == kFileTypeRegularFile);
    } else {
        return false;
    }
}

bool file_exists_dir(const char *path) {
    LOG_FN_ENTRY("path=%s", path);

    if (!path) return false;

    FileInfo info;
    if (SUCCEEDED(file_info(path, &info))) {
        LOG_DEBUG("info.exists = %d", info.exists);
        LOG_DEBUG("info.type == kFileTypeDirectory = %d", info.type == kFileTypeDirectory);
        LOG_DEBUG("exists_dir = %d", info.exists && (info.type == kFileTypeDirectory));
        return info.exists && (info.type == kFileTypeDirectory);
    } else {
        return false;
    }
}

MmResult file_flush(int fnbr) {
    ON_FAILURE_RETURN(file_validate_fnbr(fnbr));
    errno = 0;
    int result = fflush(file_table[fnbr].file_ptr);
    RETURN_RESULT(SUCCEEDED(result) ? kOk : errno);
}

static MmResult file_get_config_dir_impl(char *buf, size_t size) {
    CHECK_PARAM(buf != NULL);
    ON_FAILURE_RETURN(file_get_home(buf, size));
    return file_append_path(buf, ".mmbasic", size);
}
static inline bool file_is_write_only(int fnbr) {
    const char *mode = file_table[fnbr].mode;
    return (strchr(mode, 'w') && !strchr(mode, '+')) ||
           (strchr(mode, 'a') && !strchr(mode, '+'));
}

int file_eof(int fnbr) {
    static const int error_result = 1; // To match other MMBasic platforms
    ON_FAILURE_ERROR_EX(file_validate_fnbr(fnbr), error_result);

    if (file_is_write_only(fnbr)) {
        RETURN_RESULT(error_result);
    }

    FILE* f = file_table[fnbr].file_ptr;
    clearerr(f);
    errno = 0;
    int ch = fgetc(f);  // Try to read beyond the end of the file.
    if (ch == EOF) {
        if (ferror(f) && !feof(f)) {
            THROW_ERROR(errno ? errno : kError, error_result);
        }
    } else {
        if (ungetc(ch, f) == EOF) {
            THROW_ERROR(errno ? errno : kError, error_result);
        }
    }
    return ch == EOF;
}

MmResult file_normalize_separators(const char *path, char *buf, size_t buf_sz) {
    CHECK_PARAM(path != NULL);
    CHECK_PARAM(buf != NULL);

    if (FAILED(cstring_cpy(buf, path, buf_sz))) {
        return kFilenameTooLong;
    }

    for (char *p = buf; *p; p++) {
        if (*p == '\\') *p = '/';
    }

    return kOk;
}

MmResult file_size(const char *path, off_t *size) {
    LOG_FN_ENTRY("path=%s, size=%p", path, size);

    FileInfo info;
    ON_FAILURE_RETURN(file_info(path, &info));
    if (!info.exists) return kFileNotFound;
    *size = info.size;
    return kOk;
}

static inline bool file_is_read_write(int fnbr) {
    const char *mode = file_table[fnbr].mode;
    return strchr(mode, '+') != NULL || strchr(mode, 'x') != NULL;
}

int file_getc(int fnbr) {
    errno = 0;
    char ch;
    if (fread(&ch, 1, 1, file_table[fnbr].file_ptr) == 0) {
        if (ferror(file_table[fnbr].file_ptr) == 0) {
            RETURN_RESULT(-1);
        } else {
            THROW_ERROR(errno, -1);
        }
    }

    if (file_is_read_write(fnbr)) {
        // No-op seek to satisfy the CRT requirement for read/write files
        fseek(file_table[fnbr].file_ptr, 0, SEEK_CUR);
    }

    RETURN_INT((int)ch);
}

int file_putc(int fnbr, char ch) {
    errno = 0;
    if (fwrite(&ch, 1, 1, file_table[fnbr].file_ptr) == 0) {
        if (ferror(file_table[fnbr].file_ptr)) THROW_ERROR(errno, -1);
        assert(false);  // Always expect ferror to have been set.
    }
    RETURN_INT((int)ch);
}

size_t file_read(int fnbr, char *buf, size_t buf_sz) {
    errno = 0;
    size_t result = fread(buf, 1, buf_sz, file_table[fnbr].file_ptr);
    if (result < buf_sz && ferror(file_table[fnbr].file_ptr)) THROW_ERROR(errno, 0);

    if (file_is_read_write(fnbr)) {
        // No-op seek to satisfy the CRT requirement for read/write files
        fseek(file_table[fnbr].file_ptr, 0, SEEK_CUR);
    }

    RETURN_INT(result);
}

size_t file_write(int fnbr, const char *buf, size_t buf_sz) {
    errno = 0;
    size_t result = fwrite(buf, 1, buf_sz, file_table[fnbr].file_ptr);
    if (result != buf_sz) {
        if (ferror(file_table[fnbr].file_ptr)) THROW_ERROR(errno, 0);
        assert(false);  // Always expect ferror to have been set.
    }
    RETURN_INT(result);
}
