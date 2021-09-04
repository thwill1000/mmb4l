#include "../common/version.h"

void fun_call(void) {
    int i;
    long long int i64 = 0;
    char *s = NULL;
    MMFLOAT f;
    char *q;
    char *p = getCstring(ep);  // get the command we want to call
    q = p;
    while (*q) {  // convert to upper case for the match
        *q = toupper(*q);
        q++;
    }
    q = cmdline;
    while (*q) {
        if (*q == ',' || *q == '\'') break;
        q++;
    }
    if (*q == ',') q++;
    i = FindSubFun(p, false);  // it could be a defined command
    strcat(p, " ");
    strcat(p, q);
    // MMPrintString(p);PRet();
    targ = T_NOTYPE;
    if (i >= 0) {  // >= 0 means it is a user defined command
        DefinedSubFun(true, p, i, &f, &i64, &s, &targ);
    } else
        error("Unknown user function");
    if (targ & T_STR) {
        sret = GetTempStrMemory();
        Mstrcpy(sret, s);  // if it is a string then save it
    }
}