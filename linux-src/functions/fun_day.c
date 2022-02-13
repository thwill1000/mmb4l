#include <stdlib.h>

#include "../common/mmb4l.h"
#include "../common/error.h"
#include "../common/mmtime.h"

void fun_day(void) {
    int64_t time;

    if (checkstring(ep, "NOW")) {
        time = mmtime_now_ns();
    } else {
        char *arg = getCstring(ep);
        getargs(&arg, 5, "-/");  // this is a macro and must be the first
                                 // executable stmt in a block
        if (argc != 5) ERROR_SYNTAX;
        int d = atoi(argv[0]);
        int m = atoi(argv[2]);
        int y = atoi(argv[4]);
        if (d > 1000) {
            int tmp = d;
            d = y;
            y = tmp;
        }
        if (y >= 0 && y < 100) y += 2000;
        if (d < 1 || d > 31 || m < 1 || m > 12 || y < 1902 || y > 2999) ERROR_INVALID("date");
        struct tm tmbuf = { 0 };
        tmbuf.tm_year = y - 1900;
        tmbuf.tm_mon = m - 1;
        tmbuf.tm_mday = d;
        time = SECONDS_TO_NANOSECONDS(timegm(&tmbuf));
    }

    targ = T_STR;
    sret = GetTempStrMemory();
    mmtime_day_of_week(time, sret);
    CtoM(sret);
}
