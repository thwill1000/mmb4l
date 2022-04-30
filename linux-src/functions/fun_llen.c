#include "../common/mmb4l.h"
#include "../common/error.h"

void fun_llen(void) {
    void *ptr1 = NULL;
    int64_t *dest = NULL;
    getargs(&ep, 1, ",");
    if (argc != 1) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        dest = (int64_t *) ptr1;
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    iret = dest[0];
    targ = T_INT;
}
