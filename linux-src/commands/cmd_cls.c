#include "../common/console.h"
#include "../common/error.h"
#include "../common/parse.h"
#include "../common/version.h"

// See definition in draw.c
#if 0
void cmd_cls(void) {
    skipspace(cmdline);
    if (!parse_is_end(cmdline)) ERROR_SYNTAX;
    console_clear();
}
#endif
