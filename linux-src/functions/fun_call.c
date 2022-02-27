#include <ctype.h>

#include "../common/mmb4l.h"
#include "../common/utility.h"

void fun_call(void) {
    int i;
    long long int i64 = 0;
    char *s = NULL;
    MMFLOAT f;
    char *q;
    char *p = getCstring(ep);  // get the function we want to call
    q = p;
    while (*q) {  // convert to upper case for the match
        *q = toupper(*q);
        q++;
    }
    q = ep;
    while (*q) {
        if (*q == ',' || *q == '\'') break;
        q++;
    }
    if (*q == ',') q++;
    i = FindSubFun(p, true);  // it could be a defined function
    cstring_cat(p, " ", STRINGSIZE);
    cstring_cat(p, q, STRINGSIZE);
    targ = T_NOTYPE;
    if (i >= 0) {  // >= 0 means it is a user defined function
        DefinedSubFun(true, p, i, &f, &i64, &s, &targ);
    } else
        error("Unknown user function");
    if (targ & T_STR) {
        sret = GetTempStrMemory();
        Mstrcpy(sret, s);  // if it is a string then save it
    }
    if (targ & T_INT) iret = i64;
    if (targ & T_NBR) fret = f;
}
