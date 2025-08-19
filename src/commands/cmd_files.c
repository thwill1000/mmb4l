/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_files.c

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

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../common/cstring.h"
#include "../common/display.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/memory.h"
#include "../common/mmb4l.h"
#include "../core/Commands.h"

void cmd_files_internal(const char *p) {
    getargs(&p, 3, DELIM_COMMA);
    if (argc != 0 && argc != 1 && argc != 3) ON_FAILURE_ERROR(kArgumentCount);
    const char *fspec = has_arg(0) ? getCstring(argv[0]) : "";
    FileSort sort = kFileSortByName;
    if (has_arg(2)) {
        if (checkstring(argv[2], "NAME")) {
            sort = kFileSortByName;
        } else if (checkstring(argv[2], "TIME")) {
            sort = kFileSortByTime;
        } else if (checkstring(argv[2], "SIZE")) {
            sort = kFileSortBySize;
        } else if (checkstring(argv[2], "TYPE")) {
            sort = kFileSortByExtension;
        } else {
            ON_FAILURE_ERROR(kSyntax);
        }
    }

    FileList *flist = GetTempMemory(sizeof(FileList));
    ON_FAILURE_ERROR(file_list(fspec, sort, flist));

    char *buf = GetTempStrMemory();
    size_t dir_count = 0;
    size_t file_count = 0;

    // Use the current display dimensions for list control
    ON_FAILURE_ERROR(display_get_size(false, &mmb_options.width, &mmb_options.height));
    int list_count = 2;
    const bool compact = mmb_options.width <= 80;

    // Print queried directory
    display_puts(flist->directory);
    // TODO: ListNewLine() should take the line to display and be reponsible
    //       for handling lines that are wider than the display.
    list_count += ((strlen(flist->directory) + mmb_options.width - 1) / mmb_options.width) - 1;
    ListNewLine(&list_count, 0);

    // List directories first
    for (size_t i = 0; i < min(flist->count, (size_t) FILE_LIST_MAX); ++i) {
        FileMatch *file = &(flist->files[i]);
        if (file->info.type != kFileTypeDirectory) continue;
        (void) snprintf(buf, STRINGSIZE, "   <DIR>  %s", file->name);
        display_puts(buf);
        dir_count++;
        // TODO: See above
        list_count += ((strlen(buf) + mmb_options.width - 1) / mmb_options.width) - 1;
        ListNewLine(&list_count, 0);
    }

    // List everything else
    char time_buf[32];
    for (size_t i = 0; i < min(flist->count, (size_t) FILE_LIST_MAX); ++i) {
        FileMatch *file = &(flist->files[i]);
        if (file->info.type == kFileTypeDirectory) continue;
        struct tm *tm_info;
        tm_info = localtime(&(file->info.mtime));
        if (compact) {
            strftime(time_buf, 32, "%d/%m/%y %H:%M", tm_info);
            char size_buf[32];
            if (file->info.size >= 1024 * 1024 * 1024) {
                (void) snprintf(size_buf, 32, "%.1fG",
                                (double) file->info.size / (1024.0 * 1024.0 * 1024.0));
            } else if (file->info.size >= 1024 * 1024) {
                (void) snprintf(size_buf, 32, "%.1fM",
                                (double) file->info.size / (1024.0 * 1024.0));
            } else if (file->info.size >= 1024) {
                (void) snprintf(size_buf, 32, "%.1fK",
                                (double) file->info.size / 1024.0);
            } else {
                (void) snprintf(size_buf, 32, "%ld ", file->info.size);
            }
            (void) snprintf(buf, STRINGSIZE, "%s %6s %s", time_buf, size_buf,
                            file->name);
        } else {
            strftime(time_buf, 32, "%d/%m/%Y  %H:%M:%S", tm_info);
            (void) snprintf(buf, STRINGSIZE, "%s  %8ld  %s", time_buf, file->info.size,
                            file->name);
        }
        display_puts(buf);
        file_count++;
        // TODO: See above
        list_count += ((strlen(buf) + mmb_options.width - 1) / mmb_options.width) - 1;
        ListNewLine(&list_count, 0);
    }

    if (flist->count > FILE_LIST_MAX) {
        (void) snprintf(buf, STRINGSIZE, "WARNING! too many files (> %d) to list", FILE_LIST_MAX);
        display_puts(buf);
        ListNewLine(&list_count, 0);
    }

    if (flist->buf_full) {
        display_puts("WARNING! filename buffer overrun");
        ListNewLine(&list_count, 0);
    }

    // Print summary
    uint64_t mb_free;
    ON_FAILURE_ERROR(file_get_free_space(flist->directory, &mb_free));
    mb_free /= (1024 * 1024);
    if (dir_count == 1 && file_count == 1) {
        (void) snprintf(buf, STRINGSIZE, "%ld directory, %ld file, %ld MB free",
                        dir_count, file_count, mb_free);
    } else if (dir_count == 1) {
        (void) snprintf(buf, STRINGSIZE, "%ld directory, %ld files, %ld MB free",
                        dir_count, file_count, mb_free);
    } else if (file_count == 1) {
        (void) snprintf(buf, STRINGSIZE, "%ld directories, %ld file, %ld MB free",
                        dir_count, file_count, mb_free);
    } else {
        (void) snprintf(buf, STRINGSIZE, "%ld directories, %ld files, %ld MB free",
                        dir_count, file_count, mb_free);
    }
    display_puts(buf);
    ListNewLine(&list_count, 0);
}

/** FILES [fspec$] [, sort] */
void cmd_files(void) {
    cmd_files_internal(cmdline);
}
