#include "../common/version.h"

char run_cmdline[STRINGSIZE];

int program_load_file(char *filename); // program.c

void cmd_run(void) {

    // Save the cmdline for later.
    skipspace(cmdline);
    memset(run_cmdline, 0, STRINGSIZE);
    memcpy(run_cmdline, cmdline, strlen(cmdline));

    char *filename = NULL;
    if (*cmdline != '\0') {
        filename = getCstring(cmdline);
    } else if (*CurrentFile != '\0') {
        filename = CurrentFile;
    } else {
        error("Nothing to run");
        return;
    }

    if (!program_load_file(filename)) return;

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
