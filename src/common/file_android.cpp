/*-*****************************************************************************

MMBasic for Linux (MMB4L)

file_android.cpp

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

#include <cstdlib>
#include <fnmatch.h>
#include <string.h>
#include <sys/stat.h>
#include <vector>

extern "C" {

#include "cstring.h"
#include "error.h"
#include "file.h"
#include "file_private.h"
#include "logger.h"
#include "mmb4l.h"
#include "mmgetchar.h"
#include "path.h"
#include "serial.h"
#include "utility.h"

} // extern "C"

#include "saf_bridge.h"

// MmResult file_list(const char *fspec, FileSort sort, FileList *list) {
//     if (!fspec || !list) {
//         return mmresult_ex(kInternalFault, "Invalid parameter");
//     }

//     auto files = saf_list_files();
//     if (files.empty()) {
//         LOG_INFO("No files found");
//     }
//     for (const auto &file : files) {
//         LOG_INFO("Found file: %s", file.c_str());
//     }

//     return mmresult_ex(kInternalFault, "Not implemented for Android");
// }

struct s_DirStream {
    std::vector<std::string> files;
    DirEntry entry;
    size_t next;
};

MmResult file_opendir(const char *dirname, DirStream **stream) {
    LOG_INFO("Entered %s()", __func__);
    if (!dirname) return mmresult_ex(kInternalFault, "dirname == NULL");
    struct s_DirStream *ds = new s_DirStream();
    ds->files = saf_list_files();
    // LOG_INFO("saf_list_files() returned");
    ds->next = 0;
    *stream = ds;
    LOG_INFO("Exited %s()", __func__);
    return kOk;
}

MmResult file_readdir(DirStream *stream, DirEntry **entry) {
    LOG_INFO("Entered %s()", __func__);
    if (!stream) return mmresult_ex(kInternalFault, "stream == NULL");

    if (stream->next >= stream->files.size()) {
        *entry = NULL;
        return kOk;
    }

    if (FAILED(cstring_cpy(stream->entry.name, stream->files[stream->next].c_str(), STRINGSIZE))) {
        *entry = NULL;
        return kStringTooLong;
    }

    stream->next++;

    *entry = &(stream->entry);
    return kOk;
}

MmResult file_closedir(DirStream *stream) {
    LOG_INFO("Entered %s()", __func__);
    free(stream);
    LOG_INFO("Exited %s()", __func__);
    return kOk;
}

MmResult file_get_free_space(const char *path, uint64_t *free_space) {
    if (!path || !free_space) {
        return mmresult_ex(kInternalFault, "Invalid parameter");
    }

    *free_space = saf_free_space();

    return kOk;
}

MmResult file_info(const char *filename, FileInfo *info) {
    if (!filename) return mmresult_ex(kInternalFault, "filename == NULL");
    if (!info) return mmresult_ex(kInternalFault, "info == NULL");

    // HACK!
    filename += 2;

    SAFFileInfo saf_info = saf_get_file_info(filename);
    info->size = saf_info.size;
    info->time = saf_info.last_modified / 1000;
    info->type = saf_info.is_file ? kFileTypeRegularFile : kFileTypeDirectory;

    return kOk;
}
