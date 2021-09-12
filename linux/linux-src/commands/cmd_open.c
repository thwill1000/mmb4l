#include <assert.h>

#include "../common/error.h"
#include "../common/file.h"

void cmd_open(void) {
    int file_num;
    char *mode, *fname;
    char ss[3];  // this will be used to split up the argument line

    ss[0] = GetTokenValue("FOR");
    ss[1] = tokenAS;
    ss[2] = 0;
    {  // start a new block
        getargs(
            &cmdline, 5,
            ss);  // getargs macro must be the first executable stmt in a block
        fname = getCstring(argv[0]);
        if (argc == 3 && *argv[1] == tokenAS && toupper(fname[0]) == 'C' &&
            toupper(fname[1]) == 'O' && toupper(fname[2]) == 'M') {
            assert(false);
            // SerialOpen(fname, argv[2]);
            return;
        }

        if (argc != 5) error("Invalid Syntax");
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
            error("Invalid file access mode");
        if (*argv[4] == '#') argv[4]++;
        file_num = getinteger(argv[4]);

        MMfopen(fname, mode, file_num);
    }
}
