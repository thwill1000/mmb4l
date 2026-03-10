/*
 * Copyright (c) 2024-2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <string.h>

#include "error_stubs.h"

char error_msg[256];

MmResult error_init(ErrorState *error_state) { return kOk; }

void error_get_line_and_file(int *line, char *file_path) { }

MmResult error_throw(MmResult result) {
    return error_throw_ex(result, mmresult_to_string(result));
}

MmResult error_throw_ex(MmResult result, const char *msg, ...) {
    strcpy(error_msg, msg);
    return result;
}

MmResult error_throw_legacy(const char *msg, ...) {
    strcpy(error_msg, msg);
    return kOk;
}

void error_set_callback(void (*fn)(void *), void *data) { }

void error_clear_callback() { }
