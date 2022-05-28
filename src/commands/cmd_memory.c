/*-*****************************************************************************

MMBasic for Linux (MMB4L)

cmd_memory.c

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

#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/console.h"
#include "../common/error.h"
#include "../common/utility.h"

#define ERROR_ADDRESS_NOT_DIVISIBLE_BY(i)      error_throw_ex(kError, "Address not divisible by %", i)
#define ERROR_DST_ADDRESS_NOT_DIVISIBLE_BY(i)  error_throw_ex(kError, "Destination address not divisible by %", i)
#define ERROR_SRC_ADDRESS_NOT_DIVISIBLE_BY(i)  error_throw_ex(kError, "Source address not divisible by %", i)

static int64_t getint64(const char *p, int64_t min, int64_t max) {
    int64_t i = getinteger(p);
    if (i < min || i > max) {
        char buf[STRINGSIZE];
#if defined(ENV64BIT)
        sprintf(buf, "%ld is invalid (valid is %ld to %ld)", i, min, max);
#else
        sprintf(buf, "%lld is invalid (valid is %lld to %lld)", i, min, max);
#endif
        error_throw_ex(kError, buf);
    }
    return i;
}

// Data type sizes:
//   FLOAT   - 8 bytes
//   INTEGER - 8 bytes
//   SHORT   - 2 bytes
//   WORD    - 4 bytes

static void memory_copy_internal(const char *p, size_t element_size) {
    getargs(&p, 5, ",");
    if (argc != 5) ERROR_SYNTAX;
    uintptr_t src = get_poke_addr(argv[0]);
    if (src % element_size) ERROR_SRC_ADDRESS_NOT_DIVISIBLE_BY(element_size);
    uintptr_t dst = get_poke_addr(argv[2]);
    if (dst % element_size) ERROR_DST_ADDRESS_NOT_DIVISIBLE_BY(element_size);
    size_t num = (size_t) getint64(argv[4], 0, UINT32_MAX);
    memmove((void *) dst, (void *) src, num * element_size);
}

/** MEMORY COPY BYTE src, dst, number_of_bytes */
static void memory_copy_byte(const char *p) {
    memory_copy_internal(p, 1);
}

/** MEMORY COPY FLOAT src, dst, number_of_floats */
static void memory_copy_float(const char *p) {
    memory_copy_internal(p, 8);
}

/** MEMORY COPY INTEGER src, dst, number_of_integers */
static void memory_copy_integer(const char *p) {
    memory_copy_internal(p, 8);
}

/** MEMORY COPY SHORT src, dst, number_of_shorts */
static void memory_copy_short(const char *p) {
    memory_copy_internal(p, 2);
}

/** MEMORY COPY WORD src, dst, number_of_words */
static void memory_copy_word(const char *p) {
    memory_copy_internal(p, 4);
}

static void memory_copy(const char *p) {
    const char *p2;
    if ((p2 = checkstring(p, "BYTE"))) {
        memory_copy_byte(p2);
    } else if ((p2 = checkstring(p, "FLOAT"))) {
        memory_copy_float(p2);
    } else if ((p2 = checkstring(p, "INTEGER"))) {
        memory_copy_integer(p2);
    } else if ((p2 = checkstring(p, "SHORT"))) {
        memory_copy_short(p2);
    } else if ((p2 = checkstring(p, "WORD"))) {
        memory_copy_word(p2);
    } else {
        // Assume BYTE.
        memory_copy_byte(p);
    }
}

static void memory_set_internal(const char *p, size_t element_size, int64_t min, int64_t max) {
    getargs(&p, 5, ",");
    if (argc != 5) ERROR_SYNTAX;
    uintptr_t to = get_poke_addr(argv[0]);
    if ((uintptr_t) to % element_size) ERROR_ADDRESS_NOT_DIVISIBLE_BY(element_size);
    int64_t value = getint64(argv[2], min, max);
    int64_t num = getint64(argv[4], 0, UINT32_MAX);
    if (element_size == 1) {
        memset((void *) to, (int) value, (size_t) num);
    } else {
        for (int32_t i = 0; i < num; ++i) {
            memcpy((void *) (to + element_size * i), &value, element_size);
        }
    }
}

/** MEMORY SET BYTE address, byte_value, number_of_bytes */
static void memory_set_byte(const char *p) {
    memory_set_internal(p, 1, 0, 255);
}

