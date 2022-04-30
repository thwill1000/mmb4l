#include "../common/mmb4l.h"
#include "../common/error.h"

void fun_lgetstr(void) {
    void *ptr1 = NULL;
    char *p;
    char *s = NULL;
    int64_t *src = NULL;
    int start, nbr, j;
    getargs(&ep, 5, ",");
    if (argc != 5) ERROR_ARGUMENT_COUNT;
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK);
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID_VARIABLE;
        if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARG_NOT_INTEGER_ARRAY(1);
        src = (int64_t *)ptr1;
        s = (char *)&src[1];
    } else ERROR_ARG_NOT_INTEGER_ARRAY(1);
    j = (vartbl[VarIndex].dims[0] - mmb_options.base) * 8;
    start = getint(argv[2], 1, j);
    nbr = getinteger(argv[4]);
    if (nbr < 1 || nbr > MAXSTRLEN) ERROR_NUMBER_OUT_OF_BOUNDS;
    if (start + nbr > src[0]) nbr = src[0] - start + 1;
    sret = GetTempStrMemory();  // this will last for the life of the command
    s += (start - 1);
    p = sret + 1;
    *sret = nbr;
    while (nbr--) *p++ = *s++;
    *p = 0;
    targ = T_STR;
}
