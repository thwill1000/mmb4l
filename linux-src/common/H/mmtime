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
