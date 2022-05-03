/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmtime.h

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

#if !defined(MMTIME_H)
#define MMTIME_H

#include <stdint.h>
#include <time.h>

#define MILLISECONDS_TO_NANOSECONDS(x)  ((int64_t) x * 1000000L)
#define NANOSECONDS_TO_MILLISECONDS(x)  ((int64_t) x / 1000000L)
#define NANOSECONDS_TO_SECONDS(x)       ((int64_t) x / 1000000000L)
#define SECONDS_TO_NANOSECONDS(x)       ((int64_t) x * 1000000000L)

extern const struct timespec ONE_MICROSECOND;
extern const struct timespec ONE_MILLISECOND;

void mmtime_init(void);

/**
 * Gets the number of nanoseconds elapsed since the epoch
 * (00:00:00 on January 1, 1970, Coordinated Universal Time.)
 */
int64_t mmtime_now_ns();

/** Gets the current value of the Timer in nanoseconds. */
int64_t mmtime_get_timer_ns(void);

/** Sets the Timer to the given value in nanoseconds. */
void mmtime_set_timer_ns(int64_t timer_ns);

/**
 * Converts nanoseconds elapsed since the epoch into a date 'DD-MM-YYYY'
 * in the local time zone.
 */
void mmtime_date_string(int64_t time_ns, char *buf);

/**
 * Converts nanoseconds elapsed since the epoch into a time 'HH:MM:SS'
 * in the local time zone.
 */
void mmtime_time_string(int64_t time_ns, char *buf);

/**
 * Converts nanoseconds elapsed since the epoch into a day
 * in the GMT time zone.
 */
void mmtime_day_of_week(int64_t time_ns, char* buf);

/** Sleeps for a given number of nanoseconds. */
void mmtime_sleep_ns(int64_t duration_ns);

#endif
