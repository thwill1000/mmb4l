#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/program.h"
#include "../common/utility.h"

#define ERROR_NOTHING_TO_RUN  error_throw_ex(kError, "Nothing to run")

char run_cmdline[STRINGSIZE];

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
        ERROR_NOTHING_TO_RUN;
        return;
    }

    if (FAILED(program_load_file(filename))) return;

    ClearRuntime();
    WatchdogSet = false;
    PrepareProgram(true);
    IgnorePIN = false;
    if (mmb_options.prog_flash_size != PROG_FLASH_SIZE)
        ExecuteProgram(
                ProgMemory +
                mmb_options.prog_flash_size);  // run anything that might be in the library
    if (*ProgMemory != T_NEWLINE) return;  // no program to run
    nextstmt = ProgMemory;
}
