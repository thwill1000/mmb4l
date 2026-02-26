/*-*****************************************************************************

MMBasic for Linux (MMB4L)

keybuf_windows.c

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
   be displayed on the keybuf at startup (additional copyright messages may
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

// Undefine HRESULT macros that conflict with MMB4L definitions
#undef FAILED
#undef SUCCEEDED

#include "error.h"
#include "keybuf.h"

bool keybuf_isatty(void) {
    return true;
}

void keybuf_pump_tty(void) {
    static HANDLE hStdin = INVALID_HANDLE_VALUE;
    if (hStdin == INVALID_HANDLE_VALUE) {
        hStdin = GetStdHandle(STD_INPUT_HANDLE);
        if (hStdin == INVALID_HANDLE_VALUE) return;
    }

    DWORD available = 0;
    if (!GetNumberOfConsoleInputEvents(hStdin, &available) || available == 0) return;

    // Drain all available events in one call
    while (available-- > 0) {
        INPUT_RECORD record;
        DWORD read_count = 0;
        if (!ReadConsoleInput(hStdin, &record, 1, &read_count) || read_count == 0) break;
        if (record.EventType != KEY_EVENT) continue;
        if (!record.Event.KeyEvent.bKeyDown) continue;
        char ch = record.Event.KeyEvent.uChar.AsciiChar;
        if (ch != 0) keybuf_put(ch);
    }
}
