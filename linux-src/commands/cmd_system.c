#include <stdbool.h>

#include "../common/error.h"
#include "../common/version.h"

int cmd_system_simple(char *cmd) {
    return system(cmd);
}

int cmd_system_capture_output(char *cmd, char *variable) {
    char *var_ptr = findvar(variable, V_FIND | V_EMPTY_OK);

    if (!(vartbl[VarIndex].type & T_INT)) ERROR_ARGUMENT_NOT_INTEGER_ARRAY("2");
    if (vartbl[VarIndex].dims[1] != 0) ERROR_INVALID("variable");
    if (vartbl[VarIndex].dims[0] <= 0) ERROR_ARGUMENT_NOT_INTEGER_ARRAY("2");

    int size = (vartbl[VarIndex].dims[0] - OptionBase) * 8;
    char *buf = (char *) var_ptr + 8;
    FILE *f = popen(cmd, "r");
    error_check();

    int i;
    for (i = 0; i < size; ++i) {
        int ch = fgetc(f);
        if (ch == EOF) break;
        buf[i] = (char) ch;
    }
    *((int64_t *) var_ptr) = i;

    return pclose(f);
}

void cmd_system(void) {
    getargs(&cmdline, 3, ",");
    int result = 0;
    errno = 0;
    switch (argc) {
        case 1:
            result = cmd_system_simple(getCstring(argv[0]));
            break;
        case 3:
            result = cmd_system_capture_output(getCstring(argv[0]), argv[2]);
            break;
        default:
            ERROR_SYNTAX;
    }

    error_check();
    if (result != 0) {
        error_code(result, "System command failed, exit code [%]", result);
    }
}
