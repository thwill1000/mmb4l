#include "../common/version.h"

int program_load_file(char *filename);

void cmd_run(void) {
    skipspace(cmdline);
    if (*cmdline && *cmdline != '\'') {
        char *filename = getCstring(cmdline);
        if (!program_load_file(filename)) return;
    }

    ClearRuntime();
    WatchdogSet = false;
    PrepareProgram(true);
    IgnorePIN = false;
    if (Option.ProgFlashSize != PROG_FLASH_SIZE)
        ExecuteProgram(
            ProgMemory +
            Option.ProgFlashSize);  // run anything that might be in the library
    if (*ProgMemory != T_NEWLINE) return;  // no program to run
    nextstmt = ProgMemory;
}