/** MEMORY SET FLOAT address, float_value, number_of_floats */
static void memory_set_float(const char *p) {
    getargs(&p, 5, ",");
    if (argc != 5) ERROR_SYNTAX;
    uintptr_t to = get_poke_addr(argv[0]);
    if ((uintptr_t) to % 8) ERROR_ADDRESS_NOT_DIVISIBLE_BY(8);
    MMFLOAT value = getnumber(argv[2]);
    int64_t num = getint64(argv[4], 0, UINT32_MAX);
    for (int32_t i = 0; i < num; ++i) {
        memcpy((void *) (to + 8 * i), &value, 8);
    }
}

/** MEMORY SET INTEGER address, integer, number_of_integers */
static void memory_set_integer(const char *p) {
    memory_set_internal(p, 8, INT64_MIN, INT64_MAX);
}

/** MEMORY SET SHORT address, short_value, number_of_shorts */
static void memory_set_short(const char *p) {
    memory_set_internal(p, 2, 0, UINT16_MAX);
}

/** MEMORY SET WORD address, word_value, number_of_words */
static void memory_set_word(const char *p) {
    memory_set_internal(p, 4, 0, UINT32_MAX);
}

static void memory_set(const char *p) {
    const char *p2;
    if ((p2 = checkstring(p, "BYTE"))) {
        memory_set_byte(p2);
    } else if ((p2 = checkstring(p, "FLOAT"))) {
        memory_set_float(p2);
    } else if ((p2 = checkstring(p, "INTEGER"))) {
        memory_set_integer(p2);
    } else if ((p2 = checkstring(p, "SHORT"))) {
        memory_set_short(p2);
    } else if ((p2 = checkstring(p, "WORD"))) {
        memory_set_word(p2);
    } else {
        // Assume BYTE.
        memory_set_byte(p);
    }
}

static void count_program_size_and_lines(int *num_bytes, int *num_lines) {
    *num_bytes = 0;
    *num_lines = 0;

    const char *p = ProgMemory;
    while (1) {
        while (*p) {
            p++;
            (*num_bytes)++;
        }  // look for the zero marking the start of an element
        if (p[1] == 0) break;  // end of the program
        if (p[1] == T_LINENBR) {
            (*num_lines)++;
            p += 3;  // skip over the line number
            (*num_bytes) += 3;
        } else if (p[1] == T_NEWLINE) {
            (*num_lines)++;
            p += 1;  // skip over the new line token
            (*num_bytes) += 1;
        }
        p++;
        (*num_bytes)++;
        skipspace(p);
        if (p[0] == T_LABEL) {
            (*num_bytes) += p[1] + 2;
            p += p[1] + 2;  // skip over the label
        }
    }
}

static int count_variables() {
    int vcnt = 0;
    for (int i = 0; i < varcnt; i++) {
        if (vartbl[i].type != T_NOTYPE) vcnt++;
    }
    return vcnt;
}

static void memory_report(const char *unused) {

    int num_bytes = 0;
    int num_lines = 0;
    count_program_size_and_lines(&num_bytes, &num_lines);
    sprintf(
            inpbuf,
            "    Program:%4dK (%2d%%) used %3dK free (%d line%s)\r\n",
            (num_bytes + 512) / 1024,
            (num_bytes * 100) / PROG_FLASH_SIZE,
            (PROG_FLASH_SIZE - num_bytes + 512) / 1024,
            num_lines,
            num_lines == 1 ? "" : "s");
    console_puts(inpbuf);

    int32_t vcnt = count_variables();
    int32_t size = sizeof(struct s_vartbl);
    sprintf(
            inpbuf,
            "  Variables:%4dK (%2d%%) used %3dK free (%d variables)\r\n",
            (int32_t) ((vcnt * size + 512) / 1024),
            (int32_t) (vcnt * 100 / MAXVARS),
            (int32_t) (((MAXVARS * size + 512) / 1024) - ((vcnt * size + 512) / 1024)),
            vcnt);
    console_puts(inpbuf);

    int ram_used = (UsedHeap() + 512) / 1024;
    int percent_used = ((UsedHeap() + 512) * 100) / HEAP_SIZE;
    sprintf(
            inpbuf,
            "General RAM:%4dK (%2d%%) used %3dK free\r\n",
            ram_used,
            percent_used,
            (HEAP_SIZE / 1024) - ram_used);
    console_puts(inpbuf);
}

void cmd_memory(void) {
    // getargs(&cmdline, 10, ",");
    // for (int i = 0; i < argc; ++i) {
    //     printf("%d: %s, ", i, argv[i]);
    // }
    // printf("\n");
    const char *p;
    if ((p = checkstring(cmdline, "COPY"))) {
        memory_copy(p);
    } else if ((p = checkstring(cmdline, "SET"))) {
        memory_set(p);
    } else {
        memory_report(p);
    }
}
