/*-*****************************************************************************

MMBasic for Linux (MMB4L)

keybuf.c

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

#include <assert.h>
#include <stdio.h>

#include <SDL.h>

#include "interrupt.h"
#include "keybuf.h"
#include "keycodes.h"
#include "logger.h"
#include "options.h"
#include "mmb4l.h"
#include "rx_buf.h"

#define KEYBUF_SIZE 256

static char keybuf_data[KEYBUF_SIZE];
static RxBuf keybuf_buf;
static SDL_mutex *keybuf_mutex = NULL;
static SDL_Thread *keybuf_thread = NULL;
static SDL_atomic_t keybuf_stop;

/**
 * Platform-specific function to read a single character from the terminal.
 * Implemented in keybuf_linux.c and keybuf_windows.c.
 * Should block until a character is available or keybuf_stop is set.
 * Returns the character read, or -1 on error or if keybuf_stop is set.
 */
int keybuf_read_char(void);

static int keybuf_thread_fn(void *data) {
    (void) data;
    while (!SDL_AtomicGet(&keybuf_stop)) {
        int ch = keybuf_read_char();
        if (ch == -1) {
            if (!SDL_AtomicGet(&keybuf_stop)) {
                fprintf(stderr, "keybuf: error reading from terminal\n");
            }
            break;
        }
        keybuf_put((char) ch);
    }
    return 0;
}

MmResult keybuf_init(void) {
    LOG_FN_ENTRY();

    rx_buf_init(&keybuf_buf, keybuf_data, sizeof(keybuf_data));

    keybuf_mutex = SDL_CreateMutex();
    if (!keybuf_mutex) {
        fprintf(stderr, "keybuf: failed to create mutex: %s\n", SDL_GetError());
        return kError;
    }

    SDL_AtomicSet(&keybuf_stop, false);
    keybuf_thread = SDL_CreateThread(keybuf_thread_fn, "keybuf", NULL);
    if (!keybuf_thread) {
        fprintf(stderr, "keybuf: failed to create thread: %s\n", SDL_GetError());
        SDL_DestroyMutex(keybuf_mutex);
        keybuf_mutex = NULL;
        return kError;
    }

    RETURN_RESULT(kOk);
}

void keybuf_term(void) {
    // Signal the thread to stop. We don't wait for it to exit - the OS will
    // clean up when the process exits. The thread may be blocked in
    // keybuf_read_char() but that's acceptable since we're exiting anyway.
    SDL_AtomicSet(&keybuf_stop, true);
}

void keybuf_clear(void) {
    SDL_LockMutex(keybuf_mutex);
    rx_buf_clear(&keybuf_buf);
    SDL_UnlockMutex(keybuf_mutex);
}

int keybuf_count(void) {
    SDL_LockMutex(keybuf_mutex);
    int count = rx_buf_size(&keybuf_buf);
    SDL_UnlockMutex(keybuf_mutex);
    return count;
}

void keybuf_unget(char ch) {
    SDL_LockMutex(keybuf_mutex);
    rx_buf_unget(&keybuf_buf, ch);
    SDL_UnlockMutex(keybuf_mutex);
}

int keybuf_match_chars(char *pattern) {
    if (*pattern == '\0') return 1;

    // if (rx_buf_size(&keybuf_buf) == 0) {
    //     perform_background_tasks(); // Which calls other background processing.
    // }

    SDL_LockMutex(keybuf_mutex);
    int ch = rx_buf_get(&keybuf_buf);
    SDL_UnlockMutex(keybuf_mutex);

    if (ch == -1) {
        return 0;
    } else if (ch == *pattern && keybuf_match_chars(++pattern)) {
        return 1;
    } else {
        keybuf_unget(ch);
        return 0;
    }
}

