/*-*****************************************************************************

MMBasic for Linux (MMB4L)

console_linux.c

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
   be displayed on the console at startup (additional copyright messages may
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

#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>

#include "console_private.h"
#include "error.h"
#include "mmtime.h"

static struct termios orig_termios;

static ConsoleState *self;

static MmResult console_install_winch_signal_handler(void);

MmResult console_private_init(ConsoleState *_self) {
    self = _self;
    return console_install_winch_signal_handler();
}

void console_disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void console_enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &orig_termios);
    // atexit(console_disable_raw_mode); - done in main.c
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0; // 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    //fcntl(STDIN_FILENO, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
}

static void handle_winch(int sig) {
    self->requires_sync = true;
}

static MmResult console_install_winch_signal_handler(void) {
    // Install signal handler for window size changes.
    struct sigaction sa;
    sa.sa_handler = handle_winch;
    ON_FAILURE_RETURN(sigemptyset(&sa.sa_mask));
    sa.sa_flags = 0;
    ON_FAILURE_RETURN(sigaction(SIGWINCH, &sa, NULL));

    return kOk;
}

MmResult console_sync_size(int timeout_ms) {
    static int safe_width = 80;
    static int safe_height = 40;
    struct winsize ws= { 0 };
    int fd = open("/dev/tty", O_RDWR);
    if (fd >= 0) {
        int64_t timeout_ns = mmtime_now_ns() + MILLISECONDS_TO_NANOSECONDS(timeout_ms);
        do {
            // Alternatively consider: ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)
            if (SUCCEEDED(ioctl(fd, TIOCGWINSZ, &ws)) && ws.ws_col > 0) break;
            nanosleep(&ONE_MICROSECOND, NULL);
        } while (mmtime_now_ns() < timeout_ns);
        close(fd);
    }

    if (ws.ws_col > 0) {
        // Success.
        safe_width = ws.ws_col;
        safe_height = ws.ws_row;
    }

    // NOTE: Previously when the console size could not be determined this
    //       function would return a failure and "all hell would break loose" with
    //       endless "Cannot determine terminal size" errors being reported.
    //       Now we return the last successful values determined, or 80x40.
    self->width = safe_width;
    self->height = safe_height;

    return kOk;
}
