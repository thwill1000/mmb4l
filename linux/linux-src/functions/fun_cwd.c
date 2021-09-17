#include <unistd.h>

#include "../common/error.h"
#include "../common/global_aliases.h"
#include "../common/parse.h"
#include "../common/version.h"

void fun_cwd(void) {
    g_rtn_type = T_STR;
    g_string_rtn = GetTempStrMemory();

    errno = 0;
    char *result = getcwd(g_string_rtn, STRINGSIZE);
    error_check();

    CtoM(g_string_rtn);
}
