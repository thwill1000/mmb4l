#include "../common/version.h"

char *MMgetcwd(void);

void fun_cwd(void) {
    sret = CtoM(MMgetcwd());
    targ = T_STR;
}
