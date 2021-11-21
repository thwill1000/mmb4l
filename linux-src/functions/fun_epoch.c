#include <stdlib.h>

#include "../common/error.h"
#include "../common/mmtime.h"
#include "../common/version.h"

void fun_epoch(void) {
    if (checkstring(ep, "NOW")) {
        targ = T_INT;
        iret = NANOSECONDS_TO_SECONDS(mmtime_now_ns());
        return;
    }

    char *arg = getCstring(ep);
    getargs(&arg, 11, "-/ :");
    if (argc != 11) ERROR_SYNTAX;

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

    int h = atoi(argv[6]);
    int min = atoi(argv[8]);
    int s = atoi(argv[10]);
    if (h < 0 || h > 23 || min < 0 || m > 59 || s < 0 || s > 59) ERROR_INVALID("time");

    struct tm tmbuf = { 0 };
    tmbuf.tm_year = y - 1900;
    tmbuf.tm_mon = m - 1;
    tmbuf.tm_mday = d;
    tmbuf.tm_hour = h;
    tmbuf.tm_min = min;
    tmbuf.tm_sec = s;
    targ = T_INT;
    iret = timegm(&tmbuf);
}
