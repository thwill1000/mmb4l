#include "../common/error.h"
#include "../common/utility.h"
#include "../common/version.h"

unsigned int UsedHeap(void); // memory.c

// Data type sizes:
//   FLOAT   - 8 bytes
//   INTEGER - 8 bytes
//   SHORT   - 2 bytes
//   WORD    - 4 bytes

/** MEMORY COPY BYTE src, dst, number_of_bytes. */
void memory_copy_byte(char *p) {
    getargs(&p, 5, ",");
    if (argc != 5) error("Syntax");
    void *src = get_peek_addr(argv[0]);
    void *dst = get_poke_addr(argv[2]);
    int64_t num = getinteger(argv[4]);
    if (num > 0) {
        memcpy(dst, src, num);
    }
}

/** MEMORY COPY FLOAT src, std, number_of_floats. */
void memory_copy_float(char *p) {
    ERROR_UNIMPLEMENTED("memory_cmd.c#memory_copy_float");
}

/** MEMORY COPY INTEGER src, dst, number_of_integers. */
void memory_copy_integer(char *p) {
    ERROR_UNIMPLEMENTED("memory_cmd.c#memory_copy_integer");
}

/** MEMORY COPY SHORT src, dst, number_of_shorts. */
void memory_copy_short(char *p) {
    ERROR_UNIMPLEMENTED("memory_cmd.c#memory_copy_short");
}

/** MEMORY COPY WORD src, dst, number_of_words. */
void memory_copy_word(char *p) {
    ERROR_UNIMPLEMENTED("memory_cmd.c#memory_copy_word");
}

void memory_copy(char *p) {
    char *p2;
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

/** MEMORY SET BYTE address, byte_value, number_of_bytes. */
void memory_set_byte(char *p) {
    getargs(&p, 5, ",");
    if (argc != 5) error("Syntax");
    //printf("%s\n", argv[0]);
    void *to = get_poke_addr(argv[0]);
    //printf("%ld\n", to);
    int64_t value = getint(argv[2], 0, 255);
    int64_t num = getinteger(argv[4]);
    if (num > 0) {
        memset(to, (int) value, (size_t) num);
    }
}

/** MEMORY SET FLOAT address, float_value, number_of_floats. */
void memory_set_float(char *p) {
    ERROR_UNIMPLEMENTED("memory_cmd.c#memory_set_float");
}

/** MEMORY SET INTEGER address, integer, number_of_integers. */
void memory_set_integer(char *p) {
    ERROR_UNIMPLEMENTED("memory_cmd.c#memory_set_integer");
}

#define uint64_t unsigned long long

/** MEMORY SET SHORT address, short_value, number_of_shorts. */
void memory_set_short(char *p) {
    getargs(&p, 5, ",");
    if (argc != 5) error("Syntax");
    //printf("%s\n", argv[0]);
    void *to = get_poke_addr(argv[0]);
    if ((uint64_t) to % 2) error("Address not divisible by 2");
    //printf("%ld\n", to);
    // TODO: should we cope with -ve values ?
    int64_t value = getint(argv[2], 0, 65535);
    int64_t num = getinteger(argv[4]);
    for (int64_t i = 0; i < num; ++i) {
        memcpy(to + 2 * i, &value, 2);
    }
}

/** MEMORY SET WORD address, word_value, number_of_words. */
void memory_set_word(char *p) {
    ERROR_UNIMPLEMENTED("memory_cmd.c#memory_set_word");
}

void memory_set(char *p) {
    char *p2;
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

void memory_report(char *unused) {

    int i, vcnt;
    int pbytes, pm, pp, gm, gp;

    // calculate the number of variables
    for (i = vcnt = 0; i < varcnt; i++) {
        if (vartbl[i].type != T_NOTYPE) vcnt++;
    }

    // count the number of lines in the program
    char *p = ProgMemory;
    pbytes = i = 0;
    while (1) {
        while (*p) {
            p++;
            pbytes++;
        }  // look for the zero marking the start of an element
        if (p[1] == 0) break;  // end of the program
        if (p[1] == T_LINENBR) {
            i++;
            p += 3;  // skip over the line number
            pbytes += 3;
        } else if (p[1] == T_NEWLINE) {
            i++;
            p += 1;  // skip over the new line token
            pbytes += 1;
        }
        p++;
        pbytes++;
        skipspace(p);
        if (p[0] == T_LABEL) {
            pbytes += p[1] + 2;
            p += p[1] + 2;  // skip over the label
        }
    }
    pm = (PROG_FLASH_SIZE + 512) / 1024;
    pp = ((PROG_FLASH_SIZE + 512) * 100) / PROG_FLASH_SIZE;
    gm = (UsedHeap() + 512) / 1024;
    gp = ((UsedHeap() + 512) * 100) / HEAP_SIZE;

    sprintf(
            inpbuf,
            "    Program:%4dK (%2d%%) used %3dK free (%d line%s)\r\n",
            (pbytes + 512) / 1024,
            (pbytes * 100) / PROG_FLASH_SIZE,
            (PROG_FLASH_SIZE - pbytes + 512) / 1024,
            i,
            i == 1 ? "" : "s");
    MMPrintString(inpbuf);
    sprintf(
            inpbuf,
            "  Variables:%4ldK (%2d%%) used %3ldK free (%d variables)\r\n",
            ((vcnt * sizeof(struct s_vartbl)) + 512) / 1024,
            (vcnt * 100) / MAXVARS,
            (((MAXVARS * sizeof(struct s_vartbl)) + 512) / 1024) - (((vcnt * sizeof(struct s_vartbl)) + 512) / 1024),
            vcnt);
    MMPrintString(inpbuf);
    sprintf(
            inpbuf,
            "General RAM:%4dK (%2d%%) used %3dK free\r\n",
            gm,
            gp,
            (HEAP_SIZE / 1024) - gm);
    MMPrintString(inpbuf);
}

void cmd_memory(void) {
    // getargs(&cmdline, 10, ",");
    // for (int i = 0; i < argc; ++i) {
    //     printf("%d: %s, ", i, argv[i]);
    // }
    // printf("\n");
    char *p;
    if ((p = checkstring(cmdline, "COPY"))) {
        memory_copy(p);
    } else if ((p = checkstring(cmdline, "SET"))) {
        memory_set(p);
    } else {
        memory_report(p);
    }
}
