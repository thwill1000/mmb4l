#include <assert.h>
#include <stdio.h>

#include "mmtime.h"

const struct timespec ONE_MICROSECOND = { 0, 1000 };
const struct timespec ONE_MILLISECOND = { 0, 1000000 };

const char *DAYS_OF_WEEK[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

int64_t mmtime_base_ns;

void mmtime_init(void) {
    mmtime_base_ns = mmtime_now_ns();
}

int64_t mmtime_now_ns() {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return SECONDS_TO_NANOSECONDS(now.tv_sec) + (int64_t) now.tv_nsec;
}

int64_t mmtime_get_timer_ns(void) {
    return mmtime_now_ns() - mmtime_base_ns;
}

void mmtime_set_timer_ns(int64_t timer_ns) {
    mmtime_base_ns = mmtime_now_ns() - timer_ns;
}

void mmtime_date_string(int64_t time_ns, char *buf) {
    time_t t = NANOSECONDS_TO_SECONDS(time_ns);
    struct tm *tmbuf;
    tmbuf = localtime(&t);
    sprintf(buf, "%02d-%02d-%04d", tmbuf->tm_mday, tmbuf->tm_mon + 1, tmbuf->tm_year + 1900);
}

void mmtime_time_string(int64_t time_ns, char *buf) {
    time_t t = NANOSECONDS_TO_SECONDS(time_ns);
    struct tm *tmbuf;
    tmbuf = localtime(&t);
    sprintf(buf, "%02d:%02d:%02d", tmbuf->tm_hour, tmbuf->tm_min, tmbuf->tm_sec);
}

void mmtime_day_of_week(int64_t time_ns, char* buf) {
    time_t t = NANOSECONDS_TO_SECONDS(time_ns);
    struct tm *tmbuf = gmtime(&t);
    sprintf(buf, "%s", DAYS_OF_WEEK[tmbuf->tm_wday]);
}

void mmtime_sleep_ns(int64_t duration_ns) {
    assert(duration_ns >= 0);
    struct timespec t = { duration_ns / 1000000000, duration_ns % 1000000000 };
    nanosleep(&t, NULL);
}
