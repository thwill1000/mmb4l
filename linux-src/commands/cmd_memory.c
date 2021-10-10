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
    void *src = (void *) get_peek_addr(argv[0]);
    void *dst = (void *) get_poke_addr(argv[2]);
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
    uintptr_t to = get_poke_addr(argv[0]);
    int64_t value = getint(argv[2], 0, 255);
    int64_t num = getinteger(argv[4]);
    if (num > 0) {
        memset((void *) to, (int) value, (size_t) num);
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
    uintptr_t to = get_poke_addr(argv[0]);
    if ((uintptr_t) to % 2) error("Address not divisible by 2");
    // TODO: should we cope with -ve values ?
    int64_t value = getint(argv[2], 0, 65535);
    int64_t num = getinteger(argv[4]);
    for (int32_t i = 0; i < num; ++i) {
        memcpy((void *) (to + 2 * i), &value, 2);
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

static void count_program_size_and_lines(int *num_bytes, int *num_lines) {
    *num_bytes = 0;
    *num_lines = 0;

    char *p = ProgMemory;
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

void memory_report(char *unused) {

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
    MMPrintString(inpbuf);

    int32_t vcnt = count_variables();
    int32_t size = sizeof(struct s_vartbl);
    sprintf(
            inpbuf,
            "  Variables:%4dK (%2d%%) used %3dK free (%d variables)\r\n",
            (int32_t) ((vcnt * size + 512) / 1024),
            (int32_t) (vcnt * 100 / MAXVARS),
            (int32_t) (((MAXVARS * size + 512) / 1024) - ((vcnt * size + 512) / 1024)),
            vcnt);
    MMPrintString(inpbuf);

    int ram_used = (UsedHeap() + 512) / 1024;
    int percent_used = ((UsedHeap() + 512) * 100) / HEAP_SIZE;
    sprintf(
            inpbuf,
            "General RAM:%4dK (%2d%%) used %3dK free\r\n",
            ram_used,
            percent_used,
            (HEAP_SIZE / 1024) - ram_used);
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