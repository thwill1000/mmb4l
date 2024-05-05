/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "../error.h"
#include "../memory.h"
#include "../mmresult.h"
#include "../../Configuration.h"
#include "../../core/MMBasic.h"
#include "test_helper.h"

#include <string.h>

char error_msg[256];

// Defined in "common/error.c"
void error_init(ErrorState *error_state) { }

MmResult error_throw(MmResult result) {
    return error_throw_ex(result, mmresult_to_string(result));
}

MmResult error_throw_ex(MmResult result, const char *msg, ...) {
    strcpy(error_msg, msg);
    return result;
}

MmResult error_throw_legacy(const char *msg, ...) {
    strcpy(error_msg, msg);
    return kOk;
}

void clear_prog_memory() {
    ProgMemory[0] = '\0'; // Program ends "\0\0\xFF".
    ProgMemory[1] = '\0';
    ProgMemory[2] = '\xFF';
}

// TODO: can I just use program_tokenise() instead ?
void tokenise_and_append(const char* untokenised) {
    strcpy(inpbuf, untokenised);

    tokenise(0);
    if (error_msg[0]) return;

    // Current end of ProgMemory should be "\0\0".
    char *pmem = ProgMemory;
    int count = 0;
    for (; count != 2; ++pmem) {
        count = (*pmem  == '\0') ? count + 1 : 0;
    }
    pmem -= 1; // We leave one '\0' between each command.

    if (pmem == ProgMemory + 1) pmem = ProgMemory;

    // Do not just do a strcpy() from the tknbuf as it may contain embedded \0.
    // The actual end is when there are two consecutive \0.
    for (const char *pbuf = tknbuf; pbuf[0] || pbuf[1]; pmem++, pbuf++) {
        *pmem = *pbuf;
    }
    *pmem++ = '\0';
    *pmem++ = '\0';
}

const char *audio_last_error() { return ""; }

const char *events_last_error() { return ""; }

const char *graphics_last_error() { return ""; }