int keybuf_get(void) {
    // LOG_FN_ENTRY();

    const int ESCAPE_MAP_ENTRY_LEN = 8;

    static char ESCAPE_MAP[] = {
        'O', 'P', '\0', '\0', '\0', '\0', '\0', F1,
        'O', 'Q', '\0', '\0', '\0', '\0', '\0', F2,
        'O', 'R', '\0', '\0', '\0', '\0', '\0', F3,
        'O', 'S', '\0', '\0', '\0', '\0', '\0', F4,
        '[', '1', '5',  '~',  '\0', '\0', '\0', F5,
        '[', '1', '7',  '~',  '\0', '\0', '\0', F6,
        '[', '1', '8',  '~',  '\0', '\0', '\0', F7,
        '[', '1', '9',  '~',  '\0', '\0', '\0', F8,
        '[', '2', '0',  '~',  '\0', '\0', '\0', F9,
        '[', '2', '1',  '~',  '\0', '\0', '\0', F10,  // F10 - is captured by the Gnome WM
        '[', '2', '3',  '~',  '\0', '\0', '\0', F11,  // F11 - is captured by the Gnome WM
        '[', '2', '4',  '~',  '\0', '\0', '\0', F12,
        '[', '2', '~',  '\0', '\0', '\0', '\0', INSERT,
        '[', '3', '~',  '\0', '\0', '\0', '\0', DEL,
        '[', '5', '~',  '\0', '\0', '\0', '\0', PUP,
        '[', '6', '~',  '\0', '\0', '\0', '\0', PDOWN,
        '[', 'A', '\0', '\0', '\0', '\0', '\0', UP,
        '[', 'B', '\0', '\0', '\0', '\0', '\0', DOWN,
        '[', 'C', '\0', '\0', '\0', '\0', '\0', RIGHT,
        '[', 'D', '\0', '\0', '\0', '\0', '\0', LEFT,
        '[', 'F', '\0', '\0', '\0', '\0', '\0', END,
        '[', 'H', '\0', '\0', '\0', '\0', '\0', HOME,
        '[', '1', ';',  '2',  'P',  '\0', '\0', SHIFT_FN(F1),
        '[', '1', ';',  '2',  'Q',  '\0', '\0', SHIFT_FN(F2),
        '[', '1', ';',  '2',  'R',  '\0', '\0', SHIFT_FN(F3),
        '[', '1', ';',  '2',  'S',  '\0', '\0', SHIFT_FN(F4),
        '[', '1', '5',  ';',  '2',  '~',  '\0', SHIFT_FN(F5),
        '[', '1', '7',  ';',  '2',  '~',  '\0', SHIFT_FN(F6),
        '[', '1', '8',  ';',  '2',  '~',  '\0', SHIFT_FN(F7),
        '[', '1', '9',  ';',  '2',  '~',  '\0', SHIFT_FN(F8),
        '[', '2', '0',  ';',  '2',  '~',  '\0', SHIFT_FN(F9),
        '[', '2', '1',  ';',  '2',  '~',  '\0', SHIFT_FN(F10),
        '[', '2', '3',  ';',  '2',  '~',  '\0', SHIFT_FN(F11),
        '[', '2', '4',  ';',  '2',  '~',  '\0', SHIFT_FN(F12),
        0xFF};

    // perform_background_tasks(); // Which calls console_pump_input();

    SDL_LockMutex(keybuf_mutex);
    int ch = rx_buf_get(&keybuf_buf);
    SDL_UnlockMutex(keybuf_mutex);

    switch (ch) {
        // case 0x0A:
        //     ch = ENTER;
        //     break;

        case ESC: {
            // Wait briefly for the rest of the escape sequence to arrive
            // in case it is being delivered character by character.
            SDL_Delay(20);  // 20ms is enough for a local terminal sequence

            char *p = ESCAPE_MAP;
            while (*p != 0xFF) {
                if (keybuf_match_chars(p)) {
                    ch = *(p + ESCAPE_MAP_ENTRY_LEN - 1);
                    break;
                }
                p += ESCAPE_MAP_ENTRY_LEN;
            }
            break;
        }

        case DEL:
            // As the result of a historical quirk of terminals:
            //  - the [Backspace] key sends the ASCII code for "Delete" (0x7F)
            //  - the [Delete] keys sends the escape sequence \x1b[3~ which will
            //    be handled by the 'case ESC:' clause above.
            ch = BKSP;
            break;

        default:
            break;
    }

    RETURN_INT(ch);
}

