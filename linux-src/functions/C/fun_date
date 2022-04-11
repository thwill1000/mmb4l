#include "../common/mmb4l.h"
#include "../common/mmtime.h"

void fun_date(void) {
    targ = T_STR;
    sret = GetTempStrMemory();
    mmtime_date_string(mmtime_now_ns(), sret);
    CtoM(sret);
}
