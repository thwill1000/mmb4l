/*-*****************************************************************************

MMBasic for Linux (MMB4L)

file_linux.c

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

#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include "cstring.h"
#include "error.h"
#include "file.h"
#include "file_private.h"
#include "utility.h"

struct s_DirStream {
    DIR *dir;
    DirEntry entry;
};

MmResult file_chdir(const char *dirname) {
    CHECK_PARAM(dirname != NULL);

    errno = 0;
    if (SUCCEEDED(chdir(dirname))) {
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

    errno = 0;
    if (SUCCEEDED(closedir(stream->dir))) {
        free(stream);
        return kOk;
    } else {
        free(stream);
        return errno;
    }
}

bool file_exists_symlink(const char *path) {
    struct stat st;
    // Note use of lstat() rather than stat(), the latter would follow the symbolic link.
    return SUCCEEDED(lstat(path, &st)) && S_ISLNK(st.st_mode);
}

int file_fsync(int fd) {
    return fsync(fd);
}

MmResult file_getcwd(char *buf, size_t buf_sz) {
    CHECK_PARAM(buf != NULL);

    errno = 0;
    if (getcwd(buf, buf_sz)) {
        return kOk;
    } else {
        return errno;
    }
}

MmResult file_get_free_space(const char *path, uint64_t *free_space) {
    CHECK_PARAM(path != NULL);
    CHECK_PARAM(free_space != NULL);

    struct statvfs fs_stat;
    errno = 0;
    if (SUCCEEDED(statvfs(path, &fs_stat))) {
        // Calculate free space: available blocks * block size
        // Use f_bavail (blocks available to non-privileged users) rather than f_bfree
        *free_space = (uint64_t)fs_stat.f_bavail * (uint64_t)fs_stat.f_frsize;
        return kOk;
    } else {
        return errno;
    }
}

MmResult file_get_home(char *buf, size_t buf_sz) {
    CHECK_PARAM(buf != NULL);

    errno = 0;
    const char *home = getenv("HOME");
    if (!home) return errno; // Probably never happens.
    if (FAILED(cstring_cpy(buf, home, buf_sz))) {
        return kFilenameTooLong;
    }
    return kOk;
}

MmResult file_info(const char *filename, FileInfo *info) {
    CHECK_PARAM(filename != NULL);
    CHECK_PARAM(info != NULL);

    struct stat st;
    if (SUCCEEDED(stat(filename, &st))) {
        info->exists = true;
        info->size = st.st_size;
        info->mtime = st.st_mtime;

        if (S_ISREG(st.st_mode)) {
            info->type = kFileTypeRegularFile;
        } else if (S_ISDIR(st.st_mode)) {
            info->type = kFileTypeDirectory;
        } else if (S_ISLNK(st.st_mode)) {
            info->type = kFileTypeSymbolicLink;
        } else if (S_ISBLK(st.st_mode)) {
            info->type = kFileTypeBlockDevice;
        } else if (S_ISCHR(st.st_mode)) {
            info->type = kFileTypeCharacterDevice;
        } else if (S_ISFIFO(st.st_mode)) {
            info->type = kFileTypeNamedPipe;
        } else if (S_ISSOCK(st.st_mode)) {
            info->type = kFileTypeSocket;
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

MmResult file_open(const char *path, const char *mode, FILE **file) {
    CHECK_PARAM(path != NULL);
    CHECK_PARAM(mode != NULL);
    CHECK_PARAM(file != NULL);

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
            if (!f) return errno;
        }
        errno = 0;
        if (FAILED(fseek(f, 0, SEEK_END))) return errno;
    } else {
        errno = 0;
        f = fopen(path, mode);
        if (!f) return errno;
    }

    *file = f;
    return kOk;
}

MmResult file_opendir(const char *dirname, DirStream **stream) {
    CHECK_PARAM(dirname != NULL);
    CHECK_PARAM(stream != NULL);

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

MmResult file_mkdir(const char *dirname) {
    CHECK_PARAM(dirname != NULL);

    errno = 0;
    if (SUCCEEDED(mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))) {
        return kOk;
    } else {
        return errno;
    }
}

MmResult file_mkfile(const char *filename) {
    CHECK_PARAM(filename != NULL);

    FileInfo info;
    ON_FAILURE_RETURN(file_info(filename, &info));
    if (info.exists) return kFileExists;

    errno = 0;
    FILE* file = fopen(filename, "w");
    if (file) {
        fclose(file);
        return kOk;
    } else {
        return errno;
    }
}

MmResult file_readdir(DirStream *stream, DirEntry **entry) {
    CHECK_PARAM(stream != NULL);
    CHECK_PARAM(entry != NULL);

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

MmResult file_readlink(const char *path, char *buf, size_t *buf_sz) {
    CHECK_PARAM(path != NULL);
    CHECK_PARAM(buf != NULL);

    errno = 0;
    ssize_t result = readlink(path, buf, *buf_sz);
    if (result == -1) {
        return errno;
    } else {
        *buf_sz = (size_t) result;
        return kOk;
    }
}

MmResult file_rename(const char *old_filename, const char *new_filename) {
    CHECK_PARAM(old_filename != NULL);
    CHECK_PARAM(new_filename != NULL);

    errno = 0;
    if SUCCEEDED(rename(old_filename, new_filename)) {
        return kOk;
    } else {
        return errno;
    }
}

MmResult file_rmdir(const char *dirname) {
    CHECK_PARAM(dirname != NULL);

    errno = 0;
    if (SUCCEEDED(rmdir(dirname))) {
        return kOk;
    } else {
        return errno;
    }
}
