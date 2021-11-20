#include "../common/error.h"
#include "../common/program.h"
#include "../common/utility.h"
#include "../common/version.h"

void cmd_load(void) {
    getargs(&cmdline, 1, " ,");
    if (argc == 1) {
        char *filename = getCstring(argv[0]);
        // Never expected to return failure - reports its own ERROR and calls longjmp().
        if (FAILED(program_load_file(filename))) ERROR_INTERNAL_FAULT;
    } else {
        ERROR_SYNTAX;
    }
}
