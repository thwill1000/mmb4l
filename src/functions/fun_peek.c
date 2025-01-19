/*-*****************************************************************************

MMBasic for Linux (MMB4L)

fun_peek.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/memory.h"
#include "../common/utility.h"
#include "../core/commandtbl.h"
#include "../core/funtbl.h"

#include <assert.h>

/** PEEK(BYTE addr%) */
static void peek_byte(int argc, char **argv, const char *p) {
    if (argc != 1) ERROR_SYNTAX;

    uintptr_t addr = get_peek_addr(p);
    g_integer_rtn = *((uint8_t *) addr);
    g_rtn_type = T_INT;
}

static char *GetCFunAddr(const char *p, int i) {
    uint32_t *ip = (uint32_t *) p;
    uint32_t size = 0;
    while (*((uint64_t *) ip) != 0xFFFFFFFFFFFFFFFF) {
        uintptr_t addr = *((uint64_t *) ip);
        ip += 2;
        size = *ip++;
        if (addr == (uintptr_t) (funtbl[i].addr)) {  // if we have a match
            int offset = *ip++;                      // get the offset in 32-bit words
            return (char *) (ip + offset);           // return the entry point
        } else {
            ip += size / sizeof(uint32_t);         // skip this CSUB
            while ((uintptr_t) ip % 2 != 0) ip++;  // and any trailing words to the next 64-bit
                                                   // boundary.
        }
    }

    return NULL;
}

/** PEEK(CFUNADDR cfun) */
static void peek_cfunaddr(int argc, char **argv, const char *p) {
    if (argc != 1) ERROR_SYNTAX;

    skipspace(p);

    // The argument may be an unquoted function / sub name.
    char name[MAXVARLEN + 1];
    MmResult result = parse_name(&p, name);
    int idx = -1;
    if (SUCCEEDED(result)) {
        idx = FindSubFun(name, kFunction | kSub);
    }

    // Or a string expression evaluating to a function / sub name.
    if (idx == -1) {
        getargs(&p, 1, ",");
        if (argc != 1) ERROR_ARGUMENT_COUNT;
        char *s = getCstring(argv[0]);
        idx = FindSubFun(s, kFunction | kSub);
    }

    const CommandToken cmd = (idx == -1)
        ? INVALID_COMMAND_TOKEN
        : commandtbl_decode(funtbl[idx].addr);
    if (cmd != cmdCFUN && cmd != cmdCSUB) ERROR_INVALID_ARGUMENT;

    // Search through program flash and the library looking for a match to
    // the function being called.
    char *addr = GetCFunAddr(CFunctionFlash, idx);
    // if (!addr) addr = GetCFunAddr(CFunctionLibrary, idx);
    if (!addr) ERROR_INTERNAL_FAULT;

    g_rtn_type = T_INT;
    g_integer_rtn = (uintptr_t) addr;
}

#include <stdio.h>

/** PEEK(DATAPTR) */
static void peek_dataptr(int argc, char **argv, const char *p) {
    if (argc != 1) ERROR_SYNTAX;
    assert(sizeof(DataReadPointer) == sizeof(MMINTEGER));
    DataReadPointer *pdrp = (DataReadPointer *) &g_integer_rtn;
    pdrp->next_line_offset = NextDataLine - ProgMemory;
    pdrp->next_data = NextData;
    g_rtn_type = T_INT;
}

/** PEEK(INTEGER addr%) */
static void peek_integer(int argc, char **argv, const char *p) {
    if (argc != 1) ERROR_SYNTAX;
#if defined(ENV32BIT)
    g_integer_rtn = *(uint64_t *)(get_peek_addr(p) & 0xFFFFFFF8);
#else
    g_integer_rtn = *(uint64_t *)(get_peek_addr(p) & 0xFFFFFFFFFFFFFFF8);
#endif
    g_rtn_type = T_INT;
}

/** PEEK(FLOAT addr%) */
static void peek_float(int argc, char **argv, const char *p) {
    if (argc != 1) ERROR_SYNTAX;
#if defined(ENV32BIT)
    g_float_rtn = *(MMFLOAT *)(get_peek_addr(p) & 0xFFFFFFF8);
#else
    g_float_rtn = *(MMFLOAT *)(get_peek_addr(p) & 0xFFFFFFFFFFFFFFF8);
#endif
    g_rtn_type = T_NBR;
}

