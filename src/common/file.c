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
#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <libgen.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
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

struct s_DirStream {
    DIR *dir;
    DirEntry entry;
};

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

/**
 * @param  filename  filename in C-string style, not MMBasic style.
 */
MmResult file_open(const char *filename, const char *mode, int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) return kFileInvalidFileNumber;
    if (file_table[fnbr].type != fet_closed) return kFileAlreadyOpen;

    // random writing is not allowed when a file is opened for append so open it
    // first for read+update and if that does not work open it for
    // writing+update.  This has the same effect as opening for append+update
    // but will allow writing
    FILE *f = NULL;
    if (*mode == 'x') {
        errno = 0;
        f = fopen(filename, "rb+");
        if (!f) {
            errno = 0;
            f = fopen(filename, "wb+");
            if (!f) return errno;
        }
        errno = 0;
        if (FAILED(fseek(f, 0, SEEK_END))) return errno;
    } else {
        errno = 0;
        f = fopen(filename, mode);
        if (!f) return errno;
    }

    file_table[fnbr].type = fet_file;
    file_table[fnbr].file_ptr = f;

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

MmResult file_delete(const char *filename) {
    errno = 0;
    if (SUCCEEDED(remove(filename))) {
        return kOk;
    } else {
        return errno;
    }
}

MmResult file_get_free_space(const char *path, uint64_t *free_space) {
    if (!path || !free_space) {
        return mmresult_ex(kInternalFault, "Invalid parameter");
    }

    struct statvfs fs_stat;
    errno = 0;
    if (statvfs(path, &fs_stat) != 0) {
        return errno;
    }

    // Calculate free space: available blocks * block size
    // Use f_bavail (blocks available to non-privileged users) rather than f_bfree
    *free_space = (uint64_t)fs_stat.f_bavail * (uint64_t)fs_stat.f_frsize;
    
    return kOk;
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

    if (file_a->size < file_b->size) return -1;
    if (file_a->size > file_b->size) return 1;
    return strcmp(file_a->name, file_b->name); // Secondary sort by name
}

/**
 * Comparison functions for qsort to sort by file modification time.
 */
