#include <errno.h>
#include <unistd.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/parse.h"

void fun_cwd(void) {
    g_rtn_type = T_STR;
    g_string_rtn = GetTempStrMemory();

    errno = 0;
    if (!getcwd(g_string_rtn, STRINGSIZE)) error_throw(errno);

    CtoM(g_string_rtn);
}
