#include "../common/version.h"

void cmd_longstring(void) {
    char *tp;
    // tp = checkstring(cmdline, "SETBYTE");
    // if (tp) {
    //     void *ptr1 = NULL;
    //     int64_t *dest = NULL;
    //     int p = 0;
    //     uint8_t *q = NULL;
    //     int nbr;
    //     int j = 0;
    //     getargs(&tp, 5, ",");
    //     if (argc != 5) error("Argument count");
    //     ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    //     if (vartbl[VarIndex].type & T_INT) {
    //         if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
    //         if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
    //             error("Argument 1 must be integer array");
    //         }
    //         j = (vartbl[VarIndex].dims[0] - OptionBase) * 8 - 1;
    //         dest = (long long int *)ptr1;
    //         q = (uint8_t *)&dest[1];
    //     } else
    //         error("Argument 1 must be integer array");
    //     p = getint(argv[2], OptionBase, j - OptionBase);
    //     nbr = getint(argv[4], 0, 255);
    //     q[p - OptionBase] = nbr;
    //     return;
    // }
    tp = checkstring(cmdline, "APPEND");
    if (tp) {
        void *ptr1 = NULL;
        int64_t *dest = NULL;
        char *p = NULL;
        char *q = NULL;
        int i, j, nbr;
        getargs(&tp, 3, ",");
        if (argc != 3) error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q = (char *)&dest[1];
            q += dest[0];
        } else
            error("Argument 1 must be integer array");
        j = (vartbl[VarIndex].dims[0] - OptionBase);
        p = getstring(argv[2]);
        nbr = i = *p++;
        if (j * 8 < dest[0] + i) error("Integer array too small");
        while (i--) *q++ = *p++;
        dest[0] += nbr;
        return;
    }
    // tp = checkstring(cmdline, "TRIM");
    // if (tp) {
    //     void *ptr1 = NULL;
    //     int64_t *dest = NULL;
    //     uint32_t trim;
    //     char *p, *q = NULL;
    //     int i;
    //     getargs(&tp, 3, ",");
    //     if (argc != 3) error("Argument count");
    //     ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    //     if (vartbl[VarIndex].type & T_INT) {
    //         if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
    //         if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
    //             error("Argument 1 must be integer array");
    //         }
    //         dest = (long long int *)ptr1;
    //         q = (char *)&dest[1];
    //     } else
    //         error("Argument 1 must be integer array");
    //     trim = getint(argv[2], 1, dest[0] - 1);
    //     i = dest[0] - trim;
    //     p = q + trim;
    //     while (i--) *q++ = *p++;
    //     dest[0] -= trim;
    //     return;
    // }
    tp = checkstring(cmdline, "REPLACE");
    if (tp) {
        void *ptr1 = NULL;
        int64_t *dest = NULL;
        char *p = NULL;
        char *q = NULL;
        int i, nbr;
        getargs(&tp, 5, ",");
        if (argc != 5) error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q = (char *)&dest[1];
        } else
            error("Argument 1 must be integer array");
        p = getstring(argv[2]);
        nbr = getint(argv[4], 1, dest[0] - *p + 1);
        q += nbr - 1;
        i = *p++;
        while (i--) *q++ = *p++;
        return;
    }
    tp = checkstring(cmdline, "LOAD");
    if (tp) {
        void *ptr1 = NULL;
        int64_t *dest = NULL;
        char *p;
        char *q = NULL;
        int i, j;
        getargs(&tp, 5, ",");
        if (argc != 5) error("Argument count");
        int64_t nbr = getinteger(argv[2]);
        i = nbr;
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            dest[0] = 0;
            q = (char *)&dest[1];
        } else
            error("Argument 1 must be integer array");
        j = (vartbl[VarIndex].dims[0] - OptionBase);
        p = getstring(argv[4]);
        if (nbr > *p) nbr = *p;
        p++;
        if (j * 8 < dest[0] + nbr) error("Integer array too small");
        while (i--) *q++ = *p++;
        dest[0] += nbr;
        return;
    }
    tp = checkstring(cmdline, "LEFT");
    if (tp) {
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest = NULL, *src = NULL;
        char *p = NULL;
        char *q = NULL;
        int i, j, nbr;
        getargs(&tp, 5, ",");
        if (argc != 5) error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            q = (char *)&dest[1];
        } else
            error("Argument 1 must be integer array");
        j = (vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p = (char *)&src[1];
        } else
            error("Argument 2 must be integer array");
        nbr = i = getinteger(argv[4]);
        if (nbr > src[0]) nbr = i = src[0];
        if (j * 8 < i) error("Destination array too small");
        while (i--) *q++ = *p++;
        dest[0] = nbr;
        return;
    }
    tp = checkstring(cmdline, "RIGHT");
    if (tp) {
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest = NULL, *src = NULL;
        char *p = NULL;
        char *q = NULL;
        int i, j, nbr;
        getargs(&tp, 5, ",");
        if (argc != 5) error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            q = (char *)&dest[1];
        } else
            error("Argument 1 must be integer array");
        j = (vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p = (char *)&src[1];
        } else
            error("Argument 2 must be integer array");
        nbr = i = getinteger(argv[4]);
        if (nbr > src[0]) {
            nbr = i = src[0];
        } else
            p += (src[0] - nbr);
        if (j * 8 < i) error("Destination array too small");
        while (i--) *q++ = *p++;
        dest[0] = nbr;
        return;
    }
    tp = checkstring(cmdline, "MID");
    if (tp) {
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest = NULL, *src = NULL;
        char *p = NULL;
        char *q = NULL;
        int i, j, nbr, start;
        getargs(&tp, 7, ",");
        if (argc != 7) error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            q = (char *)&dest[1];
        } else
            error("Argument 1 must be integer array");
        j = (vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p = (char *)&src[1];
        } else
            error("Argument 2 must be integer array");
        start = getint(argv[4], 1, src[0]);
        nbr = getinteger(argv[6]);
        p += start - 1;
        if (nbr + start > src[0]) {
            nbr = src[0] - start + 1;
        }
        i = nbr;
        if (j * 8 < nbr) error("Destination array too small");
        while (i--) *q++ = *p++;
        dest[0] = nbr;
        return;
    }
    tp = checkstring(cmdline, "CLEAR");
    if (tp) {
        void *ptr1 = NULL;
        int64_t *dest = NULL;
        getargs(&tp, 1, ",");
        if (argc != 1) error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
        } else
            error("Argument 1 must be integer array");
        dest[0] = 0;
        return;
    }
    tp = checkstring(cmdline, "RESIZE");
    if (tp) {
        void *ptr1 = NULL;
        int64_t *dest = NULL;
        int j = 0;
        getargs(&tp, 3, ",");
        if (argc != 3) error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            j = (vartbl[VarIndex].dims[0] - OptionBase) * 8;
            dest = (long long int *)ptr1;
        } else
            error("Argument 1 must be integer array");
        dest[0] = getint(argv[2], OptionBase, j - OptionBase) + 1;
        return;
    }
    tp = checkstring(cmdline, "UCASE");
    if (tp) {
        void *ptr1 = NULL;
        int64_t *dest = NULL;
        char *q = NULL;
        int i;
        getargs(&tp, 1, ",");
        if (argc != 1) error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q = (char *)&dest[1];
        } else
            error("Argument 1 must be integer array");
        i = dest[0];
        while (i--) {
            if (*q >= 'a' && *q <= 'z') *q -= 0x20;
            q++;
        }
        return;
    }
    tp = checkstring(cmdline, "PRINT");
    if (tp) {
        void *ptr1 = NULL;
        int64_t *dest = NULL;
        char *q = NULL;
        int i, j, fnbr;
        getargs(&tp, 5, ",;");
        if (argc < 1 || argc > 4) error("Argument count");
        if (argc > 0 &&
            *argv[0] == '#') {  // check if the first arg is a file number
            argv[0]++;
            fnbr = getinteger(argv[0]);  // get the number
            i = 1;
            if (argc >= 2 && *argv[1] == ',')
                i = 2;  // and set the next argument to be looked at
        } else {
            fnbr = 0;  // no file number so default to the standard output
            i = 0;
        }
        if (argc >= 1) {
            ptr1 = findvar(argv[i], V_FIND | V_EMPTY_OK);
            if (vartbl[VarIndex].type & T_INT) {
                if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
                if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                    error("Argument must be integer array");
                }
                dest = (long long int *)ptr1;
                q = (char *)&dest[1];
            } else
                error("Argument must be integer array");
            j = dest[0];
            while (j--) {
                MMfputc(*q++, fnbr);
            }
            i++;
        }
        if (argc > i) {
            if (*argv[i] == ';') return;
        }
        MMfputs("\2\r\n", fnbr);
        return;
    }
    tp = checkstring(cmdline, "LCASE");
    if (tp) {
        void *ptr1 = NULL;
        int64_t *dest = NULL;
        char *q = NULL;
        int i;
        getargs(&tp, 1, ",");
        if (argc != 1) error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (long long int *)ptr1;
            q = (char *)&dest[1];
        } else
            error("Argument 1 must be integer array");
        i = dest[0];
        while (i--) {
            if (*q >= 'A' && *q <= 'Z') *q += 0x20;
            q++;
        }
        return;
    }
    tp = checkstring(cmdline, "COPY");
    if (tp) {
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest = NULL, *src = NULL;
        char *p = NULL;
        char *q = NULL;
        int i = 0, j;
        getargs(&tp, 3, ",");
        if (argc != 3) error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            dest[0] = 0;
            q = (char *)&dest[1];
        } else
            error("Argument 1 must be integer array");
        j = (vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p = (char *)&src[1];
            i = src[0];
        } else
            error("Argument 2 must be integer array");
        if (j * 8 < i) error("Destination array too small");
        while (i--) *q++ = *p++;
        dest[0] = src[0];
        return;
    }
    tp = checkstring(cmdline, "CONCAT");
    if (tp) {
        void *ptr1 = NULL;
        void *ptr2 = NULL;
        int64_t *dest = NULL, *src = NULL;
        char *p = NULL;
        char *q = NULL;
        int i = 0, j, d = 0, s = 0;
        getargs(&tp, 3, ",");
        if (argc != 3) error("Argument count");
        ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 1 must be integer array");
            }
            dest = (int64_t *)ptr1;
            d = dest[0];
            q = (char *)&dest[1];
        } else
            error("Argument 1 must be integer array");
        j = (vartbl[VarIndex].dims[0] - OptionBase);
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 2 must be integer array");
            }
            src = (int64_t *)ptr2;
            p = (char *)&src[1];
            i = s = src[0];
        } else
            error("Argument 2 must be integer array");
        if (j * 8 < (d + s)) error("Destination array too small");
        q += d;
        while (i--) *q++ = *p++;
        dest[0] += src[0];
        return;
    }
    error("Invalid option");
}
