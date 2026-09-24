/*-*****************************************************************************

MMBasic for Linux (MMB4L)

file_windows.c

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

#include <windows.h>
#include <direct.h>
#include <io.h>
#include <stdlib.h>
#include <sys/stat.h>

// Undefine HRESULT macros that conflict with MMB4L definitions
#undef FAILED
#undef SUCCEEDED

#include "cstring.h"
#include "error.h"
#include "file.h"
#include "file_private.h"
#include "path.h"

#ifndef S_ISREG
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#endif

#ifndef S_ISDIR
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#endif

struct s_DirStream {
    HANDLE handle;
    WIN32_FIND_DATAA find_data;
    bool first;  // FindFirstFile already retrieves the first entry
    DirEntry entry;
};

MmResult file_chdir(const char *dirname) {
    CHECK_PARAM(dirname != NULL);

    errno = 0;
    if (SUCCEEDED(_chdir(dirname))) {
        return kOk;
    } else {
        return errno;
    }
}

MmResult file_delete(const char *filename) {
    CHECK_PARAM(filename != NULL);

    errno = 0;
    if (SUCCEEDED(remove(filename))) {
        return kOk;
    } else {
        return errno;
    }
}

MmResult file_closedir(DirStream *stream) {
    CHECK_PARAM(stream != NULL);

    if (FindClose(stream->handle)) {
        free(stream);
        return kOk;
    } else {
        free(stream);
        return mmresult_ex(kError, "Failed to close directory stream");
    }
}

bool file_exists_symlink(const char *path) {
    // Symbolic links are not currently support on Windows
    RETURN_BOOL(false);
}

int file_fsync(int fd) {
    return _commit(fd);
}

MmResult file_getcwd(char *buf, size_t buf_sz) {
    CHECK_PARAM(buf != NULL);

    errno = 0;
    char cwd[PATH_MAX];
    if (!_getcwd(cwd, (int) sizeof(cwd))) {
        return errno ? errno : mmresult_ex(kError, "Failed to get current working directory");
    }

    ON_FAILURE_RETURN(file_normalize_separators(cwd, buf, buf_sz));
    file_strip_trailing_separator(buf);
    return kOk;
}

MmResult file_get_free_space(const char *path, uint64_t *free_space) {
    CHECK_PARAM(path != NULL);
    CHECK_PARAM(free_space != NULL);

    // Extract the drive name from the path (e.g. "C:\")
    char drive[MAX_PATH];
    ON_FAILURE_RETURN(path_get_canonical(path, drive, sizeof(drive)));
    char *colon = strchr(drive, ':');
    if (colon) {
        *(colon + 1) = '\0';
    }

    ULARGE_INTEGER available_bytes;
    if (GetDiskFreeSpaceEx(drive, &available_bytes, NULL, NULL)) {
        *free_space = (uint64_t)available_bytes.QuadPart;
        return kOk;
    } else {
        return mmresult_ex(kError, "Failed to get free space for drive");
    }
}

MmResult file_get_home(char *buf, size_t buf_sz) {
    CHECK_PARAM(buf != NULL);

    errno = 0;
    const char *home = getenv("USERPROFILE");
    if (!home) {
        return errno ? errno : mmresult_ex(kError, "Failed to get home directory");
    }

    ON_FAILURE_RETURN(file_normalize_separators(home, buf, buf_sz));
    file_strip_trailing_separator(buf);
    return kOk;
}

MmResult file_info(const char *filename, FileInfo *info) {
    CHECK_PARAM(filename != NULL);
    CHECK_PARAM(info != NULL);

    // Handle drive paths without trailing separator, e.g. "C:" should be treated as "C:/"
    char drive[4];
    if (strlen(filename) == 2 && isalpha(filename[0]) && filename[1] == ':') {
        drive[0] = filename[0];
        drive[1] = ':';
        drive[2] = '/';
        drive[3] = '\0';
        filename = drive;
    }

    struct stat st;
    if (SUCCEEDED(stat(filename, &st))) {
        info->exists = true;
        info->size = st.st_size;
        info->mtime = st.st_mtime;

        if (S_ISREG(st.st_mode)) {
            info->type = kFileTypeRegularFile;
        } else if (S_ISDIR(st.st_mode)) {
            info->type = kFileTypeDirectory;
        } else {
            info->type = kFileTypeUnknown;
        }
    } else {
        info->exists = false;
        info->size = 0;
        info->mtime = 0;
        info->type = kFileTypeUnknown;
    }

    return kOk;
}

bool file_is_absolute(const char *path) {
    if (path == NULL) return false;

    // Allow UNIX style absolute paths
    if (path[0] == '/' || path[0] == '\\') return true;

    // UNC path: \\server\share
    if (path[0] == '\\' && path[1] == '\\') return true;

    // Drive letter: C:\ or C:/
    if (isalpha(path[0]) && path[1] == ':' && (path[2] == '\\' || path[2] == '/')) return true;

    return false;
}

MmResult file_open(const char *path, const char *mode, int fnbr) {
    CHECK_PARAM(path != NULL);
    CHECK_PARAM(mode != NULL);
    ON_FAILURE_RETURN(file_validate_fnbr(fnbr));
    if (file_table[fnbr].type != fet_closed) RETURN_RESULT(kFileAlreadyOpen);

    // Random writing is not allowed when a file is opened for append so open it
    // first for read & update and if that does not work open it for
    // write & update. This has the same effect as opening for append & update
    // but will allow writing.
    FILE *f = NULL;
    if (*mode == 'x') {
        errno = 0;
        f = fopen(path, "rb+");
        if (!f) {
            errno = 0;
            f = fopen(path, "wb+");
            if (!f) RETURN_RESULT(errno);
        }
    } else {
        errno = 0;
        f = fopen(path, mode);
        if (!f) RETURN_RESULT(errno);
    }

    // Seek to end to ensure correct position returned by first call to ftell()
    if (*mode == 'x' || *mode == 'a') {
        errno = 0;
        if (FAILED(fseek(f, 0, SEEK_END))) {
            MmResult result = errno ? errno : mmresult_ex(kError, "Failed to seek to end of file");
            fclose(f);
            RETURN_RESULT(result);
        }
    }

    file_table[fnbr].type = fet_file;
    file_table[fnbr].file_ptr = f;
    strcpy(file_table[fnbr].mode, mode);

    RETURN_RESULT(kOk);
}

MmResult file_opendir(const char *dirname, DirStream **stream) {
    CHECK_PARAM(dirname != NULL);
    CHECK_PARAM(stream != NULL);

    *stream = NULL;

    // Check that the directory exists
    FileInfo info;
    ON_FAILURE_RETURN(file_info(dirname, &info));
    if (!info.exists) return kFileNotFound;
    if (info.type != kFileTypeDirectory) return kNotADirectory;

    DirStream *ds = (DirStream *) malloc(sizeof(DirStream));
    if (!ds) return kOutOfMemory;

    // FindFirstFile requires a wildcard pattern
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*", dirname);

    ds->handle = FindFirstFileA(pattern, &ds->find_data);
    if (ds->handle == INVALID_HANDLE_VALUE) {
        free(ds);
        *stream = NULL;
        return mmresult_ex(kError, "Failed to open directory stream");
    }
    ds->first = true;  // First entry already retrieved by FindFirstFile
    *stream = ds;
    return kOk;
}

static bool file_is_drive_path(const char *path) {
    if (path == NULL) return false;

    // Must have at least a drive letter and colon e.g. "C:"
    if (!isalpha(path[0]) || path[1] != ':') return false;

    if (path[2] == '\0') {
        // After the colon must be either end of string ...
        return true;
    } else if (path[2] == '\\' || path[2] == '/') {
        // ... or backslash or forward slash followed by end of string
        return path[3] == '\0';
    } else {
        return false;
    }
}

MmResult file_mkdir(const char *dirname) {
    CHECK_PARAM(dirname != NULL);

    // If dirname is just a Windows drive path (e.g. "C:" or "C:\" or "C:/")
    // then return success if the drive exists and is accessible
    if (file_is_drive_path(dirname)) {
        FileInfo info;
        ON_FAILURE_RETURN(file_info(dirname, &info));
        return info.exists ? kOk : kFileNotFound;
    }

    errno = 0;
    if (SUCCEEDED(_mkdir(dirname))) {
        return kOk;
    } else {
        return errno;
    }
}

MmResult file_mkfile(const char *filename, const char *contents) {
    CHECK_PARAM(filename != NULL);

    FileInfo info;
    ON_FAILURE_RETURN(file_info(filename, &info));
    if (info.exists) return kFileExists;

    errno = 0;
    FILE* file = fopen(filename, "w");
    if (file) {
        if (contents) {
            fputs(contents, file);
        }
        fclose(file);
        return kOk;
    } else {
        return errno;
    }
}

MmResult file_mksymlink(const char *target, const char *link) {
    CHECK_PARAM(target != NULL);
    CHECK_PARAM(link != NULL);
    return mmresult_ex(kUnimplemented, "Symbolic link creation is not implemented on Windows");
}

MmResult file_readdir(DirStream *stream, DirEntry **entry) {
    CHECK_PARAM(stream != NULL);
    CHECK_PARAM(entry != NULL);

    if (stream->first) {
        // First entry already retrieved by FindFirstFile
        stream->first = false;
    } else {
        if (!FindNextFileA(stream->handle, &stream->find_data)) {
            if (GetLastError() == ERROR_NO_MORE_FILES) {
                // End of directory, not an error
                *entry = NULL;
                return kOk;
            } else {
                *entry = NULL;
                return mmresult_ex(kError, "Failed to read next directory entry");
            }
        }
    }

    if (FAILED(cstring_cpy(stream->entry.name, stream->find_data.cFileName, STRINGSIZE))) {
        *entry = NULL;
        return kStringTooLong;
    }

    if (stream->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        stream->entry.type = kFileTypeDirectory;
    } else if (stream->find_data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        stream->entry.type = kFileTypeSymbolicLink;
    } else {
        stream->entry.type = kFileTypeRegularFile;
    }

    *entry = &(stream->entry);
    return kOk;
}

MmResult file_readlink(const char *path, char *buf, size_t *buf_sz) {
    // printf("file_readlink: UNIMPLEMENTED (path='%s')\n", path);
    // CHECK_PARAM(path != NULL);
    // CHECK_PARAM(buf != NULL);
    // CHECK_PARAM(buf_sz != NULL);
    // RETURN_RESULT(kUnimplemented);
    RETURN_RESULT(EINVAL);
}

MmResult file_rename(const char *old_filename, const char *new_filename) {
    CHECK_PARAM(old_filename != NULL);
    CHECK_PARAM(new_filename != NULL);

    errno = 0;
    if (SUCCEEDED(rename(old_filename, new_filename))) {
        return kOk;
    } else {
        return errno;
    }
}

MmResult file_rmdir(const char *dirname) {
    CHECK_PARAM(dirname != NULL);

    errno = 0;
    if (SUCCEEDED(_rmdir(dirname))) {
        return kOk;
    } else {
        return errno;
    }
}
