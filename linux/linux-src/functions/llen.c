#include "../common/global_aliases.h"
#include "../common/version.h"

void fun_llen(void) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    getargs(&ep, 1, ",");
    if (argc != 1) error("Argument count");
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
            error("Argument 1 must be integer array");
        }
        dest = (int64_t *) ptr1;
    } else
        error("Argument 1 must be integer array");
    iret = dest[0];
    targ = T_INT;
}