/** PEEK(PROGMEM, offset) */
static void peek_progmem(int argc, char **argv, const char *p) {
    if (argc != 3) ERROR_SYNTAX;

    int64_t offset = getinteger(argv[2]);
    g_rtn_type = T_INT;
    g_integer_rtn = *(ProgMemory + offset);
}

/** PEEK(SHORT addr%) */
static void peek_short(int argc, char **argv, const char *p) {
    if (argc != 1) ERROR_SYNTAX;
#if defined(ENV32BIT)
    g_integer_rtn = *(uint16_t *)(get_peek_addr(p) & 0xFFFFFFFE);
#else
    g_integer_rtn = *(uint16_t *)(get_peek_addr(p) & 0xFFFFFFFFFFFFFFFE);
#endif
    g_rtn_type = T_INT;
}

/** PEEK(VAR, offset) */
static void peek_var(int argc, char **argv, const char *p) {
    if (argc != 3) ERROR_SYNTAX;

    void *pvar = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    int64_t offset = getinteger(argv[2]);
    g_rtn_type = T_INT;
    g_integer_rtn = *((char *) pvar + offset);
}

/** PEEK(VARADDR var) */
static void peek_varaddr(int argc, char **argv, const char *p) {
    if (argc != 1) ERROR_SYNTAX;
    void *pvar = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    g_rtn_type = T_INT;
    g_integer_rtn = (uintptr_t) pvar;
}

/** PEEK(VARHEADER var) */
static void peek_varheader(int argc, char **argv, const char *p) {
    if (argc != 1) ERROR_SYNTAX;
    (void) findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    g_rtn_type = T_INT;
    g_integer_rtn = (uintptr_t) &vartbl[VarIndex];
}

/** PEEK(VARTBL, offset) */
static void peek_vartbl(int argc, char **argv, const char *p) {
    if (argc != 3) ERROR_SYNTAX;

    int64_t offset = getinteger(argv[2]);
    g_rtn_type = T_INT;
    g_integer_rtn = *((char *) vartbl + offset);
}

/** PEEK(WORD addr%) */
static void peek_word(int argc, char **argv, const char *p) {
    if (argc != 1) ERROR_SYNTAX;
#if defined(ENV32BIT)
    g_integer_rtn = *(uint32_t *)(get_peek_addr(p) & 0xFFFFFFFC);
#else
    g_integer_rtn = *(uint32_t *)(get_peek_addr(p) & 0xFFFFFFFFFFFFFFFC);
#endif
    g_rtn_type = T_INT;
}

void fun_peek(void) {
    getargs(&ep, 3, ",");

    const char* p;
    if ((p = checkstring(argv[0], "BYTE"))) {
        peek_byte(argc, argv, p);
    } else if ((p = checkstring(argv[0], "CFUNADDR"))) {
        peek_cfunaddr(argc, argv, p);
    } else if ((p = checkstring(argv[0], "DATAPTR"))) {
        peek_dataptr(argc, argv, p);
    } else if ((p = checkstring(argv[0], "INTEGER"))) {
        peek_integer(argc, argv, p);
    } else if ((p = checkstring(argv[0], "FLOAT"))) {
        peek_float(argc, argv, p);
    } else if ((p = checkstring(argv[0], "PROGMEM"))) {
        peek_progmem(argc, argv, p);
    } else if ((p = checkstring(argv[0], "SHORT"))) {
        peek_short(argc, argv, p);
    } else if ((p = checkstring(argv[0], "VAR"))) {
        peek_var(argc, argv, p);
    } else if ((p = checkstring(argv[0], "VARADDR"))) {
        peek_varaddr(argc, argv, p);
    } else if ((p = checkstring(argv[0], "VARHEADER"))) {
        peek_varheader(argc, argv, p);
    } else if ((p = checkstring(argv[0], "VARTBL"))) {
        peek_vartbl(argc, argv, p);
    } else if ((p = checkstring(argv[0], "WORD"))) {
        peek_word(argc, argv, p);
    } else {
        ERROR_SYNTAX;
    }
}
