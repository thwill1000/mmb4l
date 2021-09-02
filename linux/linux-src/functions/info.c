#include <sys/stat.h>

#include "../common/global_aliases.h"
#include "../common/utility.h"
#include "../common/version.h"

void info_filesize(char *p) {
    char *path = getCstring(p);
    char canonical[STRINGSIZE];
    canonicalize_path(path, canonical, STRINGSIZE - 1);
    struct stat st;
    stat(canonical, &st);

    // TODO: error handling

    g_integer_rtn = st.st_size;
    g_rtn_type = T_INT;
}

void info_option(char *p) {
    if (checkstring(p, "BASE")) {
        g_integer_rtn = (OptionBase == 1) ? 1 : 0;
        g_rtn_type = T_INT;
    } else if (checkstring(p, "BREAK")) {
		g_integer_rtn = g_break_key;
		g_rtn_type = T_INT;
    } else {
        error("Syntax");
    }
}

void fun_info(void) {
    // char *tp;
    // sret = GetTempStrMemory();  // this will last for the life of the command
    // char rettype = 'S';
    // tp = checkstring(ep, "FILESIZE");
    char *p;
    if (p = checkstring(ep, "FILESIZE")) {
        info_filesize(p);
    } else if (p = checkstring(ep, "OPTION")) {
        info_option(p);
    } else {
        error("Syntax");
    }

    // if (rettype == 'S') {
    //     CtoM(sret);
    //     targ = T_STR;
    // } else if (rettype == 'N') {
    //     targ = T_NBR;
    // } else {
    //     targ = T_INT;
    // }
}
