#include <time.h>

#include "../common/version.h"

void fun_date(void) {
    time_t time_of_day;
    struct tm *tmbuf;

    time_of_day = time(NULL);
    tmbuf = localtime(&time_of_day);
    sret = GetTempStrMemory();  // this will last for the life of the command
    sprintf(sret, "%02d-%02d-%04d", tmbuf->tm_mday, tmbuf->tm_mon + 1,
            tmbuf->tm_year + 1900);
    CtoM(sret);
    targ = T_STR;
}
