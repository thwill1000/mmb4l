#include <ctype.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/utility.h"

void cmd_call(void) {
    int i;
    char *q;
    char *p = getCstring(cmdline);  // get the command we want to call
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
    cstring_cat(p, " ", STRINGSIZE);
    cstring_cat(p, q, STRINGSIZE);
    // MMPrintString(p);PRet();
    if (i >= 0) {  // >= 0 means it is a user defined command
        DefinedSubFun(false, p, i, NULL, NULL, NULL, NULL);
    } else {
        error("Unknown user subroutine");
    }
}