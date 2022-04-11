#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/file.h"

void cmd_copy(void) {  // thanks to Bryan Rentoul for the contribution
    char *oldf, *newf, ss[2];
    char c;
    int of, nf;

    ss[0] = tokenTO;  // this will be used to split up the argument line
    ss[1] = 0;
    {
        getargs(&cmdline, 3, ss);    // getargs macro must be the first executable stmt in a block
        if (argc != 3) ERROR_SYNTAX;
        oldf = getCstring(argv[0]);  // get the old file name and convert to a
                                     // standard C string
        newf = getCstring(argv[2]);  // get the new file name and convert to a
                                     // standard C string

        of = file_find_free();
        file_open(oldf, "r", of);

        nf = file_find_free();
        file_open(newf, "w", nf);  // We'll just overwrite any existing file
    }

    while (1) {
        if (file_eof(of)) break;
        c = file_getc(of);
        file_putc(c, nf);
    }

    file_close(of);
    file_close(nf);
}
