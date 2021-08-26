#include <time.h>

#include "../common/version.h"

void fun_time(void) {
    time_t time_of_day;
    struct tm *tmbuf;

    time_of_day = time(NULL);
    tmbuf = localtime(&time_of_day);
    sret = GetTempStrMemory();  // this will last for the life of the command
    sprintf(sret, "%02d:%02d:%02d", tmbuf->tm_hour, tmbuf->tm_min,
            tmbuf->tm_sec);
    CtoM(sret);
    targ = T_STR;
}
