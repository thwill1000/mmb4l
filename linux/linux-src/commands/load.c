#include "../common/version.h"

int program_load_file(char *filename);

void cmd_load(void) {
    getargs(&cmdline, 1, " ,");
    if (argc == 1) {
        char *filename = getCstring(argv[0]);
        program_load_file(filename);
    } else {
        error("Syntax");
    }
}
