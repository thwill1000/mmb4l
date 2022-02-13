#include <stdio.h>

#include "../common/mmb4l.h"
#include "../common/mmtime.h"

void fun_datetime(void) {
    time_t t;
    if (checkstring(ep, "NOW")) {
        t = NANOSECONDS_TO_SECONDS(mmtime_now_ns());
    } else {
        t = getint(ep, 0x80000000, 0x7FFFFFFF);
    }
    struct tm *tmbuf;
    tmbuf = gmtime(&t);

    targ = T_STR;
    sret = GetTempStrMemory();
    sprintf(
            sret,
            "%02d-%02d-%04d %02d:%02d:%02d",
            tmbuf->tm_mday,
            tmbuf->tm_mon + 1,
            tmbuf->tm_year + 1900,
            tmbuf->tm_hour,
            tmbuf->tm_min,
            tmbuf->tm_sec);
    CtoM(sret);
}
