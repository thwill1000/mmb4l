#include <stdbool.h>

#include "../common/console.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"
#include "../common/program.h"
#include "../common/utility.h"
#include "../common/version.h"

/** Gets the path of the file to write to. */
static void cmd_autosave_get_file_path(char *file_path) {
    char *filename = getCstring(cmdline);
    errno = 0;
    munge_path(filename, file_path, STRINGSIZE);
    error_check();

    if (strlen(file_get_extension(file_path)) == 0) {
        if (strlen(file_path) > MAXSTRLEN - 4) {
            error_code(ENAMETOOLONG, "Path too long");
        }
        strcat(file_path, ".bas");
    }
}

/** Reads input from the console into the buffer. */
static int cmd_autosave_read(char *buf) {
    int ch;
    int count = 0;
    char *p = buf;
    char previous = '\0';

    for (;;) {
        ch = MMInkey();

        switch (ch) {
            case -1:
                continue;
            case F1:
            case F2:
            case F6:
                goto cmd_autosave_read_exit;
            case TAB:
                ch = ' ';
                break;
            case '\n':
                if (p == buf) continue;  // Throw away an initial line feed
                                         // which can follow the command.
                break;

        }

        if ((p - buf) >= EDIT_BUFFER_SIZE) ERROR_OUT_OF_MEMORY;

        if ((isprint(ch) && (previous == '\r'))
                || (ch == '\r' && previous == '\r')
                || (ch == '\n')) {
            *p++ = '\n';
            count = 0;
            console_putc('\n');
        }

        if (isprint(ch)) {
            *p++ = ch;
            if (count++ > 240) ERROR_LINE_LENGTH;
            console_putc(ch);
        }

        previous = ch;
    }

cmd_autosave_read_exit:

    if (previous == '\r') *p++ = '\n';
    *p = '\0'; // Terminate with a NULL.

    if (MMCharPos > 1) console_putc('\n');

    return ch;
}

/** Writes out the file. */
static void cmd_autosave_write_file(char *file_path, char *buf) {
    int fnbr = file_find_free();
    file_open(file_path, "wb", fnbr);
    char *p = buf;
    while (*p) {
        file_putc(*p++, fnbr);
    }
    file_close(fnbr);
}

void cmd_autosave(void) {
    if (CurrentLinePtr) ERROR_INVALID_IN_PROGRAM;

    char file_path[STRINGSIZE]; // Don't use GetTempStrMemory() because it will
                                // be cleared when we call ClearProgram() later.
    cmd_autosave_get_file_path(file_path);
    ClearProgram();             // Clear leftovers from the previous program.
    char *buf = GetTempMemory(EDIT_BUFFER_SIZE);
    int exit_key = cmd_autosave_read(buf);
    cmd_autosave_write_file(file_path, buf);

    if (file_has_suffix(file_path, ".bas", true)) {
        program_load_file(file_path);
        if (exit_key == F2) {
            strcpy(inpbuf, "RUN\n");
            tokenise(true);
            ExecuteProgram(tknbuf);
        }
    }
}
