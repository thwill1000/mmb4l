#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include "mmb4l.h"
#include "console.h"
#include "error.h"
#include "file.h"
#include "path.h"
#include "serial.h"
#include "utility.h"

// We don't use the 0'th entry, but it makes things simpler since MMBasic
// indexes file numbers from 1.
FileEntry file_table[MAXOPENFILES + 1] = { 0 };

/**
 * @param  fname  filename in C-string style, not MMBasic style.
 */
void file_open(char *fname, char *mode, int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) ERROR_INVALID_FILE_NUMBER;
    if (file_table[fnbr].type != fet_closed) ERROR_ALREADY_OPEN;

    char path[STRINGSIZE];
    path_munge(fname, path, STRINGSIZE);
    error_check();

    // random writing is not allowed when a file is opened for append so open it
    // first for read+update and if that does not work open it for
    // writing+update.  This has the same effect as opening for append+update
    // but will allow writing
    FILE *f = NULL;
    if (*mode == 'x') {
        errno = 0;
        f = fopen(path, "rb+");
        if (!f) {
            errno = 0;
            f = fopen(path, "wb+");
            if (!f) error_system(errno);
        }
        errno = 0;
        if FAILED(fseek(f, 0, SEEK_END)) error_system(errno);
    } else {
        errno = 0;
        f = fopen(path, mode);
        if (!f) error_system(errno);
    }

    file_table[fnbr].type = fet_file;
    file_table[fnbr].file_ptr = f;
}

void file_close(int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) ERROR_INVALID_FILE_NUMBER;

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
            serial_close(fnbr);
            break;
    }
}

void file_close_all(void) {
    for (int fnbr = 1; fnbr <= MAXOPENFILES; fnbr++) {
        if (file_table[fnbr].type != fet_closed) file_close(fnbr);
    }
}

int file_getc(int fnbr) {
    if (fnbr < 0 || fnbr > MAXOPENFILES) ERROR_INVALID_FILE_NUMBER;
    if (fnbr == 0) return MMgetchar();

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
            return (int) ch;
        }

        case fet_serial:
            return serial_getc(fnbr);
    }

    ERROR_INTERNAL_FAULT;
    return -1;
}

int file_loc(int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) ERROR_INVALID_FILE_NUMBER;

    int result = -1;
    switch (file_table[fnbr].type) {
        case fet_closed:
            ERROR_NOT_OPEN;
            break;

        case fet_file:
            errno = 0;
            result = ftell(file_table[fnbr].file_ptr) + 1;
            error_check();
            break;

        case fet_serial:
            return serial_rx_queue_size(fnbr);
            break;
    }

    return result;
}

int file_lof(int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) ERROR_INVALID_FILE_NUMBER;

    int result = -1;
    switch (file_table[fnbr].type) {
        case fet_closed:
            ERROR_NOT_OPEN;
            break;

        case fet_file: {
            errno = 0;
            FILE *f = file_table[fnbr].file_ptr;
            int pos = ftell(f);
            error_check();
            fseek(f, 0L, SEEK_END);
            error_check();
            result = ftell(f);
            error_check();
            fseek(f, pos, SEEK_SET);
            error_check();
            break;
        }

        case fet_serial:
            result = 0; // Serial I/O ports are unbuffered.
            break;
    }

    return result;
}

int file_putc(int ch, int fnbr) {
    if (fnbr < 0 || fnbr > MAXOPENFILES) ERROR_INVALID_FILE_NUMBER;
    if (fnbr == 0) return console_putc(ch);

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
            // TODO: Do I really want to be flushing every character ?
            fflush(file_table[fnbr].file_ptr); // Can this fail ?
            return (int) ch;
        }

        case fet_serial:
            return serial_putc(ch, fnbr);
    }

    ERROR_INTERNAL_FAULT;
    return -1;
}

int file_eof(int fnbr) {
    if (fnbr < 0 || fnbr > MAXOPENFILES) ERROR_INVALID_FILE_NUMBER;
    if (fnbr == 0) return 0;

    switch (file_table[fnbr].type) {
        case fet_closed:
            ERROR_NOT_OPEN;
            break;

        case fet_file: {
            FILE *f = file_table[fnbr].file_ptr;
            errno = 0;
            int ch = fgetc(f);  // the Watcom compiler will only set eof after
                                 // it has tried to read beyond the end of file
            int i = (feof(f) != 0) ? 1 : 0;
            error_check();
            ungetc(ch, f);  // undo the Watcom bug fix
            error_check();
            return i;
        }

        case fet_serial:
            return serial_eof(fnbr);
    }

    ERROR_INTERNAL_FAULT;
    return 1;
}

void file_seek(int fnbr, int idx) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) ERROR_INVALID_FILE_NUMBER;
    if (idx < 1) ERROR_INVALID("seek position");

    if (file_table[fnbr].type == fet_closed) ERROR_NOT_OPEN;
    FILE *f = file_table[fnbr].file_ptr;

    fflush(f);
    fsync(fileno(f));
    fseek(f, idx - 1, SEEK_SET); // MMBasic indexes from 1, not 0.
    error_check();
}

int file_find_free(void) {
    for (int fnbr = 1; fnbr <= MAXOPENFILES; fnbr++) {
        if (file_table[fnbr].type == fet_closed) return fnbr;
    }
    error("Too many open files");
    return -1;
}
