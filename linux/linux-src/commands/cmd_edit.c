#include <assert.h>
#include <sys/stat.h>

#include "../common/utility.h"
#include "../common/version.h"

int program_load_file(char *filename); // program.c

extern char CurrentFile[STRINGSIZE];
extern char error_file[STRINGSIZE];
extern int error_line;

static int run_editor(char *file_path, int line) {
    // char *mmeditor = getenv("MMEDITOR");

    char command[STRINGSIZE * 2];
    if (line > 0) {
        snprintf(command, STRINGSIZE * 2, "nano -R +%d \"%s\"", line, file_path);
    } else {
        snprintf(command, STRINGSIZE * 2, "nano -R \"%s\"", file_path);
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
    struct stat st;
    stat(file_path, &st);
    if (st.st_size == 0) {
        if (remove(file_path) != 0) return false;
    }
    return true;
}

static int file_exists(char *file_path) {
    errno = 0;
    FILE *f = fopen(file_path, "r");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
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
    if (!canonicalize_path(fname, file_path, STRINGSIZE)) {
        MMerrno = errno;
        errno = 0; // Is this necessary ?
        switch (MMerrno) {
            case ENOENT:
                new_file = true;
                break;
            case ENAMETOOLONG:
                error("Path too long");
                break;
            default:
                error(strerror(MMerrno));
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

    // If we don't have a current file then try and load the one we've just edited.
    if (*CurrentFile == '\0' && file_exists(file_path)) {
        program_load_file(file_path);
    }
}
