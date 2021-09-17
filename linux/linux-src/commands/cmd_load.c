#include "../common/error.h"
#include "../common/program.h"
#include "../common/version.h"

void cmd_load(void) {
    getargs(&cmdline, 1, " ,");
    if (argc == 1) {
        char *filename = getCstring(argv[0]);
        program_load_file(filename);
    } else {
        ERROR_SYNTAX;
    }
}
