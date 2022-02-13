#include "../common/mmb4l.h"
#include "../common/exit_codes.h"

void cmd_end(void) {
    getargs(&cmdline, 1, ",");
    mmb_exit_code = (argc == 1) ? getint(argv[0], 0, 255) : EX_OK;
    longjmp(mark, JMP_END);
}
