#include <ctype.h>
#include <string.h>

#include "../common/version.h"

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
    strcat(p, " ");
    strcat(p, q);
    // MMPrintString(p);PRet();
    if (i >= 0) {  // >= 0 means it is a user defined command
        DefinedSubFun(false, p, i, NULL, NULL, NULL, NULL);
    } else {
        error("Unknown user subroutine");
    }
}