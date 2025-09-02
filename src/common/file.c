/*-*****************************************************************************

MMBasic for Linux (MMB4L)

file.c

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cstring.h"
#include "error.h"
#include "file.h"
#include "file_private.h"
#include "mmb4l.h"
#include "mmgetchar.h"
#include "path.h"
#include "serial.h"
#include "utility.h"

MmResult (*file_0_putc_fn)(char c) = NULL;
MmResult (*file_0_write_fn)(const char *buf, size_t *sz) = NULL;

// We don't use the 0'th entry, but it makes things simpler since MMBasic
// indexes file numbers from 1.
FileEntry file_table[MAXOPENFILES + 1] = { 0 };

MmResult file_init(MmResult (*putc_fn)(char), MmResult (*write_fn)(const char *, size_t *)) {
    file_0_putc_fn = putc_fn;
    file_0_write_fn = write_fn;
    return kOk;
}

char *file_basename(char *path) {
    return basename(path);
}

char *file_dirname(char *path) {
    return dirname(path);
}

MmResult file_close(int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) return kFileInvalidFileNumber;

    switch (file_table[fnbr].type) {
        case fet_closed:
            return kFileNotOpen;

        case fet_file: {
            errno = 0;
            int result = fclose(file_table[fnbr].file_ptr);
            file_table[fnbr].type = fet_closed;
            file_table[fnbr].file_ptr = NULL;
            if (FAILED(result)) return errno;
            break;
        }

        case fet_serial:
            return serial_close(fnbr);
    }

    return kOk;
}

void file_close_all(void) {
    for (int fnbr = 1; fnbr <= MAXOPENFILES; fnbr++) {
        if (file_table[fnbr].type != fet_closed) (void) file_close(fnbr);
    }
}

int file_getc(int fnbr) {
    if (fnbr < 0 || fnbr > MAXOPENFILES) {
        error_throw(kFileInvalidFileNumber);
        return -1;
    }
    if (fnbr == 0) return MMgetchar();

    switch (file_table[fnbr].type) {
        case fet_closed:
            error_throw(kFileNotOpen);
            return -1;

        case fet_file: {
            errno = 0;
            char ch;
            if (fread(&ch, 1, 1, file_table[fnbr].file_ptr) == 0) {
                if (ferror(file_table[fnbr].file_ptr) == 0) {
                    return -1;
                } else {
                    error_throw(errno);
                }
            }
            return (int) ch;
        }

        case fet_serial:
            return serial_getc(fnbr);
    }

    error_throw(kInternalFault);
    return -1;
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
static MmResult file_parse_fspec(const char *fspec, char *dirname, char *pattern) {
    if (!fspec || !dirname || !pattern) {
        return mmresult_ex(kInternalFault, "Invalid parameter");
    }

    ON_FAILURE_RETURN(path_get_canonical(fspec, dirname, PATH_MAX));

    // If the fspec is just a directory name then return all files
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

MmResult file_list(const char *fspec, FileSort sort, FileList *list) {
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
        snprintf(full_path, sizeof(full_path), "%s/%s", list->directory, entry->name);
#pragma GCC diagnostic pop

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

int file_loc(int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) {
        error_throw(kFileInvalidFileNumber);
        return -1;
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            error_throw(kFileNotOpen);
            return -1;

        case fet_file:
            errno = 0;
            long int result = ftell(file_table[fnbr].file_ptr);
            if (result == -1L) error_throw(errno);
            return (int) (result + 1);
            break;

        case fet_serial:
            return serial_rx_queue_size(fnbr);
            break;
    }

    return -1;
}

int file_lof(int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) {
        error_throw(kFileInvalidFileNumber);
        return -1;
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            error_throw(kFileNotOpen);
            return -1;

        case fet_file: {
            errno = 0;
            FILE *f = file_table[fnbr].file_ptr;
            long int current = ftell(f);
            if (current == -1L) error_throw(errno);
            if (FAILED(fseek(f, 0L, SEEK_END))) error_throw(errno);
            long int result = ftell(f);
            if (result == -1L) error_throw(errno);
            if (FAILED(fseek(f, current, SEEK_SET))) error_throw(errno);
            return result;
            break;
        }

        case fet_serial:
            return 0; // Serial I/O ports are unbuffered.
            break;
    }

    return -1;
}

int file_putc(int fnbr, int ch) {
    if (fnbr < 0 || fnbr > MAXOPENFILES) {
        error_throw(kFileInvalidFileNumber);
        return -1;
    }

    if (fnbr == 0) {
        assert(file_0_putc_fn);
        ON_FAILURE_ERROR_EX(file_0_putc_fn(ch), -1);
        return ch;
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            error_throw(kFileNotOpen);
            return -1;

        case fet_file: {
            errno = 0;
            if (fwrite(&ch, 1, 1, file_table[fnbr].file_ptr) == 0) {
                if (ferror(file_table[fnbr].file_ptr)) error_throw(errno);
                assert(false); // Always expect ferror to have been set.
            }
            // TODO: Do I really want to be flushing every character ?
            if (FAILED(fflush(file_table[fnbr].file_ptr))) error_throw(errno);
            return (int) ch;
        }

        case fet_serial:
            return serial_putc(fnbr, ch);
    }

    error_throw(kInternalFault);
    return -1;
}

int file_eof(int fnbr) {
    if (fnbr < 0 || fnbr > MAXOPENFILES) {
        error_throw(kFileInvalidFileNumber);
        return 0;
    }
    if (fnbr == 0) return 0;

    switch (file_table[fnbr].type) {
        case fet_closed:
            error_throw(kFileNotOpen);
            return 0;

        case fet_file: {
            FILE *f = file_table[fnbr].file_ptr;
            errno = 0;
            int ch = fgetc(f); // Try to read beyond the end of the file.
            if (ch == EOF) {
                if (ferror(f)) error_throw(errno);
            } else {
                if (ungetc(ch, f) == EOF) error_throw(errno);
            }
            return ch == EOF;
        }

        case fet_serial:
            return serial_eof(fnbr);
    }

    error_throw(kInternalFault);
    return 1;
}

size_t file_read(int fnbr, char *buf, size_t sz) {
    if (fnbr < 0 || fnbr > MAXOPENFILES) {
        ON_FAILURE_ERROR_EX(kFileInvalidFileNumber, 0);
    }
    assert(fnbr != 0); // if (fnbr == 0) return console_write(buf, sz);

    switch (file_table[fnbr].type) {
        case fet_closed:
            error_throw(kFileNotOpen);
            return 0;

        case fet_file: {
            errno = 0;
            size_t result = fread(buf, 1, sz, file_table[fnbr].file_ptr);
            if (result < sz && ferror(file_table[fnbr].file_ptr)) error_throw(errno);
            return result;
        }

        case fet_serial:
            assert(false); // return serial_write(fnbr, buf, sz);
            break;
    }

    ON_FAILURE_ERROR_EX(kInternalFault, 0);

    return 0;
}

void file_seek(int fnbr, int idx) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) {
        error_throw(kFileInvalidFileNumber);
        return;
    }
    if (idx < 1) {
        error_throw(kFileInvalidSeekPosition);
        return;
    }

    if (file_table[fnbr].type == fet_closed) {
        error_throw(kFileNotOpen);
        return;
    }
    FILE *f = file_table[fnbr].file_ptr;

    errno = 0;
    if (FAILED(fflush(f))) error_throw(errno);
    if (FAILED(fsync(fileno(f)))) error_throw(errno);
    if (FAILED(fseek(f, idx - 1, SEEK_SET))) error_throw(errno); // MMBasic indexes from 1, not 0.
}

int file_find_free(void) {
    for (int fnbr = 1; fnbr <= MAXOPENFILES; fnbr++) {
        if (file_table[fnbr].type == fet_closed) return fnbr;
    }
    error_throw(kTooManyOpenFiles);
    return -1;
}

size_t file_write(int fnbr, const char *buf, size_t sz) {
    if (fnbr < 0 || fnbr > MAXOPENFILES) {
        error_throw(kFileInvalidFileNumber);
        return 0;
    }

    if (fnbr == 0) {
        assert(file_0_write_fn);
        ON_FAILURE_ERROR_EX(file_0_write_fn(buf, &sz), 0);
        return sz;
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            error_throw(kFileNotOpen);
            return 0;

        case fet_file: {
            errno = 0;
            size_t result = fwrite(buf, 1, sz, file_table[fnbr].file_ptr);
            if (result != sz) {
                if (ferror(file_table[fnbr].file_ptr)) error_throw(errno);
                assert(false); // Always expect ferror to have been set.
            }
            if (FAILED(fflush(file_table[fnbr].file_ptr))) error_throw(errno);
            return result;
        }

        case fet_serial:
            return serial_write(fnbr, buf, sz);
            break;
    }

    error_throw(kInternalFault);
    return -1;
}

bool file_exists_regular(const char *filename) {
    if (!filename) return false;

    FileInfo info;
    if (SUCCEEDED(file_info(filename, &info))) {
        return info.exists && (info.type == kFileTypeRegularFile);
    } else {
        return false;
    }
}

bool file_exists_dir(const char *dirname) {
    if (!dirname) return false;

    FileInfo info;
    if (SUCCEEDED(file_info(dirname, &info))) {
        return info.exists && (info.type == kFileTypeDirectory);
    } else {
        return false;
    }
}

MmResult file_size(const char *path, off_t *size) {
    FileInfo info;
    ON_FAILURE_RETURN(file_info(path, &info));
    if (!info.exists) return kFileNotFound;
    *size = info.size;
    return kOk;
}

bool file_is_file(int fnbr) {
    assert(fnbr >= 0 && fnbr <= MAXOPENFILES);
    if (fnbr >= 0 && fnbr <= MAXOPENFILES) {
        return file_table[fnbr].type == fet_file;
    } else {
        return false;
    }
}

bool file_is_serial(int fnbr) {
    assert(fnbr >= 0 && fnbr <= MAXOPENFILES);
    if (fnbr >= 0 && fnbr <= MAXOPENFILES) {
        return file_table[fnbr].type == fet_serial;
    } else {
        return false;
    }
}