static int compare_by_time(const void *a, const void *b) {
    const FileMatch *file_a = (const FileMatch *)a;
    const FileMatch *file_b = (const FileMatch *)b;

    if (file_a->time < file_b->time) return -1;
    if (file_a->time > file_b->time) return 1;
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

        struct stat st;
        if (stat(full_path, &st) != 0) {
            // If we can't stat the file, make stuff up, perhaps we should omit it ?
            st.st_size = 0;
            st.st_mtime = 0;
        }

        // Add the file to our list
        FileMatch *fmatch = &list->files[files_added];

        // Copy the filename to the buffer
        strcpy(buf_ptr, entry->name);
        fmatch->name = buf_ptr;
        fmatch->size = st.st_size;
        fmatch->time = st.st_mtime;

        // Set file type based on stat information
        // Note: We use stat() result rather than DirEntry#type for more reliable results
        if (S_ISREG(st.st_mode)) {
            fmatch->type = kFileTypeRegularFile;
        } else if (S_ISDIR(st.st_mode)) {
            fmatch->type = kFileTypeDirectory;
        } else if (S_ISLNK(st.st_mode)) {
            fmatch->type = kFileTypeSymbolicLink;
        } else if (S_ISBLK(st.st_mode)) {
            fmatch->type = kFileTypeBlockDevice;
        } else if (S_ISCHR(st.st_mode)) {
            fmatch->type = kFileTypeCharacterDevice;
        } else if (S_ISFIFO(st.st_mode)) {
            fmatch->type = kFileTypeNamedPipe;
        } else if (S_ISSOCK(st.st_mode)) {
            fmatch->type = kFileTypeSocket;
        } else {
            fmatch->type = kFileTypeUnknown;
        }

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

bool file_exists(const char *filename) {
    struct stat st;
    return (stat(filename, &st) == 0) && S_ISREG(st.st_mode) ? true : false;
}

bool file_exists_dir(const char *dirname) {
    if (!dirname) return false;

    struct stat st;
    return (stat(dirname, &st) == 0) && S_ISDIR(st.st_mode) ? true : false;
}

int64_t file_size(int fnbr) {
    struct stat st;
    if (fstat(fileno(file_table[fnbr].file_ptr), &st) == 0) {
        return st.st_size;
    } else {
        // File probably doesn't exist.
        // TODO: Check errno.
        return -1;
    }
}

MmResult file_chdir(const char *dirname) {
    errno = 0;
    if (FAILED(chdir(dirname))) return errno;
    return kOk;
}

MmResult file_rmdir(const char *dirname) {
    errno = 0;
    if (FAILED(rmdir(dirname))) return errno;
    return kOk;
}

MmResult file_getcwd(char *buf, size_t size) {
    errno = 0;
    if (!getcwd(buf, size)) return errno;
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

MmResult file_readlink(const char *path, char *buf, size_t *bufsiz) {
    errno = 0;
    ssize_t result = readlink(path, buf, *bufsiz);
    if (result == -1) {
        return errno;
    } else {
        *bufsiz = (size_t) result;
        return kOk;
    }
}

MmResult file_rename(const char *old_filename, const char *new_filename) {
    errno = 0;
    if FAILED(rename(old_filename, new_filename)) {
        return errno;
    } else {
        return kOk;
    }
}

MmResult file_mkdir(const char *dirname) {
    // TODO: check/validate mode/permissions.
    errno = 0;
    if (FAILED(mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))) {
        return errno;
    } else {
        return kOk;
    }
}

MmResult file_opendir(const char *dirname, DirStream **stream) {
    if (!dirname) return mmresult_ex(kInternalFault, "dirname == NULL");
    errno = 0;
    DIR *dir = opendir(dirname);
    if (dir) {
        DirStream *ds = (DirStream *) malloc(sizeof(DirStream));
        if (!ds) {
            closedir(dir);
            return kOutOfMemory;
        }
        ds->dir = dir;
        *stream = ds;
        return kOk;
    } else {
        *stream = NULL;
        return errno;
    }
}

MmResult file_readdir(DirStream *stream, DirEntry **entry) {
    if (!stream) return mmresult_ex(kInternalFault, "stream == NULL");
    errno = 0;
    struct dirent *e = readdir(stream->dir);
    if (!e) {
        if (errno == 0) {
            // End of directory, not an error
            *entry = NULL;
            return kOk;
        } else {
            *entry = NULL;
            return errno;
        }
    }

    if (FAILED(cstring_cpy(stream->entry.name, e->d_name, STRINGSIZE))) {
        *entry = NULL;
        return kStringTooLong;
    }

    switch (e->d_type) {
        case DT_BLK:
            stream->entry.type = kFileTypeBlockDevice;
            break;
        case DT_CHR:
            stream->entry.type = kFileTypeCharacterDevice;
            break;
        case DT_DIR:
            stream->entry.type = kFileTypeDirectory;
            break;
        case DT_FIFO:
            stream->entry.type = kFileTypeNamedPipe;
            break;
        case DT_LNK:
            stream->entry.type = kFileTypeSymbolicLink;
            break;
        case DT_REG:
            stream->entry.type = kFileTypeRegularFile;
            break;
        case DT_SOCK:
            stream->entry.type = kFileTypeSocket;
            break;
        default:
            stream->entry.type = kFileTypeUnknown;
            break;
    }

    *entry = &(stream->entry);
    return kOk;
}

MmResult file_closedir(DirStream *stream) {
    if (!stream) return mmresult_ex(kInternalFault, "stream == NULL");
    errno = 0;
    if (SUCCEEDED(closedir(stream->dir))) {
        free(stream);
        return kOk;
    } else {
        free(stream);
        return errno;
    }
}
