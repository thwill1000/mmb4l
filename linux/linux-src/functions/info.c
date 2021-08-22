#include <sys/stat.h>

#include "../common/version.h"

extern void sanitise_path(const char* src, char* dst);

void fun_info(void) {
    char *tp;
    sret = GetTempStrMemory();  // this will last for the life of the command
    char rettype = 'S';
    tp = checkstring(ep, "FILESIZE");
    if (tp) {
        char *p = getCstring(tp);

        char sane_path[STRINGSIZE];
        sanitise_path(p, sane_path);
        struct stat st;
        stat(sane_path, &st);

        // TODO: error handling

        iret = st.st_size;
        targ = T_INT;
        return;
    } else {
        error("Syntax");
    }

    if (rettype == 'S') {
        CtoM(sret);
        targ = T_STR;
    } else if (rettype == 'N') {
        targ = T_NBR;
    } else {
        targ = T_INT;
    }
}
