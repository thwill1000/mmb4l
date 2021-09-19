#include <stdint.h>

#include "../common/error.h"
#include "../common/utility.h"
#include "../common/version.h"

/** POKE BYTE addr%, byte% */
static void poke_byte(int argc, char** argv, char *p) {
    if (argc != 3) error("Argument count");

    void *addr = get_poke_addr(p);
    int64_t value = getinteger(argv[2]);

    *((uint8_t *) addr) = (uint8_t) value;
}

/** POKE FLOAT addr%, float! */
static void poke_float(int argc, char** argv, char *p) {
    if (argc != 3) error("Argument count");

    void *addr = get_poke_addr(p);
    if (((uintptr_t) addr) % 8) error("Address not divisible by 8");
    MMFLOAT value = getnumber(argv[2]);

    *((MMFLOAT *) addr) = value;
}

/** POKE INTEGER addr%, integer% */
static void poke_integer(int argc, char** argv, char *p) {
    if (argc != 3) error("Argument count");

    void *addr = get_poke_addr(p);
    if (((uintptr_t) addr) % 8) error("Address not divisible by 8");
    int64_t value = getinteger(argv[2]);

    *((uint64_t *) addr) = (uint64_t) value;
}

static void poke_legacy(int argc, char** argv, char *p) {
    ERROR_UNIMPLEMENTED("poke.c#poke_legacy");
}

/** POKE SHORT addr%, short% */
static void poke_short(int argc, char** argv, char *p) {
    if (argc != 3) error("Argument count");

    void *addr = get_poke_addr(p);
    if (((uintptr_t) addr) % 2) error("Address not divisible by 2");
    int64_t value = getinteger(argv[2]);

    *((uint16_t *) addr) = (uint16_t) value;
}

/** POKE VAR var, offset%, byte% */
static void poke_var(int argc, char** argv, char *p) {
    void *pvar = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);

    if (vartbl[VarIndex].type & T_CONST) {
        error("Cannot change a constant");
    }

    int64_t offset = getinteger(argv[2]);
    int64_t value = getinteger(argv[4]);

    *((char *)pvar + offset) = value;
}

/** POKE VARTBL, offset%, byte% */
static void poke_vartbl(int argc, char** argv, char *p) {
    ERROR_UNIMPLEMENTED("poke.c#poke_vartbl");
}

/** POKE WORD addr%, word% */
static void poke_word(int argc, char** argv, char *p) {
    if (argc != 3) error("Argument count");

    void *addr = get_poke_addr(p);
    if (((uintptr_t) addr) % 4) error("Address not divisible by 4");
    int32_t value = getinteger(argv[2]);

    *((uint32_t *) addr) = (uint32_t) value;
}

void cmd_poke(void) {
    getargs(&cmdline, 5, ",");

    char* p;
    if ((p = checkstring(argv[0], "BYTE"))) {
        poke_byte(argc, argv, p);
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
        poke_legacy(argc, argv, p);
    }
}