#include <assert.h>
#include <sys/stat.h>

#include "../common/error.h"
#include "../common/program.h"
#include "../common/utility.h"
#include "../common/version.h"

static void get_mmbasic_nanorc(char *path) {
    *path = '\0';
    char *home = getenv("HOME");
    if (!home) return;
    sprintf(path, "%s/.mmbasic/mmbasic.nanorc", home);
    if (!file_exists(path)) {
        *path = '\0';
    }
}

static int run_editor(char *file_path, int line) {
     // char *mmeditor = getenv("MMEDITOR");

    char nanorc[STRINGSIZE];
    get_mmbasic_nanorc(nanorc);

    char command[STRINGSIZE * 2];
    if (*nanorc == '\0') {
        int ret = snprintf(
                command,
                STRINGSIZE * 2,
                "nano +%d \"%s\"", line > 0 ? line : 1,
                file_path);
        if (ret < 0) abort();
    } else {
        // Note early values or nano, such as the default version for Raspbian
        // do not support the --rcfile flag.
        int ret = snprintf(
                command,
                STRINGSIZE * 2,
                "nano --rcfile=%s +%d \"%s\"",
                nanorc,
                line > 0 ? line : 1,
                file_path);
        if (ret < 0) abort();
    }

    errno = 0;
    return system(command) == 0;
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
    if (file_exists(file_path) && file_is_empty(file_path)) {
        return remove(file_path) == 0;
    }
    return 1;
}

void cmd_edit(void) {
    getargs(&cmdline, 1, ",");

    if (CurrentLinePtr) error("Invalid in a program");

    char fname[STRINGSIZE] = { 0 };
    int current = false;
    int line = -1;
    if (argc == 1) {
        if (checkstring(argv[0], "CURRENT")) {
            current = true;
        } else {
            strcpy(fname, getCstring(argv[0]));
        }
    }

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
    if (!canonicalize_path(fname, file_path, STRINGSIZE)) {
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
    if (!run_editor(file_path, line)) error("Editor could not be run");

    // If we created a new file and it is still empty then delete it.
    if (new_file) {
        if (!delete_if_empty(file_path)) {
            error("Temporary file could not be deleted");
        }
    }

    // If we have just edited a .BAS file then load it.
    if (file_exists(file_path)
            && file_is_regular(file_path)
            && file_has_extension(file_path, ".BAS", true)) {
        // Never expected to return failure - reports its own ERROR and calls longjmp().
        if (FAILED(program_load_file(file_path))) ERROR_INTERNAL_FAULT;
    }
}
