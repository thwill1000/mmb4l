#include "../common/version.h"

void fun_choice(void) {
    MMFLOAT f = 0;
    long long int i64 = 0;
    char *s = NULL;
    int t = T_NOTYPE;
    getargs(&ep, 5, ",");
    if (argc != 5) error("Syntax");
    int which = getnumber(argv[0]);
    if (which) {
        evaluate(argv[2], &f, &i64, &s, &t, false);
    } else {
        evaluate(argv[4], &f, &i64, &s, &t, false);
    }
    if (t & T_INT) {
        iret = i64;
        targ = T_INT;
        return;
    } else if (t & T_NBR) {
        fret = f;
        targ = T_NBR;
        return;
    } else if (t & T_STR) {
        sret = GetTempStrMemory();
        Mstrcpy(sret, s);  // copy the string
        targ = T_STR;
        return;
    } else
        error("Syntax");
}
