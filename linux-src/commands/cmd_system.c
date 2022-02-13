#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common/mmb4l.h"
#include "../common/error.h"

static int cmd_system_simple(char *cmd) {
    return system(cmd);
}

/**
 * Executes a system command and captures its STDOUT in a buffer.
 *
 * @param  cmd  system command to execute.
 * @param  buf  buffer to capture output in, this will not be '\0' terminated.
 * @param  sz   on entry size of buffer,
 *              on return number of characters in buffer.
 * @return      exit code of the executed system command.
 */
static int cmd_system_capture(char *cmd, char *buf, size_t *sz) {
    FILE *f = popen(cmd, "r");
    error_check();

    int i;
    for (i = 0; i < *sz; ++i) {
        int ch = fgetc(f);
        if (ch == EOF) break;
        buf[i] = (char) ch;
    }

    // Trim trailing whitespace and non-ASCII garbage.
    while (buf[i] < 33 || buf[i] > 126) i--;

    *sz = i + 1;

    return pclose(f);
}

static int cmd_system_capture_output_in_string(char *cmd, char *var_ptr) {
    size_t sz = MAXSTRLEN;
    int result = cmd_system_capture(cmd, var_ptr + 1, &sz);
    *var_ptr = sz;
    return result;
}

static int cmd_system_capture_output_in_long_string(char *cmd, char *var_ptr) {
    size_t sz = (vartbl[VarIndex].dims[0] - OptionBase) * 8;
    char *buf = (char *) var_ptr + 8;
    int result = cmd_system_capture(cmd, buf, &sz);
    *((int64_t *) var_ptr) = sz;
    return result;
}

int cmd_system_capture_output(char *cmd, char *variable) {
    char *var_ptr = findvar(variable, V_FIND | V_EMPTY_OK);

    if (vartbl[VarIndex].type & T_STR) {
        if (vartbl[VarIndex].dims[0] == 0) {
            return cmd_system_capture_output_in_string(cmd, var_ptr);
        }
    } else if (vartbl[VarIndex].type & T_INT) {
        if ((vartbl[VarIndex].dims[0] > 0)
                && (vartbl[VarIndex].dims[1] == 0)) {
            return cmd_system_capture_output_in_long_string(cmd, var_ptr);
        }
    }

    ERROR_INVALID("2nd argument; expected string or long string");
    return -1;
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
