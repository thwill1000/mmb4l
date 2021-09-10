#include <stdint.h>

#include "../common/error.h"
#include "../common/global_aliases.h"
#include "../common/utility.h"
#include "../common/version.h"

void peek_byte(int argc, char **argv, char *p) {
    if (argc != 1) error("Syntax");

    void *addr = get_peek_addr(p);
    g_integer_rtn = *((char *) addr);
    g_rtn_type = T_INT;
}

void peek_cfunaddr(int argc, char **argv, char *p) {
    ERROR_UNIMPLEMENTED("peek.c#peek_cfunaddr");
}

void peek_integer(int argc, char **argv, char *p) {
    if (argc != 1) error("Syntax");
    g_integer_rtn =
        *(uint64_t *)((uintptr_t)get_peek_addr(p) & 0xFFFFFFFFFFFFFFF8);
    g_rtn_type = T_INT;
}

void peek_float(int argc, char **argv, char *p) {
    // printf("a\n");
    if (argc != 1) error("Syntax");
    // printf("b\n");
    g_float_rtn =
        *(MMFLOAT *)((uintptr_t)get_peek_addr(p) & 0xFFFFFFFFFFFFFFF8);
    // printf("c\n");
    // printf("%g\n", g_float_rtn);
    g_rtn_type = T_NBR;
}

void peek_legacy(int argc, char **argv, char *p) {
    ERROR_UNIMPLEMENTED("peek.c#peek_legacy");
}

void peek_progmem(int argc, char **argv, char *p) {
    ERROR_UNIMPLEMENTED("peek.c#peek_progmem");
}

void peek_short(int argc, char **argv, char *p) {
    if (argc != 1) error("Syntax");
    g_integer_rtn =
        *(unsigned short *)((uintptr_t)get_peek_addr(p) & 0xFFFFFFFFFFFFFFFE);
    g_rtn_type = T_INT;
}

void peek_var(int argc, char **argv, char *p) {
    if (argc != 3) error("Syntax");

    void *pvar = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    int64_t offset = getinteger(argv[2]);
    g_integer_rtn = *((char *) pvar + offset);
    g_rtn_type = T_INT;
}

/** PEEK(VARADDR var) */
void peek_varaddr(int argc, char **argv, char *p) {
    if (argc != 1) error("Syntax");
    void *pvar = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    g_integer_rtn = (int64_t) pvar;
    g_rtn_type = T_INT;
}

void peek_vartbl(int argc, char **argv, char *p) {
    ERROR_UNIMPLEMENTED("peek.c#peek_vartbl");
}

/** Peek(Word addr%) */
void peek_word(int argc, char **argv, char *p) {
    if (argc != 1) error("Syntax");
    g_integer_rtn =
        *(unsigned int *)((uintptr_t)get_peek_addr(p) & 0xFFFFFFFFFFFFFFFC);
    g_rtn_type = T_INT;
}

// Will return a byte within the PIC32 virtual memory space.
void fun_peek(void) {
    getargs(&ep, 3, ",");

    char* p;
    if (p = checkstring(argv[0], "BYTE")) {
        peek_byte(argc, argv, p);
    } else if (p = checkstring(argv[0], "CFUNADDR")) {
        peek_cfunaddr(argc, argv, p);
    } else if (p = checkstring(argv[0], "INTEGER")) {
        peek_integer(argc, argv, p);
    } else if (p = checkstring(argv[0], "FLOAT")) {
        peek_float(argc, argv, p);
    } else if (p = checkstring(argv[0], "PROGMEM")) {
        peek_progmem(argc, argv, p);
    } else if (p = checkstring(argv[0], "SHORT")) {
        peek_short(argc, argv, p);
    } else if (p = checkstring(argv[0], "VAR")) {
        peek_var(argc, argv, p);
    } else if (p = checkstring(argv[0], "VARADDR")) {
        peek_varaddr(argc, argv, p);
    } else if (p = checkstring(argv[0], "VARTBL")) {
        peek_vartbl(argc, argv, p);
    } else if (p = checkstring(argv[0], "WORD")) {
        peek_word(argc, argv, p);
    } else {
        peek_legacy(argc, argv, p);
    }

    // if ((p = checkstring(argv[0], "CFUNADDR"))) {
    //     if (argc != 1) error("Syntax");
    //     i = FindSubFun(p, true);  // search for a function first
    //     if (i == -1)
    //         i = FindSubFun(p, false);  // and if not found try for a subroutine
    //     if (i == -1 || !(*subfun[i] == cmdCFUN || *subfun[i] == cmdCSUB))
    //         error("Invalid argument");
    //     // search through program flash and the library looking for a match to
    //     // the function being called
    //     j = GetCFunAddr((int *)CFunctionFlash, i);
    //     if (!j) j = GetCFunAddr((int *)CFunctionLibrary, i);
    //     if (!j) error("Internal fault (sorry)");
    //     iret = (unsigned int)j;  // return the entry point
    //     targ = T_INT;
    //     return;
    // }

    // if ((p = checkstring(argv[0], "BYTE"))) {
    //     if (argc != 1) error("Syntax");
    //     iret = *(unsigned char *)get_peek_addr(p);
    //     targ = T_INT;
    //     return;
    // }

    // if ((p = checkstring(argv[0], "WORD"))) {
    //     if (argc != 1) error("Syntax");
    //     iret = *(unsigned int *)(get_peek_addr(p) &
    //                              0b11111111111111111111111111111100);
    //     targ = T_INT;
    //     return;
    // }

    // if ((p = checkstring(argv[0], "INTEGER"))) {
    //     if (argc != 1) error("Syntax");
    //     iret = *(unsigned int *)(get_peek_addr(p) &
    //                              0b11111111111111111111111111111100);
    //     targ = T_INT;
    //     return;
    // }

    // if ((p = checkstring(argv[0], "FLOAT"))) {
    //     if (argc != 1) error("Syntax");
    //     fret =
    //         *(MMFLOAT *)(get_peek_addr(p) & 0b11111111111111111111111111111100);
    //     targ = T_NBR;
    //     return;
    // }

    // if (argc != 3) error("Syntax");

    // if (checkstring(argv[0], "PROGMEM")) {
    //     iret = *((char *)ProgMemory + (int)getinteger(argv[2]));
    //     targ = T_INT;
    //     return;
    // }

    // if (checkstring(argv[0], "VARTBL")) {
    //     iret = *((char *)vartbl + (int)getinteger(argv[2]));
    //     targ = T_INT;
    //     return;
    // }

    // if ((p = checkstring(argv[0], "VAR"))) {
    //     pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    //     iret = *((char *)pp + (int)getinteger(argv[2]));
    //     targ = T_INT;
    //     return;
    // }

    // // default action is the old syntax of  b = PEEK(hiaddr, loaddr)
    // iret =
    //     *(char *)(((int)getinteger(argv[0]) << 16) + (int)getinteger(argv[2]));
    // targ = T_INT;
}
