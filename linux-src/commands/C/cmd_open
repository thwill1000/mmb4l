#include <strings.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/file.h"
#include "../common/parse.h"
#include "../common/serial.h"

static void cmd_open_file(int argc, char **argv) {
    char *filename = getCstring(argv[0]);

    char *mode = NULL;
    if (str_equal(argv[2], "OUTPUT"))
        mode = "wb";  // binary mode so that we do not have lf to cr/lf
                      // translation
    else if (str_equal(argv[2], "APPEND"))
        mode = "ab";  // binary mode is used in MMfopen()
    else if (str_equal(argv[2], "INPUT"))
        mode = "rb";  // note binary mode
    else if (str_equal(argv[2], "RANDOM"))
        mode = "x";  // a special mode for MMfopen()
    else
        ERROR_INVALID("file access mode");

    int fnbr = parse_file_number(argv[4], false);
    if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;

    file_open(filename, mode, fnbr);
}

static void cmd_open_gps(int argc, char **argv) {
    ERROR_UNIMPLEMENTED("OPEN comspec AS GPS");
}

static void cmd_open_serial(int argc, char **argv) {
    char *comspec = getCstring(argv[0]);

    int fnbr = parse_file_number(argv[2], false);
    if (fnbr == -1) ERROR_INVALID_FILE_NUMBER;

    serial_open(comspec, fnbr);
}

/**
 * OPEN fname$ FOR mode AS [#]fnbr
 * OPEN comspec$ AS [#]fnbr
 * OPEN comspec$ AS GPS [,timezone_offset] [,monitor]
 */
void cmd_open(void) {
    char separators[4] = { tokenFOR, tokenAS, ',', '\0' };
    getargs(&cmdline, 7, separators);

    if (argc == 5 && *argv[1] == tokenFOR && *argv[3] == tokenAS) {
        cmd_open_file(argc, argv);
    } else if (argc > 2 && argc < 8 && *argv[1] == tokenAS && strcasecmp(argv[2], "GPS") == 0) {
        cmd_open_gps(argc, argv);
    } else if (argc == 3 && *argv[1] == tokenAS) {
        cmd_open_serial(argc, argv);
    } else {
        ERROR_SYNTAX;
    }
}
