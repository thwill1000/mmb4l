/*-*****************************************************************************

MMBasic for Linux (MMB4L)

streamio.c

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
   be displayed on the console at startup (additional copyright messages may
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

#include <assert.h>

#include "error.h"
#include "file.h"
#include "file_private.h"
#include "logger.h"
#include "prompt.h"
#include "streamio.h"
#include "serial.h"
#include "utility.h"

MmResult (*streamio_0_flush_fn)() = NULL;
MmResult (*streamio_0_putc_fn)(char c) = NULL;
MmResult (*streamio_0_write_fn)(const char *buf, size_t *sz) = NULL;

MmResult streamio_init(MmResult (*flush_fn)(), MmResult (*putc_fn)(char), MmResult (*write_fn)(const char *, size_t *)) {
    streamio_0_flush_fn = flush_fn;
    streamio_0_putc_fn = putc_fn;
    streamio_0_write_fn = write_fn;
    return kOk;
}

MmResult streamio_close(int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) return kFileInvalidFileNumber;

    switch (file_table[fnbr].type) {
        case fet_closed:
            RETURN_RESULT(kFileNotOpen);

        case fet_file:
            RETURN_RESULT(file_close(fnbr));

        case fet_serial:
            RETURN_RESULT(serial_close(fnbr));

        default:
            RETURN_RESULT(INTERNAL_FAULT_EX("invalid file type: %d", file_table[fnbr].type));
    }
}

MmResult streamio_close_all(void) {
    for (int fnbr = 1; fnbr <= MAXOPENFILES; fnbr++) {
        if (file_table[fnbr].type != fet_closed) (void) streamio_close(fnbr);
    }
    return kOk;
}

int streamio_eof(int fnbr) {
    static const int error_result = 1; // To match other MMBasic platforms
    if (fnbr == 0) {
        return error_result;
    } else {
        ON_FAILURE_ERROR_EX(file_validate_fnbr(fnbr), error_result);
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            THROW_ERROR(kFileNotOpen, error_result);

        case fet_file:
            RETURN_INT(file_eof(fnbr));

        case fet_serial:
            RETURN_INT(serial_eof(fnbr));

        default:
            THROW_ERROR(INTERNAL_FAULT_EX("invalid file type: %d", file_table[fnbr].type), error_result);
    }
}

int streamio_find_free(void) {
    for (int fnbr = 1; fnbr <= MAXOPENFILES; fnbr++) {
        if (file_table[fnbr].type == fet_closed) return fnbr;
    }
    ON_FAILURE_ERROR_EX(kTooManyOpenFiles, -1);
    return -1;
}

MmResult streamio_flush(int fnbr) {
    if (fnbr == 0) {
        assert(streamio_0_flush_fn);
        RETURN_RESULT(streamio_0_flush_fn());
    } else {
        ON_FAILURE_RETURN(file_validate_fnbr(fnbr));
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            RETURN_RESULT(kFileNotOpen);

        case fet_file:
            RETURN_RESULT(file_flush(fnbr));

        case fet_serial:
            RETURN_RESULT(serial_flush(fnbr));

        default:
            RETURN_RESULT(INTERNAL_FAULT_EX("invalid file type: %d", file_table[fnbr].type));
    }
}

int streamio_getc(int fnbr) {
    if (fnbr < 0 || fnbr > MAXOPENFILES) {
        THROW_ERROR(kFileInvalidFileNumber, -1);
    }

    if (fnbr == 0) {
        int ch = -1;
        ON_FAILURE_ERROR_EX(prompt_getc(&ch), -1);
        RETURN_INT(ch);
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            THROW_ERROR(kFileNotOpen, -1);

        case fet_file:
            RETURN_INT(file_getc(fnbr));

        case fet_serial:
            RETURN_INT(serial_getc(fnbr));

        default:
            THROW_ERROR(INTERNAL_FAULT_EX("invalid file type: %d", file_table[fnbr].type), -1);
    }
}

bool streamio_is_file(int fnbr) {
    assert(fnbr >= 0 && fnbr <= MAXOPENFILES);
    if (fnbr >= 0 && fnbr <= MAXOPENFILES) {
        return file_table[fnbr].type == fet_file;
    } else {
        return false;
    }
}

bool streamio_is_serial(int fnbr) {
    assert(fnbr >= 0 && fnbr <= MAXOPENFILES);
    if (fnbr >= 0 && fnbr <= MAXOPENFILES) {
        return file_table[fnbr].type == fet_serial;
    } else {
        return false;
    }
}

int streamio_loc(int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) {
        ON_FAILURE_ERROR_EX(kFileInvalidFileNumber, -1);
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            ON_FAILURE_ERROR_EX(kFileNotOpen, -1);
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

int streamio_lof(int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) {
        ON_FAILURE_ERROR_EX(kFileInvalidFileNumber, -1);
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            ON_FAILURE_ERROR_EX(kFileNotOpen, -1);
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

MmResult streamio_open(const char *path, const char *mode, int fnbr) {
    // LOG_FN_ENTRY("path=\"%s\", mode=\"%s\", fnbr=%d", path, mode, fnbr);
    RETURN_RESULT(file_open(path, mode, fnbr));
}

int streamio_putc(int fnbr, int ch) {
    if (fnbr < 0 || fnbr > MAXOPENFILES) {
        THROW_ERROR(kFileInvalidFileNumber, -1);
    }

    if (fnbr == 0) {
        assert(streamio_0_putc_fn);
        ON_FAILURE_ERROR_EX(streamio_0_putc_fn(ch), -1);
        RETURN_INT(ch);
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            THROW_ERROR(kFileNotOpen, -1);

        case fet_file:
            RETURN_INT(file_putc(fnbr, (char)ch));

        case fet_serial:
            RETURN_INT(serial_putc(fnbr, ch));

        default:
            THROW_ERROR(INTERNAL_FAULT_EX("invalid file type: %d", file_table[fnbr].type), -1);
    }
}

size_t streamio_read(int fnbr, char *buf, size_t buf_sz) {
    CHECK_PARAM(fnbr != 0); // if (fnbr == 0) return console_write(buf, sz);
    if (fnbr < 1 || fnbr > MAXOPENFILES) {
        ON_FAILURE_ERROR_EX(kFileInvalidFileNumber, 0);
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            THROW_ERROR(kFileNotOpen, 0);
            break;

        case fet_file:
            RETURN_INT(file_read(fnbr, buf, buf_sz));

        case fet_serial:
            THROW_ERROR(INTERNAL_FAULT_EX("streamio_read() not implemented for serial ports"), 0);
            // return serial_read(fnbr, buf, sz);

        default:
            THROW_ERROR(INTERNAL_FAULT_EX("invalid file type: %d", file_table[fnbr].type), 0);
    }
}

MmResult streamio_readln(int fnbr, char *buf, size_t buf_sz) {
    CHECK_PARAM(buf != NULL);
    CHECK_PARAM(buf_sz > 0);

    if (fnbr < 0 || fnbr > MAXOPENFILES) {
        return kFileInvalidFileNumber;
    }
    if (file_table[fnbr].type == fet_closed) return kFileNotOpen;

    size_t pos = 0;
    int ch;
    buf_sz--;  // Reserve space for null terminator

    while (pos <= buf_sz) {
        ch = streamio_getc(fnbr);

        if (ch < 0) {
            if (pos > 0) break;
            if (streamio_eof(fnbr)) {
                buf[0] = '\0';
                return kOk;
            }
            return INTERNAL_FAULT_EX("streamio_getc() failed: %d", ch);
        }

        if (ch == '\n') {
            break;
        } else if (ch == '\r') {
            int next = streamio_getc(fnbr);
            if (next >= 0 && next != '\n') {
                (void) streamio_ungetc(fnbr, next);
            }
            break;
        }

        // Only store character if there's room
        if (pos < buf_sz) {
            buf[pos++] = (char)ch;
        } else {
            // Buffer full and this isn't a terminator
            (void) streamio_ungetc(fnbr, ch);  // Put it back!
            buf[pos] = '\0';
            return kStringTooLong;
        }
    }

    buf[pos] = '\0';
    return kOk;
}

void streamio_seek(int fnbr, int idx) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) {
        ON_FAILURE_ERROR(kFileInvalidFileNumber);
    }
    if (idx < 1) {
        ON_FAILURE_ERROR(kFileInvalidSeekPosition);
    }

    if (file_table[fnbr].type == fet_closed) {
        ON_FAILURE_ERROR(kFileNotOpen);
    }

    FILE *f = file_table[fnbr].file_ptr;

    errno = 0;
    if (FAILED(fflush(f))) error_throw(errno);
    if (FAILED(file_fsync(fileno(f)))) error_throw(errno);
    if (FAILED(fseek(f, idx - 1, SEEK_SET))) error_throw(errno); // MMBasic indexes from 1, not 0.
}

MmResult streamio_ungetc(int fnbr, int ch) {
    if (fnbr < 0 || fnbr > MAXOPENFILES) {
        return kFileInvalidFileNumber;
    }

    // Can't unget to console
    if (fnbr == 0) {
        return kFileInvalidOperation;
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            return kFileNotOpen;

        case fet_file:
            errno = 0;
            if (ungetc(ch, file_table[fnbr].file_ptr) == EOF) {
                if (errno) return errno;
                return INTERNAL_FAULT_EX("ungetc() failed");
            }
            return kOk;

        case fet_serial:
            // Serial ports typically don't support ungetc
            return kFileInvalidOperation;

        default:
            return INTERNAL_FAULT_EX("invalid file type: %d", file_table[fnbr].type);
    }
}

size_t streamio_write(int fnbr, const char *buf, size_t buf_sz) {
    // LOG_FN_ENTRY("fnbr=%d, buf=\"%s\", buf_sz=%d", fnbr, buf, buf_sz);

    if (fnbr == 0) {
        assert(streamio_0_write_fn);
        ON_FAILURE_ERROR_EX(streamio_0_write_fn(buf, &buf_sz), 0);
        return buf_sz;
    } else if (fnbr < 0 || fnbr > MAXOPENFILES) {
        THROW_ERROR(kFileInvalidFileNumber, 0);
    }

    switch (file_table[fnbr].type) {
        case fet_closed:
            THROW_ERROR(kFileNotOpen, 0);

        case fet_file:
            RETURN_INT(file_write(fnbr, buf, buf_sz));

        case fet_serial:
            RETURN_INT(serial_write(fnbr, buf, buf_sz));

        default:
            THROW_ERROR(INTERNAL_FAULT_EX("invalid file type: %d", file_table[fnbr].type), 0);
    }
}