void keybuf_put(char ch) {
    LOG_FN_ENTRY("ch='%c'", ch);

    // Support for ON KEY ascii_code%, handler_sub().
    // Note that 'ch' does not get added to the buffer.
    if (interrupt_check_key_press(ch)) RETURN_VOID();

    SDL_LockMutex(keybuf_mutex);

    if (ch == (char) SDL_AtomicGet(&mmb_options.break_key)) {
        // User wishes to stop the program.
        // Set the abort flag so the interpreter will halt and empty the keyboard buffer.
        SDL_AtomicSet(&MMAbort, true);
        rx_buf_clear(&keybuf_buf);
    } else {
        // If the buffer is full then this will throw away ch.
        rx_buf_put(&keybuf_buf, ch);
    }

    SDL_UnlockMutex(keybuf_mutex);

    RETURN_VOID();
}

void keybuf_key_to_string(int ch, char *buf) {
    static const int KEY_TO_STRING_MAP_ENTRY_LEN = 11;

    static char KEY_TO_STRING_MAP[] = {
        0x20,          'S', 'P',  'A',  'C',  'E', '\0', '\0', '\0', '\0', '\0',
        TAB,           'T', 'A',  'B', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        BKSP,          'B', 'K',  'S',  'P', '\0', '\0', '\0', '\0', '\0', '\0',
        ENTER,         'E', 'N',  'T',  'E',  'R', '\0', '\0', '\0', '\0', '\0',
        ESC,           'E', 'S',  'C', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F1,            'F', '1', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F2,            'F', '2', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F3,            'F', '3', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F4,            'F', '4', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F5,            'F', '5', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F6,            'F', '6', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F7,            'F', '7', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F8,            'F', '8', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F9,            'F', '9', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F10,           'F', '1',  '0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F11,           'F', '1',  '1', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        F12,           'F', '1',  '2', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        UP,            'U', 'P', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        DOWN,          'D', 'O',  'W',  'N', '\0', '\0', '\0', '\0', '\0', '\0',
        LEFT,          'L', 'E',  'F',  'T', '\0', '\0', '\0', '\0', '\0', '\0',
        RIGHT,         'R', 'I',  'G',  'H',  'T', '\0', '\0', '\0', '\0', '\0',
        INSERT,        'I', 'N',  'S',  'E',  'R',  'T', '\0', '\0', '\0', '\0',
        DEL,           'D', 'E',  'L', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        HOME,          'H', 'O',  'M',  'E', '\0', '\0', '\0', '\0', '\0', '\0',
        END,           'E', 'N',  'D', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        PUP,           'P', 'U',  'P', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        PDOWN,         'P', 'D',  'O',  'W',  'N', '\0', '\0', '\0', '\0', '\0',
        SLOCK,         'S', 'L',  'O',  'C',  'K', '\0', '\0', '\0', '\0', '\0',
        ALT,           'A', 'L',  'T', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
        SHIFT_FN(F1),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '1', '\0', '\0',
        SHIFT_FN(F2),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '2', '\0', '\0',
        SHIFT_FN(F3),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '3', '\0', '\0',
        SHIFT_FN(F4),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '4', '\0', '\0',
        SHIFT_FN(F5),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '5', '\0', '\0',
        SHIFT_FN(F6),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '6', '\0', '\0',
        SHIFT_FN(F7),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '7', '\0', '\0',
        SHIFT_FN(F8),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '8', '\0', '\0',
        SHIFT_FN(F9),  'S', 'H',  'I',  'F',  'T',  '+',  'F',  '9', '\0', '\0',
        SHIFT_FN(F10), 'S', 'H',  'I',  'F',  'T',  '+',  'F',  '1',  '0', '\0',
        SHIFT_FN(F11), 'S', 'H',  'I',  'F',  'T',  '+',  'F',  '1',  '1', '\0',
        SHIFT_FN(F12), 'S', 'H',  'I',  'F',  'T',  '+',  'F',  '1',  '2', '\0',
        0xFF
    };

    char *p = KEY_TO_STRING_MAP;
    while (*p != 0xFF) {
        if (*p == ch) {
            sprintf(buf, "[%s]", p + 1);
            return;
        }
        p += KEY_TO_STRING_MAP_ENTRY_LEN;
    }
    sprintf(buf, "'%c'", ch);
}
