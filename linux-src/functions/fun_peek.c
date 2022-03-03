#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/memory.h"
#include "../common/utility.h"

/** PEEK(BYTE addr%) */
static void peek_byte(int argc, char **argv, char *p) {
    if (argc != 1) ERROR_SYNTAX;

    uintptr_t addr = get_peek_addr(p);
    g_integer_rtn = *((uint8_t *) addr);
    g_rtn_type = T_INT;
}

static char *GetCFunAddr(char *p, int i) {
    uint32_t *ip = (uint32_t *) p;
    uint32_t size = 0;
    while (*((uint64_t *) ip) != 0xFFFFFFFFFFFFFFFF) {
        uintptr_t addr = *((uint64_t *) ip);
        ip += 2;
        size = *ip++;
        if (addr == (uintptr_t) g_subfun[i]) {  // if we have a match
            int offset = *ip++;                 // get the offset in 32-bit words
            return (char *) (ip + offset);      // return the entry point
        } else {
            ip += size / sizeof(uint32_t);         // skip this CSUB
            while ((uintptr_t) ip % 2 != 0) ip++;  // and any trailing words to the next 64-bit
                                                   // boundary.
        }
    }

    return NULL;
}

/** PEEK(CFUNADDR cfun) */
static void peek_cfunaddr(int argc, char **argv, char *p) {
    if (argc != 1) ERROR_SYNTAX;
    int idx = FindSubFun(p, true);  // search for a function first
    if (idx == -1)
        idx = FindSubFun(p, false);  // and if not found try for a subroutine
    if (idx == -1 || !(*subfun[idx] == cmdCFUN || *subfun[idx] == cmdCSUB))
        ERROR_INVALID_ARGUMENT;

    // Search through program flash and the library looking for a match to
    // the function being called.
    char *addr = GetCFunAddr(CFunctionFlash, idx);
    // if (!addr) addr = GetCFunAddr(CFunctionLibrary, idx);
    if (!addr) ERROR_INTERNAL_FAULT;

    g_rtn_type = T_INT;
    g_integer_rtn = (uintptr_t) addr;
}

#include <stdio.h>

/** PEEK(DATAPOS) */
static void peek_datapos(int argc, char **argv, char *p) {
    if (argc != 1) ERROR_SYNTAX;
    uint64_t data_pos = ((NextDataLine - ProgMemory) << 32) + NextData;
    g_integer_rtn = data_pos;
    g_rtn_type = T_INT;
}

/** PEEK(INTEGER addr%) */
static void peek_integer(int argc, char **argv, char *p) {
    if (argc != 1) ERROR_SYNTAX;
#if defined(ENV32BIT)
    g_integer_rtn = *(uint64_t *)(get_peek_addr(p) & 0xFFFFFFF8);
#else
    g_integer_rtn = *(uint64_t *)(get_peek_addr(p) & 0xFFFFFFFFFFFFFFF8);
#endif
    g_rtn_type = T_INT;
}

/** PEEK(FLOAT addr%) */
static void peek_float(int argc, char **argv, char *p) {
    if (argc != 1) ERROR_SYNTAX;
#if defined(ENV32BIT)
    g_float_rtn = *(MMFLOAT *)(get_peek_addr(p) & 0xFFFFFFF8);
#else
    g_float_rtn = *(MMFLOAT *)(get_peek_addr(p) & 0xFFFFFFFFFFFFFFF8);
#endif
    g_rtn_type = T_NBR;
}

/** PEEK(PROGMEM, offset) */
static void peek_progmem(int argc, char **argv, char *p) {
    if (argc != 3) ERROR_SYNTAX;

    int64_t offset = getinteger(argv[2]);
    g_rtn_type = T_INT;
    g_integer_rtn = *(ProgMemory + offset);
}

/** PEEK(SHORT addr%) */
static void peek_short(int argc, char **argv, char *p) {
    if (argc != 1) ERROR_SYNTAX;
#if defined(ENV32BIT)
    g_integer_rtn = *(uint16_t *)(get_peek_addr(p) & 0xFFFFFFFE);
#else
    g_integer_rtn = *(uint16_t *)(get_peek_addr(p) & 0xFFFFFFFFFFFFFFFE);
#endif
    g_rtn_type = T_INT;
}

/** PEEK(VAR, offset) */
static void peek_var(int argc, char **argv, char *p) {
    if (argc != 3) ERROR_SYNTAX;

    void *pvar = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    int64_t offset = getinteger(argv[2]);
    g_rtn_type = T_INT;
    g_integer_rtn = *((char *) pvar + offset);
}

/** PEEK(VARADDR var) */
static void peek_varaddr(int argc, char **argv, char *p) {
    if (argc != 1) ERROR_SYNTAX;
    void *pvar = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    g_rtn_type = T_INT;
    g_integer_rtn = (uintptr_t) pvar;
}

/** PEEK(VARHEADER var) */
static void peek_varheader(int argc, char **argv, char *p) {
    if (argc != 1) ERROR_SYNTAX;
    void *pvar = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    g_rtn_type = T_INT;
    g_integer_rtn = (uintptr_t) &vartbl[VarIndex];
}

/** PEEK(VARTBL, offset) */
static void peek_vartbl(int argc, char **argv, char *p) {
    if (argc != 3) ERROR_SYNTAX;

    int64_t offset = getinteger(argv[2]);
    g_rtn_type = T_INT;
    g_integer_rtn = *((char *) vartbl + offset);
}

/** PEEK(WORD addr%) */
static void peek_word(int argc, char **argv, char *p) {
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

    char* p;
    if ((p = checkstring(argv[0], "BYTE"))) {
        peek_byte(argc, argv, p);
    } else if ((p = checkstring(argv[0], "CFUNADDR"))) {
        peek_cfunaddr(argc, argv, p);
    } else if ((p = checkstring(argv[0], "DATAPOS"))) {
        peek_datapos(argc, argv, p);
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
