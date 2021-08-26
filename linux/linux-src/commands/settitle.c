#include "../common/console.h"
#include "../common/version.h"

void cmd_settitle(void) {
    console_set_title(getCstring(cmdline));
}
