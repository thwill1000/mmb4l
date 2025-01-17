/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_edit.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include "../common/mmb4l.h"
#include "../common/cstring.h"
#include "../common/path.h"
#include "../common/program.h"
#include "../common/utility.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define CMD_SIZE  (STRINGSIZE * 2)

#define ERROR_EDITOR_FAILED              error_throw_ex(kError, "Editor could not be run")
#define ERROR_FAILED_TO_DELETE_TMP_FILE  error_throw_ex(kError, "Temporary file could not be deleted")
#define ERROR_FILE_COULD_NOT_BE_CREATED  error_throw_ex(kError, "File could not be created")
#define ERROR_NOTHING_TO_EDIT            error_throw_ex(kError, "Nothing to edit")

static void get_mmbasic_nanorc(char *path) {
    *path = '\0';
    char *home = getenv("HOME");
    if (!home) return;
    sprintf(path, "%s/.mmbasic/mmbasic.nanorc", home);
    if (!path_exists(path)) {
        *path = '\0';
    }
}

static MmResult get_editor_command(const char *file_path, int line, char *command, bool *blocking) {
    *command = '\0';
    *blocking = false;
    for (const OptionsEditor *editor = options_editors; editor->name; ++editor) {
        if (strcasecmp(mmb_options.editor, editor->name) == 0) {
            strcpy(command, editor->command);
            *blocking = editor->blocking;
        }
    }

    // Special magic for Nano when we the 'mmbasic.nano.rc' file is installed.
    // Note early values or nano, such as the default version for Raspbian
    // do not support the --rcfile flag.
    if (strcasecmp(mmb_options.editor, "nano") == 0) {
        char nanorc[STRINGSIZE];
        get_mmbasic_nanorc(nanorc);
        if (*nanorc) sprintf(command, "nano --rcfile=%s +${line} ${file}", nanorc);
    }

    if (!*command) {
        // Manually specified editor.
        strcpy(command, mmb_options.editor);
    }

    char replacement[STRINGSIZE + 2];
    snprintf(replacement, STRINGSIZE + 2, "\"%s\"", file_path);
    if (FAILED(cstring_replace(command, CMD_SIZE, "${file}", replacement))) {
        return mmresult_ex(kInvalidEditor, "Editor ${file} replace failed: %s", mmb_options.editor);
    }
    snprintf(replacement, STRINGSIZE + 2, "%d", line);
    if (FAILED(cstring_replace(command, CMD_SIZE,  "${line}", replacement))) {
        return mmresult_ex(kInvalidEditor, "Editor ${line} replace failed: %s", mmb_options.editor);
    }

    return kOk;
}

static int create_empty_file(char *file_path) {
    errno = 0;
    FILE *f = fopen(file_path, "wb");
    if (!f) return false;
    fclose(f);
    return true;
}

static int delete_if_empty(char *file_path) {
    errno = 0;
    if (path_exists(file_path) && path_is_empty(file_path)) {
        return remove(file_path) == 0;
    }
    return 1;
}

void cmd_edit(void) {
    getargs(&cmdline, 1, ",");

    if (CurrentLinePtr) ERROR_INVALID_IN_PROGRAM;

    char fname[STRINGSIZE] = { 0 };
    int current = false;
    if (argc == 1) {
        if (checkstring(argv[0], "CURRENT")) {
            current = true;
        } else {
            strcpy(fname, getCstring(argv[0]));
        }
    }

    int line = 1;
    if (*fname == '\0') {
        if (!current && *mmb_error_state_ptr->file != '\0') {
            strcpy(fname, mmb_error_state_ptr->file);
            line = mmb_error_state_ptr->line;
        } else if (*CurrentFile == '\0') {
            ERROR_NOTHING_TO_EDIT;
        } else {
            strcpy(fname, CurrentFile);
        }
    }

    char file_path[STRINGSIZE];
    MmResult result = path_get_canonical(fname, file_path, STRINGSIZE);
    switch (result) {
        case kOk:
            // Nothing to see here, move along.
            break;
        case kFilenameTooLong:
            ERROR_PATH_TOO_LONG;
            break;
        default:
            error_throw(result);
            break;
    }

    // If necessary create a new file.
    bool new_file = !path_exists(file_path);
    if (new_file) {
        if (!create_empty_file(file_path)) ERROR_FILE_COULD_NOT_BE_CREATED;
    }

    // Edit the file.
    char command[CMD_SIZE] = { 0 };
    bool blocking = false;
    ON_FAILURE_ERROR(get_editor_command(file_path, line > 1 ? line : 1, command, &blocking));
    errno = 0;
    if (FAILED(system(command))) ERROR_EDITOR_FAILED;

    // If we created a new file and it is still empty after editing with an
    // editor that blocks then delete it.
    if (new_file && blocking) {
        if (!delete_if_empty(file_path)) {
            ERROR_FAILED_TO_DELETE_TMP_FILE;
        }
    }

    // If we have just edited a .bas file then load it.
    if (path_exists(file_path)
            && path_is_regular(file_path)
            && path_has_extension(file_path, ".bas", true)) {
        MmResult result = program_load_file(file_path);
        if (FAILED(result)) error_throw(result);
    }
}
