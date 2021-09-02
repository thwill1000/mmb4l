#include "../common/version.h"

int program_load_file(char *filename);

void cmd_reload(void) {
    getargs(&cmdline, 0, " ,");
    if (CurrentLinePtr != NULL) error("Invalid in a program");
    if (CurrentFile[0] == '\0') {
        MMPrintString("Nothing to reload\r\n");
    } else {
        program_load_file(CurrentFile);
    }
}
