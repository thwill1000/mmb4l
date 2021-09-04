#include "../common/version.h"

static void math_set(char *p) {
    int t = T_NBR;
    MMFLOAT f;
    long long int i64;
    char *s;
    void *ptr1 = NULL;
    int i, j, card1 = 1;
    MMFLOAT *a1float = NULL;
    int64_t *a1int = NULL;
    getargs(&p, 3, ",");
    if (!(argc == 3)) error("Argument count");
    evaluate(argv[0], &f, &i64, &s, &t, false);
    if (t & T_STR) error("Syntax");
    ptr1 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if (vartbl[VarIndex].type & T_NBR) {
        card1 = 1;
        for (i = 0; i < MAXDIM; i++) {
            j = (vartbl[VarIndex].dims[i] - OptionBase + 1);
            if (j) card1 *= j;
        }
        a1float = (MMFLOAT *)ptr1;
        if (ptr1 != vartbl[VarIndex].val.s) error("Syntax");
    } else if (vartbl[VarIndex].type & T_INT) {
        card1 = 1;
        for (i = 0; i < MAXDIM; i++) {
            j = (vartbl[VarIndex].dims[i] - OptionBase + 1);
            if (j) card1 *= j;
        }
        a1int = (int64_t *)ptr1;
        if (ptr1 != vartbl[VarIndex].val.s) error("Syntax");
    } else
        error("Argument 2 must be numerical");
    if (a1float != NULL) {
        for (i = 0; i < card1; i++)
            *a1float++ = ((t & T_INT) ? (MMFLOAT)i64 : f);
    } else {
        for (i = 0; i < card1; i++)
            *a1int++ = ((t & T_INT) ? i64 : FloatToInt64(f));
    }
}

void cmd_math(void) {
    char *p;
    if (p = checkstring(cmdline, "SET")) {
        math_set(p);
    } else {
        error("Syntax");
    }
}
