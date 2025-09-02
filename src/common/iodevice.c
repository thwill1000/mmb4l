/*-*****************************************************************************

MMBasic for Linux (MMB4L)

iodevice.c

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

#include <assert.h>

#include "error.h"
#include "file_private.h"
#include "iodevice.h"
#include "serial.h"
#include "utility.h"

MmResult iodevice_close(int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) return kFileInvalidFileNumber;

    switch (file_table[fnbr].type) {
        case fet_closed:
            return kFileNotOpen;

        case fet_file: {
            errno = 0;
            int result = fclose(file_table[fnbr].file_ptr);
            file_table[fnbr].type = fet_closed;
            file_table[fnbr].file_ptr = NULL;
            if (FAILED(result)) return errno;
            break;
        }

        case fet_serial:
            return serial_close(fnbr);
    }

    return kOk;
}

void iodevice_close_all(void) {
    for (int fnbr = 1; fnbr <= MAXOPENFILES; fnbr++) {
        if (file_table[fnbr].type != fet_closed) (void) iodevice_close(fnbr);
    }
}

int iodevice_find_free(void) {
    for (int fnbr = 1; fnbr <= MAXOPENFILES; fnbr++) {
        if (file_table[fnbr].type == fet_closed) return fnbr;
    }
    ON_FAILURE_ERROR_EX(kTooManyOpenFiles, -1);
}

bool iodevice_is_file(int fnbr) {
    assert(fnbr >= 0 && fnbr <= MAXOPENFILES);
    if (fnbr >= 0 && fnbr <= MAXOPENFILES) {
        return file_table[fnbr].type == fet_file;
    } else {
        return false;
    }
}

bool iodevice_is_serial(int fnbr) {
    assert(fnbr >= 0 && fnbr <= MAXOPENFILES);
    if (fnbr >= 0 && fnbr <= MAXOPENFILES) {
        return file_table[fnbr].type == fet_serial;
    } else {
        return false;
    }
}

MmResult iodevice_open(const char *path, const char *mode, int fnbr) {
    if (fnbr < 1 || fnbr > MAXOPENFILES) return kFileInvalidFileNumber;
    if (file_table[fnbr].type != fet_closed) return kFileAlreadyOpen;

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
            if (!f) return errno;
        }
        errno = 0;
        if (FAILED(fseek(f, 0, SEEK_END))) return errno;
    } else {
        errno = 0;
        f = fopen(path, mode);
        if (!f) return errno;
    }

    file_table[fnbr].type = fet_file;
    file_table[fnbr].file_ptr = f;

    return kOk;
}
