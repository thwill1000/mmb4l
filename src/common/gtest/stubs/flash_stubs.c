/*
 * Copyright (c) 2025-2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include "../../flash.h"

MmResult flash_init() { return kOk; }
MmResult flash_term() { return kOk; }
MmResult flash_get_addr(unsigned index, char **addr) { return kOk; }
MmResult flash_disk_load(unsigned index, const char *filename, bool overwrite) { return kOk; }
