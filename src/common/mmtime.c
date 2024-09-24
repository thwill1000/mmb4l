/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmtime.c

Copyright 2021-2022 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

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

void mmtime_date_string(int64_t time_ns, bool localtz, char *buf) {
    const time_t t = NANOSECONDS_TO_SECONDS(time_ns);
    const struct tm *tmbuf = localtz ? localtime(&t) : gmtime(&t);
    sprintf(buf, "%02d-%02d-%04d", tmbuf->tm_mday, tmbuf->tm_mon + 1, tmbuf->tm_year + 1900);
}

void mmtime_time_string(int64_t time_ns, bool localtz, char *buf) {
    const time_t t = NANOSECONDS_TO_SECONDS(time_ns);
    const struct tm *tmbuf = localtz ? localtime(&t) : gmtime(&t);
    sprintf(buf, "%02d:%02d:%02d", tmbuf->tm_hour, tmbuf->tm_min, tmbuf->tm_sec);
}

void mmtime_day_of_week(int64_t time_ns, bool localtz, char* buf) {
    const time_t t = NANOSECONDS_TO_SECONDS(time_ns);
    const struct tm *tmbuf = localtz ? localtime(&t) : gmtime(&t);
    sprintf(buf, "%s", DAYS_OF_WEEK[tmbuf->tm_wday]);
}

void mmtime_sleep_ns(int64_t duration_ns) {
    assert(duration_ns >= 0);
    struct timespec t = { duration_ns / 1000000000, duration_ns % 1000000000 };
    nanosleep(&t, NULL);
}

int64_t mmtime_get_cputime_ns(void) {
    struct timespec now;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
    return SECONDS_TO_NANOSECONDS(now.tv_sec) + (int64_t) now.tv_nsec;
}
