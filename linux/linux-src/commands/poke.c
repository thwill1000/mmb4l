#include "../common/utility.h"
#include "../common/version.h"

// utility function used by cmd_poke() to validate an address
void *GetPokeAddr(void *p) {
    return (void *) getinteger(p);

    // TODO
    // if ((i > (unsigned int)DOS_vartbl &&
    //      i < (unsigned int)DOS_vartbl + sizeof(struct s_vartbl) * MAXVARS) ||
    //     (i > (unsigned int)MMHeap && i < (unsigned int)MMHeap + HEAP_SIZE))
    //     return i;
    // else
    //     error("Address");
    // return 0;
}

/** POKE BYTE addr%, byte */
void poke_byte(int argc, char** argv, char *p) {
    if (argc != 3) error("Argument count");

    void *addr = GetPokeAddr(p);
    int64_t value = getinteger(argv[2]);

    *((char *)addr) = value;
}

void poke_legacy(int argc, char** argv, char *p) {
    ERROR_UNIMPLEMENTED("poke.c#poke_legacy");
}

/** POKE VAR var, offset, byte */
void poke_var(int argc, char** argv, char *p) {
    void *pvar = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);

    if (vartbl[VarIndex].type & T_CONST) {
        error("Cannot change a constant");
    }

    int64_t offset = getinteger(argv[2]);
    int64_t value = getinteger(argv[4]);

    *((char *)pvar + offset) = value;
}

/** POKE VARTBL, offset, byte */
void poke_vartbl(int argc, char** argv, char *p) {
    ERROR_UNIMPLEMENTED("poke.c#poke_vartbl");
}

/** POKE WORD addr%, word% */
void poke_word(int argc, char** argv, char *p) {
    ERROR_UNIMPLEMENTED("poke.c#poke_word");
}

void cmd_poke(void) {
    getargs(&cmdline, 5, ",");

    char* p;
    if (p = checkstring(argv[0], "BYTE")) {
        poke_byte(argc, argv, p);
    } else if (p = checkstring(argv[0], "VAR")) {
        poke_var(argc, argv, p);
    } else if (p = checkstring(argv[0], "VARTBL")) {
        poke_vartbl(argc, argv, p);
    } else if (p = checkstring(argv[0], "WORD")) {
        poke_word(argc, argv, p);
    } else {
        poke_legacy(argc, argv, p);
    }

    // if ((p = checkstring(argv[0], "WORD"))) {
    //     if (argc != 3) error("Argument count");
    //     *(unsigned int *)(GetPokeAddr(p) & 0b11111111111111111111111111111100) =
    //         getinteger(argv[2]);
    //     return;
    // }

    // if (argc != 5) error("Argument count");

    // if (checkstring(argv[0], "VARTBL")) {
    //     *((char *)vartbl + (unsigned int)getinteger(argv[2])) =
    //         getinteger(argv[4]);
    //     return;
    // }
    // if ((p = checkstring(argv[0], "VAR"))) {
    //     pp = findvar(p, V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    //     if (vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
    //     *((char *)pp + (unsigned int)getinteger(argv[2])) = getinteger(argv[4]);
    //     return;
    // }
    // // the default is the old syntax of:   POKE hiaddr, loaddr, byte
    // *(char *)(((int)getinteger(argv[0]) << 16) + (int)getinteger(argv[2])) =
    //     getinteger(argv[4]);
}