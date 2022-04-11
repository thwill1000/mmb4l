#include "../common/mmb4l.h"

void fun_linstr(void) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    char *srch;
    char *str = NULL;
    int slen, found = 0, i, j, n;
    getargs(&ep, 5, ",");
    if (argc < 3 || argc > 5) error("Argument count");
    int64_t start;
    if (argc == 5)
        start = getinteger(argv[4]) - 1;
    else
        start = 0;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
            error("Argument 1 must be integer array");
        }
        dest = (int64_t *)ptr1;
        str = (char *)&dest[0];
    } else
        error("Argument 1 must be integer array");
    j = (vartbl[VarIndex].dims[0] - mmb_options.base);
    srch = getstring(argv[2]);
    slen = *srch;
    iret = 0;
    if (start > dest[0] || start < 0 || slen == 0 || dest[0] == 0 ||
        slen > dest[0] - start)
        found = 1;
    if (!found) {
        n = dest[0] - slen - start;

        for (i = start; i <= n + start; i++) {
            if (str[i + 8] == srch[1]) {
                for (j = 0; j < slen; j++)
                    if (str[j + i + 8] != srch[j + 1]) break;
                if (j == slen) {
                    iret = i + 1;
                    break;
                }
            }
        }
    }
    targ = T_INT;
}
