#include <assert.h>
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
    if (!path_munge(fname, path, STRINGSIZE)) error_throw(errno);

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
            if (!f) error_throw(errno);
        }
        errno = 0;
        if FAILED(fseek(f, 0, SEEK_END)) error_throw(errno);
    } else {
        errno = 0;
        f = fopen(path, mode);
        if (!f) error_throw(errno);
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

        case fet_file: {
            errno = 0;
            int result = fclose(file_table[fnbr].file_ptr);
            file_table[fnbr].type = fet_closed;
            file_table[fnbr].file_ptr = NULL;
            if (FAILED(result)) error_throw(errno);
            break;
        }

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
                if (ferror(file_table[fnbr].file_ptr) == 0) {
                    ch = -1;
                } else {
                    error_throw(errno);
                }
            }
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

    switch (file_table[fnbr].type) {
        case fet_closed:
            ERROR_NOT_OPEN;
            assert(false);
            break;

        case fet_file:
            errno = 0;
            long int result = ftell(file_table[fnbr].file_ptr);
            if (result == -1L) error_throw(errno);
            return (int) (result + 1);
            break;

        case fet_serial:
            return serial_rx_queue_size(fnbr);
            break;
    }

    return -1;
}

int file_lof(int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) ERROR_INVALID_FILE_NUMBER;

    switch (file_table[fnbr].type) {
        case fet_closed:
            ERROR_NOT_OPEN;
            assert(false);
            break;

        case fet_file: {
            errno = 0;
            FILE *f = file_table[fnbr].file_ptr;
            long int current = ftell(f);
            if (current == -1L) error_throw(errno);
            if (FAILED(fseek(f, 0L, SEEK_END))) error_throw(errno);
            long int result = ftell(f);
            if (result == -1L) error_throw(errno);
            if (FAILED(fseek(f, current, SEEK_SET))) error_throw(errno);
            return result;
            break;
        }

        case fet_serial:
            return 0; // Serial I/O ports are unbuffered.
            break;
    }

    return -1;
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
                if (ferror(file_table[fnbr].file_ptr)) error_throw(errno);
                assert(false); // Always expect ferror to have been set.
            }
            // TODO: Do I really want to be flushing every character ?
            if (FAILED(fflush(file_table[fnbr].file_ptr))) error_throw(errno);
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
            int ch = fgetc(f); // Try to read beyond the end of the file.
            if (ch == EOF) {
                if (ferror(f)) error_throw(errno);
            } else {
                if (ungetc(ch, f) == EOF) error_throw(errno);
            }
            return ch == EOF;
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

    errno = 0;
    if (FAILED(fflush(f))) error_throw(errno);
    if (FAILED(fsync(fileno(f)))) error_throw(errno);
    if (FAILED(fseek(f, idx - 1, SEEK_SET))) error_throw(errno); // MMBasic indexes from 1, not 0.
}

int file_find_free(void) {
    for (int fnbr = 1; fnbr <= MAXOPENFILES; fnbr++) {
        if (file_table[fnbr].type == fet_closed) return fnbr;
    }
    error("Too many open files");
    return -1;
}
