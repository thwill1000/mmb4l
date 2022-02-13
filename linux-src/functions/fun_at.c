#include "../common/mmb4l.h"
#include "../common/console.h"
#include "../common/error.h"

void cmd_print(void);

void fun_at(void) {
    getargs(&ep, 3, ",");

    if (commandfunction(cmdtoken) != cmd_print) ERROR_INVALID("function");
    if (argc != 3) ERROR_ARGUMENT_COUNT;

    int64_t x = getinteger(argv[0]);
    int64_t y = getinteger(argv[2]);

    if (x < 0) ERROR_INVALID("x-coordinate");
    if (y < 0) ERROR_INVALID("y-coordinate");

    console_set_cursor_pos((int) x, (int) y);

    targ = T_STR;
    sret = GetTempStrMemory();
    sret[0] = '\0';
}
