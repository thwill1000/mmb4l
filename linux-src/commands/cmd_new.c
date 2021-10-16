#include "../common/version.h"

void cmd_new(void) {
//    if(CurrentLinePtr) error("Invalid in a program");
    checkend(cmdline);
    ClearSavedVars();                                               // clear any saved variables
    FlashWriteInit(ProgMemory, Option.ProgFlashSize);
    ClearProgram();
    WatchdogSet = false;
    Option.Autorun = false;
    SaveOptions();
    longjmp(mark, JMP_NEW);
}
