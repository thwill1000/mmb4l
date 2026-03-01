/*-*****************************************************************************

MMBasic for Linux (MMB4L)

mmtime_windows.c

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
#include <stdint.h>
#include <windows.h>

// Undefine HRESULT macros to avoid conflicts with our own definitions
#undef FAILED
#undef SUCCEEDED

#include "logger.h"
#include "mmtime.h"

int64_t mmtime_now_ns() {
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    // Combine high and low parts into a single 64-bit value (in 100-nanosecond intervals)
    int64_t intervals = ((int64_t) ft.dwHighDateTime << 32) | (uint64_t) ft.dwLowDateTime;
    // Convert from Windows epoch (1601-01-01) to Unix epoch (1970-01-01)
    intervals -= 116444736000000000LL;
    // Convert from 100-nanosecond intervals to nanoseconds
    return intervals * 100;
}

void mmtime_sleep_ns(int64_t duration_ns) {
    assert(duration_ns >= 0);

    // Create a waitable timer for sub-millisecond precision
    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (timer) {
        // Timer uses 100-nanosecond intervals, negative value means relative time
        LARGE_INTEGER due_time;
        due_time.QuadPart = -(duration_ns / 100);
        SetWaitableTimer(timer, &due_time, 0, NULL, NULL, FALSE);
        WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    } else {
        LOG_DEBUG("failed to create waitable timer, falling back to Sleep() with millisecond precision");
        Sleep((DWORD)(duration_ns / 1000000));
    }
}

int64_t mmtime_get_cputime_ns(void) {
    FILETIME creation_time, exit_time, kernel_time, user_time;
    GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time);
    // Combine kernel and user time (both in 100-nanosecond intervals)
    ULARGE_INTEGER k, u;
    k.LowPart  = kernel_time.dwLowDateTime;
    k.HighPart = kernel_time.dwHighDateTime;
    u.LowPart  = user_time.dwLowDateTime;
    u.HighPart = user_time.dwHighDateTime;
    // Convert from 100-nanosecond intervals to nanoseconds
    return (int64_t)(k.QuadPart + u.QuadPart) * 100;
}

time_t mmtime_timegm(struct tm *t) {
    return _mkgmtime(t);
}
