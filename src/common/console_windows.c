/*-*****************************************************************************

MMBasic for Linux (MMB4L)

console_windows.c

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

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

// Undefine HRESULT macros that conflict with MMB4L definitions
#undef FAILED
#undef SUCCEEDED

#include "console_private.h"
#include "error.h"

static ConsoleState *self;
static DWORD original_stdout_mode = 0;
static DWORD original_stdin_mode = 0;

MmResult console_init_platform(ConsoleState *_self) {
    self = _self;

    // Save original modes
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(hStdout, &original_stdout_mode);

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &original_stdin_mode);

    // Enable VT100 output processing
    SetConsoleMode(hStdout, original_stdout_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    // Enable VT100 input processing
    SetConsoleMode(hStdin, original_stdin_mode | ENABLE_VIRTUAL_TERMINAL_INPUT);

    // Binary mode to prevent \n -> \r\n translation
    _setmode(_fileno(stdout), _O_BINARY);

    return kOk;
}

MmResult console_term_platform(void) {
    // Restore original console modes
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(hStdout, original_stdout_mode);

    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hStdin, original_stdin_mode);

    // Restore text mode translation
    _setmode(_fileno(stdout), _O_TEXT);

    return kOk;
}

void console_disable_raw_mode(void) {
    // Do nothing, we are handling it in console_term_platform()
}

void console_enable_raw_mode(void) {
    // Do nothing, we are handling it in console_init_platform()
}

MmResult console_sync_cursor_pos(int timeout_ms) {
    (void) timeout_ms;  // Unused by Windows implementation

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) {
        return mmresult_ex(kError, "Failed to get console handle");
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return mmresult_ex(kError, "Failed to read console cursor position");
    }

    self->x = csbi.dwCursorPosition.X;
    self->y = csbi.dwCursorPosition.Y;
    return kOk;
}

MmResult console_sync_size(int timeout_ms) {
    static int safe_width = 80;
    static int safe_height = 40;

    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hStdout, &csbi)) {
            int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            if (width > 0 && height > 0) {
                safe_width = width;
                safe_height = height;
            }
        }
    }

    self->width = safe_width;
    self->height = safe_height;

    return kOk;
}
