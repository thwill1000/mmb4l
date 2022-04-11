#include "../common/mmb4l.h"
#include "../common/error.h"

void cmd_mid(void){
    getargs(&cmdline, 5, ",");
    findvar(argv[0], V_NOFIND_ERR);
    if (vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
    if (!(vartbl[VarIndex].type & T_STR)) error("Not a string");
    char *sourcestring = getstring(argv[0]);
    int start = getint(argv[2], 1, sourcestring[0]);
    int num = 0;
    if (argc == 5) num = getint(argv[4], 1, sourcestring[0]);
    if (start + num - 1 > sourcestring[0]) {
        error("Selection exceeds length of string");
    }
    while (*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
    if (!*cmdline) ERROR_SYNTAX;
    ++cmdline;
    if (!*cmdline) ERROR_SYNTAX;
    char *value = getstring(cmdline);
    if (num == 0) num = value[0];
    if (num > value[0]) error("Supplied string too short");
    char *p = &value[1];
    memcpy(&sourcestring[start], p, num);
}
