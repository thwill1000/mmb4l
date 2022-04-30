#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/utility.h"

#define ERROR_ADDRESS_NOT_DIVISIBLE_BY(i)  error_throw_ex(kError, "Address not divisible by %", i)

/** POKE BYTE addr%, byte% */
static void poke_byte(int argc, char** argv, char *p) {
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    uintptr_t addr = get_poke_addr(p);
    int64_t value = getinteger(argv[2]);

    *((uint8_t *) addr) = (uint8_t) value;
}

/** POKE DATAPOS data_pos% */
static void poke_datapos(int argc, char** argv, char *p) {
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    uint64_t data_pos = (uint64_t) getinteger(p);
    NextDataLine = ProgMemory + (data_pos >> 32);
    NextData = data_pos & 0xFFFFFFFF;
}

/** POKE FLOAT addr%, float! */
static void poke_float(int argc, char** argv, char *p) {
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    uintptr_t addr = get_poke_addr(p);
    if (addr % 8) ERROR_ADDRESS_NOT_DIVISIBLE_BY(8);
    MMFLOAT value = getnumber(argv[2]);

    *((MMFLOAT *) addr) = value;
}

/** POKE INTEGER addr%, integer% */
static void poke_integer(int argc, char** argv, char *p) {
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    uintptr_t addr = get_poke_addr(p);
    if (addr % 8) ERROR_ADDRESS_NOT_DIVISIBLE_BY(8);
    int64_t value = getinteger(argv[2]);

    *((uint64_t *) addr) = (uint64_t) value;
}

/** POKE SHORT addr%, short% */
static void poke_short(int argc, char** argv, char *p) {
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    uintptr_t addr = get_poke_addr(p);
    if (addr % 2) ERROR_ADDRESS_NOT_DIVISIBLE_BY(2);
    int64_t value = getinteger(argv[2]);

    *((uint16_t *) addr) = (uint16_t) value;
}

/** POKE VAR var, offset%, byte% */
static void poke_var(int argc, char** argv, char *p) {
    if (argc != 5) ERROR_ARGUMENT_COUNT;

    void *pvar = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);

    if (vartbl[VarIndex].type & T_CONST) ERROR_CANNOT_CHANGE_A_CONSTANT;

    int64_t offset = getinteger(argv[2]);
    int64_t value = getinteger(argv[4]);

    *((char *)pvar + offset) = (uint8_t) value;
}

/** POKE VARTBL, offset%, byte% */
static void poke_vartbl(int argc, char** argv, char *p) {
    if (argc != 5) ERROR_ARGUMENT_COUNT;

    int64_t offset = getinteger(argv[2]);
    int64_t value = getinteger(argv[4]);

    *((char *) vartbl + offset) = (uint8_t) value;
}

/** POKE WORD addr%, word% */
static void poke_word(int argc, char** argv, char *p) {
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    uintptr_t addr = get_poke_addr(p);
    if (addr % 4) ERROR_ADDRESS_NOT_DIVISIBLE_BY(4);
    int32_t value = getinteger(argv[2]);

    *((uint32_t *) addr) = (uint32_t) value;
}

void cmd_poke(void) {
    getargs(&cmdline, 5, ",");

    char* p;
    if ((p = checkstring(argv[0], "BYTE"))) {
        poke_byte(argc, argv, p);
    } else if ((p = checkstring(argv[0], "DATAPOS"))) {
        poke_datapos(argc, argv, p);
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
