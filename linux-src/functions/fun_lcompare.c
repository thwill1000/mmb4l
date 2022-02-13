#include "../common/mmb4l.h"

void fun_lcompare(void) {
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    int64_t *dest, *src;
    char *p = NULL;
    char *q = NULL;
    int d = 0, s = 0, found = 0;
    getargs(&ep, 3, ",");
    if (argc != 3) error("Argument count");
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
            error("Argument 1 must be integer array");
        }
        dest = (int64_t *)ptr1;
        q = (char *)&dest[1];
        d = dest[0];
    } else
        error("Argument 1 must be integer array");
    ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
            error("Argument 2 must be integer array");
        }
        src = (int64_t *)ptr2;
        p = (char *)&src[1];
        s = src[0];
    } else
        error("Argument 2 must be integer array");
    while (!found) {
        if (d == 0 && s == 0) {
            found = 1;
            iret = 0;
        }
        if (d == 0 && !found) {
            found = 1;
            iret = -1;
        }
        if (s == 0 && !found) {
            found = 1;
            iret = 1;
        }
        if (*q < *p && !found) {
            found = 1;
            iret = -1;
        }
        if (*q > *p && !found) {
            found = 1;
            iret = 1;
        }
        q++;
        p++;
        d--;
        s--;
    }
    targ = T_INT;
}
