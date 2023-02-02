/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_poke.c

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

#include <assert.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/utility.h"

#define ERROR_ADDRESS_NOT_DIVISIBLE_BY(i)  error_throw_ex(kError, "Address not divisible by %", i)

/** POKE BYTE addr%, byte% */
static void poke_byte(int argc, char** argv, const char *p) {
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    uintptr_t addr = get_poke_addr(p);
    int64_t value = getinteger(argv[2]);

    *((uint8_t *) addr) = (uint8_t) value;
}

/** POKE DATAPTR ptr% */
static void poke_dataptr(int argc, char** argv, const char *p) {
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    assert(sizeof(DataReadPointer) == sizeof(MMINTEGER));
    MMINTEGER i = getinteger(p);
    DataReadPointer *pdrp = (DataReadPointer *) &i;
    NextDataLine = ProgMemory + pdrp->next_line_offset;
    NextData = pdrp->next_data;
}

/** POKE FLOAT addr%, float! */
static void poke_float(int argc, char** argv, const char *p) {
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    uintptr_t addr = get_poke_addr(p);
    if (addr % 8) ERROR_ADDRESS_NOT_DIVISIBLE_BY(8);
    MMFLOAT value = getnumber(argv[2]);

    *((MMFLOAT *) addr) = value;
}

/** POKE INTEGER addr%, integer% */
static void poke_integer(int argc, char** argv, const char *p) {
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    uintptr_t addr = get_poke_addr(p);
    if (addr % 8) ERROR_ADDRESS_NOT_DIVISIBLE_BY(8);
    int64_t value = getinteger(argv[2]);

    *((uint64_t *) addr) = (uint64_t) value;
}

/** POKE SHORT addr%, short% */
static void poke_short(int argc, char** argv, const char *p) {
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    uintptr_t addr = get_poke_addr(p);
    if (addr % 2) ERROR_ADDRESS_NOT_DIVISIBLE_BY(2);
    int64_t value = getinteger(argv[2]);

    *((uint16_t *) addr) = (uint16_t) value;
}

/** POKE VAR var, offset%, byte% */
static void poke_var(int argc, char** argv, const char *p) {
    if (argc != 5) ERROR_ARGUMENT_COUNT;

    void *pvar = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);

    if (vartbl[VarIndex].type & T_CONST) ERROR_CANNOT_CHANGE_A_CONSTANT;

    int64_t offset = getinteger(argv[2]);
    int64_t value = getinteger(argv[4]);

    *((char *)pvar + offset) = (uint8_t) value;
}

/** POKE VARTBL, offset%, byte% */
static void poke_vartbl(int argc, char** argv, const char *p) {
    if (argc != 5) ERROR_ARGUMENT_COUNT;

    int64_t offset = getinteger(argv[2]);
    int64_t value = getinteger(argv[4]);

    *((char *) vartbl + offset) = (uint8_t) value;
}

/** POKE WORD addr%, word% */
static void poke_word(int argc, char** argv, const char *p) {
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    uintptr_t addr = get_poke_addr(p);
    if (addr % 4) ERROR_ADDRESS_NOT_DIVISIBLE_BY(4);
    int32_t value = getinteger(argv[2]);

    *((uint32_t *) addr) = (uint32_t) value;
}

void cmd_poke(void) {
    getargs(&cmdline, 5, ",");

    const char* p;
    if ((p = checkstring(argv[0], "BYTE"))) {
        poke_byte(argc, argv, p);
    } else if ((p = checkstring(argv[0], "DATAPTR"))) {
        poke_dataptr(argc, argv, p);
    } else if ((p = checkstring(argv[0], "FLOAT"))) {
        poke_float(argc, argv, p);
    } else if ((p = checkstring(argv[0], "INTEGER"))) {
        poke_integer(argc, argv, p);
    } else if ((p = checkstring(argv[0], "SHORT"))) {
        poke_short(argc, argv, p);
    } else if ((p = checkstring(argv[0], "VAR"))) {
        poke_var(argc, argv, p);
    } else if ((p = checkstring(argv[0], "VARTBL"))) {
        poke_vartbl(argc, argv, p);
    } else if ((p = checkstring(argv[0], "WORD"))) {
        poke_word(argc, argv, p);
    } else {
        ERROR_SYNTAX;
    }
}
