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

// #include "cstring.h"
#include "error.h"
#include "file.h"
#include "file_private.h"
// #include "utility.h"

MmResult file_chdir(const char *dirname) {
    RETURN_RESULT(kUnimplemented);
}

MmResult file_delete(const char *filename) {
    RETURN_RESULT(kUnimplemented);
}

MmResult file_closedir(DirStream *stream) {
    RETURN_RESULT(kUnimplemented);
}

bool file_exists_symlink(const char *path) {
    LOG_WARN("UNIMPLEMENTED");
    RETURN_BOOL(false);
}

int file_fsync(int fd) {
    LOG_WARN("UNIMPLEMENTED");
    RETURN_INT(0);
}

MmResult file_getcwd(char *buf, size_t buf_sz) {
    CHECK_PARAM(buf != NULL);
    RETURN_RESULT(kUnimplemented);
}

MmResult file_get_free_space(const char *path, uint64_t *free_space) {
    CHECK_PARAM(path != NULL);
    CHECK_PARAM(free_space != NULL);
    RETURN_RESULT(kUnimplemented);
}

MmResult file_get_home(char *buf, size_t buf_sz) {
    RETURN_RESULT(kUnimplemented);
}

MmResult file_info(const char *filename, FileInfo *info) {
    CHECK_PARAM(filename != NULL);
    CHECK_PARAM(info != NULL);
    RETURN_RESULT(kUnimplemented);
}

MmResult file_open(const char *path, const char *mode, int fnbr) {
    RETURN_RESULT(kUnimplemented);
}

MmResult file_opendir(const char *dirname, DirStream **stream) {
    CHECK_PARAM(dirname != NULL);
    CHECK_PARAM(stream != NULL);
    RETURN_RESULT(kUnimplemented);
}

MmResult file_mkdir(const char *dirname) {
    CHECK_PARAM(dirname != NULL);
    RETURN_RESULT(kUnimplemented);
}

MmResult file_mkfile(const char *filename) {
    CHECK_PARAM(filename != NULL);
    RETURN_RESULT(kUnimplemented);
}

MmResult file_readdir(DirStream *stream, DirEntry **entry) {
    CHECK_PARAM(stream != NULL);
    CHECK_PARAM(entry != NULL);
    RETURN_RESULT(kUnimplemented);
}

MmResult file_readlink(const char *path, char *buf, size_t *bufsiz) {
    RETURN_RESULT(kUnimplemented);
}

MmResult file_rename(const char *old_filename, const char *new_filename) {
    CHECK_PARAM(old_filename != NULL);
    CHECK_PARAM(new_filename != NULL);
    RETURN_RESULT(kUnimplemented);
}

MmResult file_rmdir(const char *dirname) {
    CHECK_PARAM(dirname != NULL);
    RETURN_RESULT(kUnimplemented);
}
