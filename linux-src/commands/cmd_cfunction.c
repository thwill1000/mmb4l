#include "../common/mmb4l.h"
#include "../common/error.h"

void cmd_cfunction(void) {
    int end_token = GetCommandValue("End CSub");  // this terminates a CSUB
    char *p = cmdline;
    while (*p != 0xff) {
        if (*p == 0) p++;  // if it is at the end of an element skip the zero marker
        if (*p == 0) error("Missing END statement");  // end of the program
        if (*p == T_NEWLINE) p++;     // skip over the newline token
        if (*p == T_LINENBR) p += 3;  // skip over the line number
        skipspace(p);
        if (*p == T_LABEL) {
            p += p[1] + 2;  // skip over the label
            skipspace(p);   // and any following spaces
        }
        if (*p == end_token) {  // found an END token
            nextstmt = p;
            skipelement(nextstmt);
            return;
        }
        p++;
    }
}
