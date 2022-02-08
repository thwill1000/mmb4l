#include "../common/option.h"
#include "../common/utility.h"
#include "../common/version.h"

void cmd_new(void) {
//    if(CurrentLinePtr) error("Invalid in a program");
    checkend(cmdline);
    ClearSavedVars();                                               // clear any saved variables
    FlashWriteInit(ProgMemory, Option.ProgFlashSize);
    ClearProgram();
    WatchdogSet = false;
    Option.Autorun = false;
    int result = options_save(&Option, OPTIONS_FILE_NAME);
    if (FAILED(result)) {
        char buf[STRINGSIZE];
        sprintf(buf, "Warning: failed to save options: %d", result);
        MMPrintString(buf);
    }
    longjmp(mark, JMP_NEW);
}
