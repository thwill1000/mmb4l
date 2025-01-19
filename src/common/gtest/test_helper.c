/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "test_helper.h"
#include "stubs/error_stubs.h"
#include "../memory.h"
#include "../../core/MMBasic.h"

#include <string.h>

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
