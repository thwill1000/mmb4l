/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <string.h>

#include "error_stubs.h"

char error_msg[256];

// Defined in "common/error.c"
void error_init(ErrorState *error_state) { }

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
