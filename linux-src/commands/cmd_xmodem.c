#include "../common/error.h"
#include "../common/parse.h"
#include "../common/version.h"
#include "../common/xmodem.h"

// TODO: Disable and restore break key ?

void cmd_xmodem(void) {
    char *p;
    bool receive;
    if ((p = checkstring(cmdline, "RECEIVE"))) {
        receive = true;
    } else if ((p = checkstring(cmdline, "SEND"))) {
        receive = false;
    } else {
        ERROR_SYNTAX;
    }

    char *filename;
    int serial_fnbr;

    {
        getargs(&p, 3, ",");  // Macro needs to start a new block.
        if (argc != 3) ERROR_ARGUMENT_COUNT;

        filename = getCstring(argv[0]);
        serial_fnbr = parse_file_number(argv[2], false);
        if (serial_fnbr == -1) ERROR_INVALID_FILE_NUMBER;
    }

    int file_fnbr = file_find_free();
    file_open(filename, receive ? "wb" : "rb", file_fnbr);

    if (receive) {
        xmodem_receive(file_fnbr, serial_fnbr);
    } else {
        xmodem_send(file_fnbr, serial_fnbr);
    }

    file_close(file_fnbr);
}
