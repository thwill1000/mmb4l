#include "../common/mmb4l.h"
#include "../common/error.h"

void cmd_inc(void) {
    char *p, *q;
    int vtype;
    getargs(&cmdline, 3, ",");
    if (argc == 1) {
        p = findvar(argv[0], V_FIND);
        if (vartbl[VarIndex].type & T_CONST) ERROR_CANNOT_CHANGE_A_CONSTANT;
        vtype = TypeMask(vartbl[VarIndex].type);
        if (vtype & T_STR) ERROR_INVALID_VARIABLE;
        if (vtype & T_NBR)
            (*(MMFLOAT *)p) = (*(MMFLOAT *)p) + 1.0;
        else if (vtype & T_INT)
            *(long long int *)p = *(long long int *)p + 1;
        else
            ERROR_SYNTAX;
    } else {
        p = findvar(argv[0], V_FIND);
        if (vartbl[VarIndex].type & T_CONST) ERROR_CANNOT_CHANGE_A_CONSTANT;
        vtype = TypeMask(vartbl[VarIndex].type);
        if (vtype & T_STR) {
            q = getstring(argv[2]);
            if (*p + *q > MAXSTRLEN) ERROR_STRING_TOO_LONG;
            Mstrcat(p, q);
        } else if (vtype & T_NBR) {
            (*(MMFLOAT *)p) = (*(MMFLOAT *)p) + getnumber(argv[2]);
        } else if (vtype & T_INT) {
            *(long long int *)p = *(long long int *)p + getinteger(argv[2]);
        } else
            ERROR_SYNTAX;
    }
}
