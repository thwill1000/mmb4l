#include "../common/version.h"

void fun_lgetbyte(void) {
    // uint8_t *s = NULL;
    char *s = NULL;
    //int64_t *src = NULL;
    getargs(&ep, 3, ",");
    if (argc != 3) error("Argument count");
    void *pvar = findvar(argv[0], V_FIND | V_EMPTY_OK); // Sets global VarIndex to point to variable.
    if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
            error("Argument 1 must be integer array");
        }
        //src = (int64_t *)pvar;
        // s = (char *)&src[1];
        s = (char *)&pvar[1];
    } else {
        error("Argument 1 must be integer array");
    }
    int j = (vartbl[VarIndex].dims[0] - OptionBase) * 8 - 1;
    int start = getint(argv[2], OptionBase, j - OptionBase);
    iret = s[start - OptionBase];
    targ = T_INT;
}
