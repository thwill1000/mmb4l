#include "../common/error.h"
#include "../common/version.h"

void cmd_rename(void) {
    char *oldf, *newf, ss[2];
    ss[0] = tokenAS;  // this will be used to split up the argument line
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
        errno = 0;
        rename(oldf, newf);
        if (error_check()) return;
    }
}
