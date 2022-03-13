#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/parse.h"

/**
 * @brief  Reads value of an environment variable into a buffer.
 *
 * The value read into the buffer does not include a trailing '\0'.
 *
 * @param[in] name     Name of the environment variable.
 * @param[out] buf     Buffer to read value into.
 * @param[in, out] sz  On entry the size of the buffer.
 *                     On exit the number of characters read into the buffer.
 */
static MmResult cmd_system_getenv_to_buf(const char *name, char *buf, size_t *sz) {
    char *value = getenv(name);
    if (!value) value = "";
    size_t len = strlen(value);
    if (len > *sz) return kStringTooLong;
    memcpy(buf, value, len);
    *sz = len;
    return kOk;
}

/**
 * @brief  Implements SYSTEM GETENV sub-command.
 *
 * SYSTEM GETENV name$, value$
 * SYSTEM GETENV name$, value%()
 */
static void cmd_system_getenv(char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) ERROR_SYNTAX;

    // Name of environment variable to query.
    char *name = getCstring(p);

    // Determine where to capture the value.
    char *value_var_ptr = findvar(argv[2], V_FIND | V_EMPTY_OK);
    char *buf = NULL;
    size_t buf_sz = 0;
    if ((vartbl[VarIndex].type & T_STR)
            &&  (vartbl[VarIndex].dims[0] == 0)) {
        // Capture value in STRING variable.
        buf = value_var_ptr + 1;
        buf_sz = STRINGSIZE - 1;
    } else if ((vartbl[VarIndex].type & T_INT)
            && (vartbl[VarIndex].dims[0] > 0)
            && (vartbl[VarIndex].dims[1] == 0)) {
        // Capture value in LONGSTRING variable.
        buf = (char *) value_var_ptr + 8;
        buf_sz = (vartbl[VarIndex].dims[0] - OptionBase) * 8;
    } else {
        ERROR_INVALID("2nd argument; expected STRING or LONGSTRING");
    }

    MmResult result = cmd_system_getenv_to_buf(name, buf, &buf_sz);

    switch (result) {
        case kOk:
            if (buf == value_var_ptr + 1) {
                // Set size of STRING variable.
                *value_var_ptr = buf_sz;
            } else {
                // Set size of LONGSTRING variable.
                *((int64_t *) value_var_ptr) = buf_sz;
            }
            break;
        case kStringTooLong:
            error("Environment variable value too long");
            break;
        default:
            error_system(result);
            break;
    }
}

/**
 * @brief  Implements SYSTEM SETENV sub-command.
 *
 * SYSTEM SETENV name$, value$
 * SYSTEM SETENV name$, value%()
 */
static void cmd_system_setenv(char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) ERROR_SYNTAX;

    char *name = getCstring(p);
    char *value = NULL;
    MmResult result = kSyntax;

    // Is the value a LONGSTRING ?
    if (parse_matches_longstring_pattern(argv[2])) {
        char *var_ptr = findvar(argv[2], V_FIND | V_EMPTY_OK);
        if (vartbl[VarIndex].type & T_INT
                && (vartbl[VarIndex].dims[0] > 0)
                && (vartbl[VarIndex].dims[1] == 0)) {
            int64_t sz = *((int64_t *) var_ptr);
            value = GetTempMemory(sz + 1);
            memcpy(value, var_ptr + 8, sz);
            value[sz] = 0;
            result = setenv(name, value, 1);
            ClearSpecificTempMemory(value);
        }
    }

    if (result != kOk) {
        value = getCstring(argv[2]);
        result = setenv(name, value, 1);
    }

    if (result != kOk) error_system(result);
}

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

/**
 * @brief  Implements base SYSTEM command.
 *
 * SYSTEM command$ [, output$   [, exit_code%]]
 * SYSTEM command$ [, output%() [, exit_code%]]
 */
void cmd_system_execute(char *p) {
    getargs(&p, 3, ",");
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

void cmd_system(void) {
    char* p;

    if ((p = checkstring(cmdline, "GETENV"))) {
        cmd_system_getenv(p);
    } else if ((p = checkstring(cmdline, "SETENV"))) {
        cmd_system_setenv(p);
    } else {
        cmd_system_execute(cmdline);
    }
}
