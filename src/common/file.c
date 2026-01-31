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
#include <errno.h>
#include <fnmatch.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

char *file_basename(char *path) {
    return basename(path);
}

char *file_dirname(char *path) {
    return dirname(path);
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

    if (!fspec || !dirname || !pattern) {
        return mmresult_ex(kInternalFault, "Invalid parameter");
    }

    ON_FAILURE_RETURN(path_get_canonical(fspec, dirname, PATH_MAX));

    // If the fspec is just a directory name then return all files
    LOG_DEBUG("dirname = %s", dirname);
    if (file_exists_dir(dirname)) {
        strcpy(pattern, "*");
        return kOk;
    }

    // Find the last slash to separate directory from pattern
    char *last_slash = strrchr(dirname, '/');
    if (!last_slash) ON_FAILURE_RETURN(kInternalFault);
    if (FAILED(cstring_cpy(pattern, last_slash + 1, STRINGSIZE))) {
        return kStringTooLong;
    }

    // Omit pattern from directory
    *last_slash = '\0';

    return kOk;
}

MmResult file_append_path(char *parent, const char *element, size_t size) {
    if (parent == NULL || element == NULL) return INTERNAL_FAULT;

    size_t parent_len = strlen(parent);
    size_t element_len = strlen(element);

    // Check if element is empty
    if (element_len == 0) {
        return kOk;
    }

    // Append a separator if necessary
    const char last_char = parent_len > 0 ? parent[parent_len - 1] : '\0';
    if (last_char != PATH_SEPARATOR && last_char != '/' && last_char != '\\') {
        if (FAILED(cstring_cat(parent, PATH_SEPARATOR_STR, size))) {
            return kFilenameTooLong;
        }
    }

    // Skip leading separator in element if present
    const char first_char = element[0];
    if (first_char == PATH_SEPARATOR || first_char == '/' || first_char == '\\') {
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

    if (!fspec || !list) {
        return mmresult_ex(kInternalFault, "Invalid parameter");
    }

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
        if (fnmatch(pattern, entry->name, 0x0) != 0) {
            continue;
        }

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

static MmResult file_get_config_dir_impl(char *buf, size_t size) {
    if (buf == NULL) return INTERNAL_FAULT;
    ON_FAILURE_RETURN(file_get_home(buf, size));
    return file_append_path(buf, ".mmbasic", size);
}

MmResult file_size(const char *path, off_t *size) {
    LOG_FN_ENTRY("path=%s, size=%p", path, size);

    FileInfo info;
    ON_FAILURE_RETURN(file_info(path, &info));
    if (!info.exists) return kFileNotFound;
    *size = info.size;
    return kOk;
}
