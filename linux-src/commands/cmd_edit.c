#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "../common/mmb4l.h"
#include "../common/cstring.h"
#include "../common/error.h"
#include "../common/path.h"
#include "../common/program.h"
#include "../common/utility.h"

static void get_mmbasic_nanorc(char *path) {
    *path = '\0';
    char *home = getenv("HOME");
    if (!home) return;
    sprintf(path, "%s/.mmbasic/mmbasic.nanorc", home);
    if (!path_exists(path)) {
        *path = '\0';
    }
}

static int get_editor_command(char *file_path, int line, char *command, bool *blocking) {
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
    cstring_replace(command, "${file}", replacement);
    snprintf(replacement, STRINGSIZE + 2, "%d", line);
    cstring_replace(command, "${line}", replacement);

    return 0;
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

    if (CurrentLinePtr) error("Invalid in a program");

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
        if (!current && *error_file != '\0') {
            strcpy(fname, error_file);
            line = error_line;
        } else if (*CurrentFile == '\0') {
            error("Nothing to edit");
        } else {
            strcpy(fname, CurrentFile);
        }
    }

    int new_file = false;
    char file_path[STRINGSIZE];
    errno = 0;
    if (!path_get_canonical(fname, file_path, STRINGSIZE)) {
        switch (errno) {
            case ENOENT:
                new_file = true;
                break;
            case ENAMETOOLONG:
                error_code(errno, "Path too long");
                break;
            default:
                error_system(errno);
                break;
        }
    }

    // If necessary create a new file.
    if (new_file) {
        if (!create_empty_file(file_path)) {
            error("File could not be created");
        }
    }

    // Edit the file.
    char command[STRINGSIZE * 2] = { 0 };
    bool blocking = false;
    if (FAILED(get_editor_command(file_path, line > 1 ? line : 1, command, &blocking))) {
        error("Unknown editor '$'", mmb_options.editor);
    }
    errno = 0;
    if (FAILED(system(command))) error("Editor could not be run");

    // If we created a new file and it is still empty after editing with an
    // editor that blocks then delete it.
    if (new_file && blocking) {
        if (!delete_if_empty(file_path)) {
            error("Temporary file could not be deleted");
        }
    }

    // If we have just edited a .bas file then load it.
    if (path_exists(file_path)
            && path_is_regular(file_path)
            && path_has_suffix(file_path, ".bas", true)) {
        // Never expected to return failure - reports its own ERROR and calls longjmp().
        if (FAILED(program_load_file(file_path))) ERROR_INTERNAL_FAULT;
    }
}
