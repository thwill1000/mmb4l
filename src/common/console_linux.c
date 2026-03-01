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

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "console_private.h"
#include "error.h"
#include "keybuf.h"
#include "mmtime.h"

enum ReadCursorPositionState {
    EXPECTING_ESCAPE,
    EXPECTING_SQUARE_BRACKET,
    EXPECTING_ROWS,
    EXPECTING_COLS,
    EXPECTING_FINISHED
};

static struct termios orig_termios;
static ConsoleState *self;

static void handle_winch(int sig) {
    self->requires_sync = true;
}

MmResult console_init_platform(ConsoleState *_self) {
    self = _self;

    // Save the original terminal settings so they can be restored on exit,
    // then configure the terminal in "raw" mode:
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    if (isatty(STDIN_FILENO)) { // We don't enable raw mode for piped input
        raw.c_lflag &= ~(ECHO | ICANON | ISIG);
        //               |       |        |
        //               |       |        +-- Disable signal generation (SIGINT, SIGQUIT etc.)
        //               |       |            so Ctrl-C, Ctrl-\ etc. are passed as raw bytes.
        //               |       +----------- Disable canonical mode so input is available
        //               |                    immediately without waiting for a newline.
        //               +------------------- Disable echo so typed characters are not
        //                                    automatically printed to the terminal.
        raw.c_cc[VMIN] = 1;   // Block until at least 1 character is available.
        raw.c_cc[VTIME] = 0;  // No timeout - wait indefinitely for input.
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        //                       |
        //                       +-- Flush any pending input before applying new settings.
    }

    // Install a signal handler for SIGWINCH which is sent by the OS whenever
    // the terminal window is resized, so we can update the console dimensions.
    struct sigaction sa;
    sa.sa_handler = handle_winch;  // Our handler to update width/height.
    errno = 0;
    if (FAILED(sigemptyset(&sa.sa_mask))) RETURN_RESULT(errno);  // No signals blocked during handler.
    sa.sa_flags = 0;
    if (FAILED(sigaction(SIGWINCH, &sa, NULL))) RETURN_RESULT(errno);

    RETURN_RESULT(kOk);
}

MmResult console_term_platform(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    RETURN_RESULT(kOk);
}

void console_putc_raw(char c) {
    (void) putc(c, stdout);
}

void console_putc_raw_n(const char *p, int count) {
    for (int i = 0; i < count; ++i) {
        (void) putc(p[i], stdout);
    }
}

MmResult console_sync_cursor_pos(int timeout_ms) {
    if (!isatty(STDIN_FILENO)) {
        LOG_WARN("cannot read cursor position from non-TTY");
        RETURN_RESULT(kOk);
    }

    // Send escape code to report cursor position.
    keybuf_clear();
    printf("\033[6n");
    fflush(stdout);

    // Read characters one at a time to match the expected pattern ESC[n;mR
    // - fails if the pattern has not been matched within the timeout.
    // - will sleep briefly if there is nothing to read.
    int64_t timeout_ns = mmtime_now_ns() + MILLISECONDS_TO_NANOSECONDS(timeout_ms);
    enum ReadCursorPositionState state = EXPECTING_ESCAPE;
    char buf[32] = { 0 };
    char *p = NULL;
    while (mmtime_now_ns() < timeout_ns && state != EXPECTING_FINISHED) {
        if (state == EXPECTING_ESCAPE) p = buf;
        int ch = keybuf_get();
        if (ch == -1) {
            mmtime_sleep_ns(MICROSECONDS_TO_NANOSECONDS(1));
            continue;
        }
        *(p++) = (char) ch;

        switch (state) {
            case EXPECTING_ESCAPE:
                state = (ch == 0x1B ? EXPECTING_SQUARE_BRACKET : EXPECTING_ESCAPE);
                break;
            case EXPECTING_SQUARE_BRACKET:
                state = (ch == '[' ? EXPECTING_ROWS : EXPECTING_ESCAPE);
                break;
            case EXPECTING_ROWS:
                state = (ch == ';'
                        ? EXPECTING_COLS
                        : (isdigit(ch) ? EXPECTING_ROWS : EXPECTING_ESCAPE));
                break;
            case EXPECTING_COLS:
                state = (ch == 'R'
                        ? EXPECTING_FINISHED
                        : (isdigit(ch) ? EXPECTING_COLS : EXPECTING_ESCAPE));
                break;
            case EXPECTING_FINISHED:
                assert(0);  // Loop should have already exited.
                break;
        }
    }

    if (state == EXPECTING_FINISHED) {
        // Parse output, rows (y) then columns (x).
        *p++ = '\0';
        sscanf(buf, "\033[%d;%dR", &self->y, &self->x);
        self->x--; // adjust to account for VT100 origin being (1,1) not (0,0).
        self->y--;
        return kOk; // Success
    } else {
        return mmresult_ex(kError, "Failed to read TTY cursor position");
    }
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
            mmtime_sleep_ns(MICROSECONDS_TO_NANOSECONDS(1));
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
