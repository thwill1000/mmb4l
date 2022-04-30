#include "../common/mmb4l.h"
#include "../common/error.h"

#define ERROR_NOT_A_STRING              error_throw_ex(kError, "Not a string")
#define ERROR_SELECTION_EXCEEDS_LENGTH  error_throw_ex(kError, "Selection exceeds length of string")
#define ERROR_STRING_TOO_SHORT          error_throw_ex(kError, "Supplied string too short")

void cmd_mid(void){
    getargs(&cmdline, 5, ",");
    findvar(argv[0], V_NOFIND_ERR);
    if (vartbl[VarIndex].type & T_CONST) ERROR_CANNOT_CHANGE_A_CONSTANT;
    if (!(vartbl[VarIndex].type & T_STR)) ERROR_NOT_A_STRING;
    char *sourcestring = getstring(argv[0]);
    int start = getint(argv[2], 1, sourcestring[0]);
    int num = 0;
    if (argc == 5) num = getint(argv[4], 1, sourcestring[0]);
    if (start + num - 1 > sourcestring[0]) ERROR_SELECTION_EXCEEDS_LENGTH;
    while (*cmdline && tokenfunction(*cmdline) != op_equal) cmdline++;
    if (!*cmdline) ERROR_SYNTAX;
    ++cmdline;
    if (!*cmdline) ERROR_SYNTAX;
    char *value = getstring(cmdline);
    if (num == 0) num = value[0];
    if (num > value[0]) ERROR_STRING_TOO_SHORT;
    char *p = &value[1];
    memcpy(&sourcestring[start], p, num);
}
