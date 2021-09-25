#include "../common/version.h"

void cmd_copy(void) {  // thanks to Bryan Rentoul for the contribution
    char *oldf, *newf, ss[2];
    char c;
    int of, nf;

    ss[0] = tokenTO;  // this will be used to split up the argument line
    ss[1] = 0;
    {
        getargs(
            &cmdline, 3,
            ss);  // getargs macro must be the first executable stmt in a block
        if (argc != 3) error("Invalid syntax");
        oldf = getCstring(argv[0]);  // get the old file name and convert to a
                                     // standard C string
        newf = getCstring(argv[2]);  // get the new file name and convert to a
                                     // standard C string

        of = FindFreeFileNbr();
        MMfopen(oldf, "r", of);

        nf = FindFreeFileNbr();
        MMfopen(newf, "w", nf);  // We'll just overwrite any existing file
    }
    while (1) {
        if (MMfeof(of)) break;
        c = MMfgetc(of);
        MMfputc(c, nf);
    }
    MMfclose(of);
    MMfclose(nf);
}
