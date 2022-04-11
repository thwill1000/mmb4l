#include "../common/mmb4l.h"
#include "../common/console.h"
#include "../common/error.h"
#include "../common/parse.h"

void cmd_cls(void) {
    skipspace(cmdline);
    if (!parse_is_end(cmdline)) ERROR_SYNTAX;
    console_clear();
}
