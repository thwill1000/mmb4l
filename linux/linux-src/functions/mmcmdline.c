#include "../common/global_aliases.h"
#include "../common/utility.h"
#include "../common/version.h"

extern char run_cmdline[STRINGSIZE];

void fun_cmdline(void) {
    char *p = run_cmdline;
    skipspace(p);

    if (*p == 34) {
        do {
            p++;
        } while (*p != 34);
        p++;
        skipspace(p);
        if (*p == ',') {
            p++;
            skipspace(p);
        }
    }

    char *q;
    if (q = strchr(p, '|')) {
        q--;
        *q = 0;
    }

    g_string_rtn = GetTempStrMemory();
    strcpy(g_string_rtn, p);
    CtoM(g_string_rtn);
    g_rtn_type = T_STR;
}
