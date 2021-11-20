#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "error.h"
#include "file.h"
#include "utility.h"
#include "version.h"

FileEntry file_table[MAXOPENFILES] = { 0 };

/**
 * @param  fname  filename in C-string style, not MMBasic style.
 */
void MMfopen(char *fname, char *mode, int fnbr) {
    if (fnbr < 1 || fnbr > 10) ERROR_INVALID("file number");
    fnbr--;
    if (file_table[fnbr].type != fet_closed) {
        error("File or device already open");
    }

    char path[STRINGSIZE];
    munge_path(fname, path, STRINGSIZE);
    error_check();

    // random writing is not allowed when a file is opened for append so open it
    // first for read+update and if that does not work open it for
    // writing+update.  This has the same effect as opening for append+update
    // but will allow writing
    FILE *f = NULL;
    if (*mode == 'x') {
        f = fopen(path, "rb+");
        if (!f) {
            f = fopen(path, "wb+");
            error_check();
        }
        fseek(f, 0, SEEK_END);
        error_check();
    } else {
        f = fopen(path, mode);
        error_check();
    }

    if (!f) {
        errno = EBADF;
        error_check();
    }

    file_table[fnbr].type = fet_file;
    file_table[fnbr].file_ptr = f;
}

void MMfclose(int fnbr) {
    if (fnbr < 1 || fnbr > 10) ERROR_INVALID("file number");
    fnbr--;

    switch (file_table[fnbr].type) {
        case fet_closed:
            ERROR_NOT_OPEN;
            break;

        case fet_file:
            errno = 0;
            fclose(file_table[fnbr].file_ptr);
            file_table[fnbr].type = fet_closed;
            file_table[fnbr].file_ptr = NULL;
            error_check();
            break;

        case fet_serial:
            // SerialClose(MMComPtr[fnbr]);
            file_table[fnbr].type = fet_closed;
            file_table[fnbr].serial_fd = -1;
            break;
    }
}

void CloseAllFiles(void) {
    for (int fnbr = 0; fnbr < MAXOPENFILES; fnbr++) {
        switch (file_table[fnbr].type) {
            case fet_closed:
                // Do nothing.
                break;

            case fet_file:
                fclose(file_table[fnbr].file_ptr);
                file_table[fnbr].type = fet_closed;
                file_table[fnbr].file_ptr = NULL;
                break;

            case fet_serial:
                // SerialClose(MMComPtr[fnbr]);
                file_table[fnbr].type = fet_closed;
                file_table[fnbr].serial_fd = -1;
                break;
        }
    }
}

int MMfgetc(int fnbr) {
    if (fnbr < 0 || fnbr > 10) ERROR_INVALID("file number");
    if (fnbr == 0) return MMgetchar();
    fnbr--;

    switch (file_table[fnbr].type) {
        case fet_closed:
            ERROR_NOT_OPEN;
            break;

        case fet_file: {
            errno = 0;
            char ch;
            if (fread(&ch, 1, 1, file_table[fnbr].file_ptr) == 0) {
                ch = -1;
            }
            error_check();
            return ch;
        }

        case fet_serial:
            // return Serialgetc(MMComPtr[fnbr]);
            break;
    }

    assert(false);
    return -1;
}

char MMfputc(char ch, int fnbr) {
    if (fnbr < 0 || fnbr > 10) ERROR_INVALID("file number");
    if (fnbr == 0) return MMputchar(ch);
    fnbr--;

    switch (file_table[fnbr].type) {
        case fet_closed:
            ERROR_NOT_OPEN;
            break;

        case fet_file: {
            errno = 0;
            if (fwrite(&ch, 1, 1, file_table[fnbr].file_ptr) == 0) {
                if (errno == 0) errno = EBADF;
            }
            error_check();
            return ch;
        }

        case fet_serial:
            // return Serialputc(ch, MMComPtr[fnbr]);
            break;
    }

    assert(false);
    return -1;
}

int MMfeof(int fnbr) {
    if (fnbr < 0 || fnbr > 10) ERROR_INVALID("file number");
    if (fnbr == 0) return 0;
    fnbr--;

    switch (file_table[fnbr].type) {
        case fet_closed:
            ERROR_NOT_OPEN;
            break;

        case fet_file: {
            FILE *f = file_table[fnbr].file_ptr;
            errno = 0;
            int ch = fgetc(f);  // the Watcom compiler will only set eof after
                                 // it has tried to read beyond the end of file
            int i = (feof(f) != 0) ? -1 : 0;
            error_check();
            ungetc(ch, f);  // undo the Watcom bug fix
            error_check();
            return i;
        }

        case fet_serial:
            // return SerialEOF(MMComPtr[fnbr]);
            break;
    }

    assert(false);
    return -1;
}

/** Find the first available free file number. */
int FindFreeFileNbr(void) {
    for (int fnbr = 0; fnbr < MAXOPENFILES; fnbr++) {
        if (file_table[fnbr].type == fet_closed) return fnbr + 1;
    }
    error("Too many files open");
    return -1;
}

bool file_exists(const char *path) {
    struct stat st;
    errno = 0;
    return stat(path, &st) == 0;
}

bool file_is_empty(const char *path) {
    struct stat st;
    errno = 0;
    stat(path, &st);
    return st.st_size == 0;
}

bool file_is_regular(const char *path) {
    struct stat st;
    errno = 0;
    return (stat(path, &st) == 0) && S_ISREG(st.st_mode) ? true : false;
}

const char *file_get_extension(const char *path) {
    char *p = strrchr(path, '.');
    return p ? p : path + strlen(path);
}

bool file_has_suffix(
        const char *path, const char *suffix, bool case_insensitive) {
    int start = strlen(path) - strlen(suffix);
    if (start < 0) return 0;
    for (int i = 0; i < strlen(suffix); ++i) {
        if (case_insensitive) {
            if (toupper(path[i + start]) != toupper(suffix[i])) return false;
        } else {
            if (path[i + start] != suffix[i]) return false;
        }
    }
    return true;
}
