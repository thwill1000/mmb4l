#include "../common/version.h"

void cmd_inc(void) {
    char *p, *q;
    int vtype;
    getargs(&cmdline, 3, ",");
    if (argc == 1) {
        p = findvar(argv[0], V_FIND);
        if (vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        vtype = TypeMask(vartbl[VarIndex].type);
        if (vtype & T_STR) error("Invalid variable");  // sanity check
        if (vtype & T_NBR)
            (*(MMFLOAT *)p) = (*(MMFLOAT *)p) + 1.0;
        else if (vtype & T_INT)
            *(long long int *)p = *(long long int *)p + 1;
        else
            error("Syntax");
    } else {
        p = findvar(argv[0], V_FIND);
        if (vartbl[VarIndex].type & T_CONST) error("Cannot change a constant");
        vtype = TypeMask(vartbl[VarIndex].type);
        if (vtype & T_STR) {
            q = getstring(argv[2]);
            if (*p + *q > MAXSTRLEN) error("String too long");
            Mstrcat(p, q);
        } else if (vtype & T_NBR) {
            (*(MMFLOAT *)p) = (*(MMFLOAT *)p) + getnumber(argv[2]);
        } else if (vtype & T_INT) {
            *(long long int *)p = *(long long int *)p + getinteger(argv[2]);
        } else
            error("syntax");
    }
}
