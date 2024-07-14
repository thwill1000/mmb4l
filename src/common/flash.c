/*-*****************************************************************************

MMBasic for Linux (MMB4L)

flash.c

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include "file.h"
#include "flash.h"
#include "utility.h"

#include <stdlib.h>

/** Is the 'flash' module initialised */
static bool flash_initialised = false;

static char* flash_slots[FLASH_NUM_SLOTS];

MmResult flash_init() {
    if (flash_initialised) return kOk;
    MmResult result = kOk;
    for (size_t i = 0; i < FLASH_NUM_SLOTS && SUCCEEDED(result); ++i) {
        flash_slots[i] = calloc(FLASH_SLOT_SIZE, 1);
        if (!flash_slots[i]) result = kOutOfMemory;
    }
    if (SUCCEEDED(result)) {
        flash_initialised = true;
    } else {
        for (size_t i = 0; i < FLASH_NUM_SLOTS; ++i) {
            free(flash_slots[i]);
        }
    }
    return result;
}

MmResult flash_term() {
    if (!flash_initialised) return kOk;
    for (size_t i = 0; i < FLASH_NUM_SLOTS; ++i) {
        free(flash_slots[i]);
    }
    flash_initialised = false;
    return kOk;
}

MmResult flash_get_addr(unsigned index, char **addr) {
    if (!flash_initialised) return kFlashModuleNotInitialised;
    if (index >= FLASH_NUM_SLOTS) return kFlashInvalidIndex;
    *addr = flash_slots[index];
    return kOk;
}

MmResult flash_disk_load(unsigned index, const char *filename, bool overwrite) {
    // TODO overwrite / already programmed.
    if (!flash_initialised) return kFlashModuleNotInitialised;
    if (index >= FLASH_NUM_SLOTS) return kFlashInvalidIndex;
    int fnbr = file_find_free();
    MmResult result = file_open(filename, "rb", fnbr);
    int size = -1;
    if (SUCCEEDED(result)) {
        size = file_lof(fnbr);
        if (size <= 0 || size > FLASH_SLOT_SIZE) result = kFlashFileTooBig;
    }
    if (SUCCEEDED(result)) {
        size_t count = file_read(fnbr, flash_slots[index], size);
        if (count == (size_t) size) {
            // Pad with 0xFF.
            for (size_t i = count; i < FLASH_SLOT_SIZE; ++i) flash_slots[index][i] = 0xFF;
        } else {
            result = kInternalFault;
        }
    }
    if (FAILED(result)) {
        (void) file_close(fnbr);
    } else {
        result = file_close(fnbr);
    }
    return result;
}
