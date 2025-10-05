/*-*****************************************************************************

MMBasic for Linux (MMB4L)

prompt.c

Copyright 2021-2025 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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
#include <string.h>
#include <sys/types.h>

#include "display.h"
#include "keycodes.h"
#include "mmb4l.h"
#include "mmgetchar.h"
#include "path.h"
#include "prompt.h"
#include "utility.h"

#define HISTORY_SIZE  4 * STRINGSIZE
#define LINE_TOO_LONG_TO_EDIT  "Line is too long to edit"
#define TAB_CHAR_IN_FN_DEF     "Tab character in function key definition"

static char history[HISTORY_SIZE];
static const char NO_ITEM[] = "";

/** Displays the contents of the 'history' buffer. */
static void dump_history() {
    char s[STRINGSIZE];
    char *p = history;
    char *start = p;
    display_puts("[BEGIN]\r\n");
    for (; p < history + HISTORY_SIZE; ++p) {
        if (*p == '\0') {
            int len = p - start;
            if (len == 0) break;
            memset(s, 0, STRINGSIZE);
            memcpy(s, start, len);
            display_puts("~");
            display_puts(s);
            display_puts("~\r\n");
            start = p + 1;
        }
    }
    display_puts("[END]\r\n");
    display_puts("\r\n");
}

/** Gets an item from the 'history' buffer. */
static char *get_history_item(int idx) {
    if (idx < 0) {
        return (char *) NO_ITEM;
    }

    int current = 0;
    char *p = history;
    char *item;

    for (;;) {
        item = p;
        if (current == idx) break;
        p += strlen(p);
        p++;
        if (p >= history + HISTORY_SIZE || *p == '\0') break;
        current++;
    }

    return item;
}

/** Gets the number of items in the 'history' buffer. */
static int get_history_count() {
    int count = 0;
    char *p = history;

    for (;;) {
        if (p >= history + HISTORY_SIZE || *p == '\0') break;
        p += strlen(p);
        p++;
        count++;
    }

    return count;
}

/**
 * Inserts a string into the start of the 'history' buffer.
 * The buffer is a sequence of strings separated by a zero byte.
 * using the up arrow usere can call up the last few commands executed.
 */
void put_history_item(char *s) {
    if (strcmp(history, s) == 0) return;  // Don't store duplicates.
    int slen = strlen(s);
    if (slen < 1 || slen > HISTORY_SIZE - 1) return;
    slen++;

    // Shift the contents of the buffer to the right.
    for (int i = HISTORY_SIZE - 1; i >= slen; i--) {
        history[i] = history[i - slen];
    }

    // Insert new string at the beginning.
    strcpy(history, s);

    // Zero the end of the buffer.
    for (int i = HISTORY_SIZE - 1; history[i]; i--) {
        history[i] = '\0';
    }
}

static MmResult handle_backspace(PromptState *pstate) {
    if (pstate->char_index <= 0) return kOk;

    size_t i = pstate->char_index - 1;
    for (char *p = inpbuf + i; *p; p++) {
        *p = *(p + 1);  // remove the char from inpbuf
    }
    while (pstate->char_index) {
        display_putc('\b');
        pstate->char_index--;
    }  // go to the beginning of the line
    display_puts(inpbuf);
    display_putc(' ');
    display_putc('\b');  // display the line and erase the last char
    for (pstate->char_index = strlen(inpbuf); pstate->char_index > i; pstate->char_index--) {
        display_putc('\b');  // return the cursor to the right position
    }

    return kOk;
}

static MmResult handle_delete(PromptState *pstate) {
    if (pstate->char_index >= strlen(inpbuf)) return kOk;

    size_t i = pstate->char_index;
    for (char *p = inpbuf + i; *p; p++) {
        *p = *(p + 1);  // remove the char from inpbuf
    }
    while (pstate->char_index) {
        display_putc('\b');
        pstate->char_index--;
    }  // go to the beginning of the line
    display_puts(inpbuf);
    display_putc(' ');
    display_putc('\b');  // display the line and erase the last char
    for (pstate->char_index = strlen(inpbuf); pstate->char_index > i; pstate->char_index--) {
        display_putc('\b');  // return the cursor to the right position
    }

    return kOk;
}

static MmResult prompt_update_inpbuf(PromptState *pstate, char *new_inpbuf) {
    // Update characters in input buffer.
    strcpy(inpbuf, new_inpbuf);

    // Erase existing input from the console.
    for (size_t i = 0; i < pstate->char_index; ++i) display_putc('\b');
    for (size_t i = 0; i < pstate->char_index; ++i) display_putc(' ');
    for (size_t i = 0; i < pstate->char_index; ++i) display_putc('\b');

    // Display the new contents of the input buffer.
    display_puts(inpbuf);

    // Handle the new input buffer being too long.
    if (strlen(inpbuf) + pstate->start_line >= pstate->max_chars) {
        return mmresult_ex(kStringTooLong, LINE_TOO_LONG_TO_EDIT);
    }

    // Update 'char_index' to reflect new input buffer contents.
    pstate->char_index = strlen(inpbuf);

    return kOk;
}

static MmResult handle_down(PromptState *pstate) {
    assert(pstate->history_idx >= -1);
    if (pstate->history_idx > -1) {
        pstate->history_idx--;
        if (pstate->history_idx == -1) {
            ON_FAILURE_RETURN(prompt_update_inpbuf(pstate, pstate->backup));
        } else {
            ON_FAILURE_RETURN(prompt_update_inpbuf(pstate, get_history_item(pstate->history_idx)));
        }
    }

    return kOk;
}

static MmResult handle_end(PromptState *pstate) {
    while (pstate->char_index < strlen(inpbuf)) {
        display_putc(inpbuf[pstate->char_index++]);
    }

    return kOk;
}

static MmResult handle_function_key(PromptState *pstate) {
    if (pstate->buf[0] >= F1 && pstate->buf[0] <= F12) {
        strcpy(pstate->buf + 1, mmb_options.fn_keys[pstate->buf[0] - F1]);

        // Currently allowing tab characters in function key definitions
        // would royally screw things up because it will interact with path
        // completion which also manipulates pstate->buf.
        // TODO: either support it or prevent it in OPTION F<NUM>.
        char *p = pstate->buf;
        while (*p) {
            if (*p == TAB) return mmresult_ex(kError, TAB_CHAR_IN_FN_DEF);
            p++;
        }
    }

    return kOk;
}

static MmResult handle_home(PromptState *pstate) {
    if (pstate->char_index <= 0) return kOk;

    if (pstate->char_index == strlen(inpbuf)) {
        pstate->insert = true;
    }

    while (pstate->char_index) {
        display_putc('\b');
        pstate->char_index--;
    }

    return kOk;
}

static MmResult handle_insert(PromptState *pstate) {
    pstate->insert = !pstate->insert;
    return kOk;
}

static MmResult handle_left(PromptState *pstate) {
    if (pstate->char_index <= 0) return kOk;

    if (pstate->char_index == strlen(inpbuf)) {
        pstate->insert = true;
    }
    display_putc('\b');
    pstate->char_index--;

    return kOk;
}

static MmResult handle_newline(PromptState *pstate) {
    pstate->save_line = 1;
    return kOk;
}

static MmResult handle_other(PromptState *pstate) {
    if (pstate->buf[0] < ' ' || pstate->buf[0] >= 0x7f) return kOk;

    size_t j = strlen(inpbuf);

    if (pstate->insert) {
        if (strlen(inpbuf) >= pstate->max_chars - 1) return kOk;  // sorry, line full
        for (char *p = inpbuf + strlen(inpbuf); j >= pstate->char_index; p--, j--) {
            *(p + 1) = *p;
        }
        inpbuf[pstate->char_index] = pstate->buf[0];  // insert the char
        display_puts(&inpbuf[pstate->char_index]);   // display new part of
                                                      // the line
        pstate->char_index++;
        for (j = strlen(inpbuf); j > pstate->char_index; j--) {
            display_putc('\b');  // return the cursor to the right position
        }
    } else {
        inpbuf[strlen(inpbuf) + 1] = 0;  // incase we are adding to the end
                                         // of the string
        inpbuf[pstate->char_index++] = pstate->buf[0];  // overwrite the char
        display_putc(pstate->buf[0]);                      // display it
        if (pstate->char_index + pstate->start_line >=
            pstate->max_chars) {  // has the input gone beyond the
                                  // end of the line?
            MMgetline(0, inpbuf);  // use the old fashioned way
                                   // of getting the line
            // if(autoOn && atoi(inpbuf) > 0) autoNext =
            // atoi(inpbuf) + autoIncr;
            pstate->save_line = 1;
        }
    }

    return kOk;
}

static MmResult handle_right(PromptState *pstate) {
    if (pstate->char_index >= strlen(inpbuf)) return kOk;

    display_putc(inpbuf[pstate->char_index]);
    pstate->char_index++;

    return kOk;
}

MmResult prompt_handle_tab(PromptState *pstate) {
    char *pstart = inpbuf;
    char *p = inpbuf;
    bool in_quote = false;
    while (*p) {
        switch (*p) {
            case ' ':
                if (!in_quote) pstart = p + 1;
                break;
            case '"':
                in_quote = !in_quote;
                pstart = p + 1;
                break;
            default:
                break;
        }
        p++;
    }

    if (FAILED(path_complete(pstart, pstate->buf + 1, sizeof(pstate->buf) - 1)))
        pstate->buf[1] = '\0';
    if (pstate->buf[1] == '\0') display_bell();

    return kOk;
}

static MmResult handle_up(PromptState *pstate) {
    assert(pstate->history_idx >= -1);

    if (pstate->history_idx + 1 < get_history_count()) {
        if (pstate->history_idx == -1) strcpy(pstate->backup, inpbuf);
        pstate->history_idx++;
        ON_FAILURE_RETURN(prompt_update_inpbuf(pstate, get_history_item(pstate->history_idx)));
    }

    return kOk;
}

MmResult prompt_get_input(void) {
    int width = -1, height = -1;
    ON_FAILURE_RETURN(display_get_size(false, &width, &height));
    int x = -1, y = -1;
    ON_FAILURE_RETURN(display_get_cursor_pos(false, &x, &y));

    PromptState state = { 0 };
    state.char_index = strlen(inpbuf); // get the current cursor position in the line
    state.start_line = x;              // save the current cursor position
    state.max_chars = width;
    state.history_idx = -1;

    display_puts(inpbuf);  // display the contents of the input buffer (if any)

    if (strlen(inpbuf) >= state.max_chars) {
        return mmresult_ex(kStringTooLong, LINE_TOO_LONG_TO_EDIT);
    }

    while (1) {
        state.buf[0] = MMgetchar();
        state.buf[1] = '\0';

        do {
            switch (state.buf[0]) {
                case '\r':
                case '\n':
                    ON_FAILURE_RETURN(handle_newline(&state));
                    break;

                case '\b':
                    ON_FAILURE_RETURN(handle_backspace(&state));
                    break;

                // case CTRLKEY('S'):
                case LEFT:
                    ON_FAILURE_RETURN(handle_left(&state));
                    break;

                // case CTRLKEY('D'):
                case RIGHT:
                    ON_FAILURE_RETURN(handle_right(&state));
                    break;

                // case CTRLKEY(']'):
                case DEL:
                    ON_FAILURE_RETURN(handle_delete(&state));
                    break;

                // case CTRLKEY('N'):
                case INSERT:
                    ON_FAILURE_RETURN(handle_insert(&state));
                    break;

                // case CTRLKEY('U'):
                case HOME:
                    ON_FAILURE_RETURN(handle_home(&state));
                    break;

                // case CTRLKEY('K'):
                case END:
                    ON_FAILURE_RETURN(handle_end(&state));
                    break;

                case F1:
                case F2:
                case F3:
                case F4:
                case F5:
                case F6:
                case F7:
                case F8:
                case F9:
                case F10:
                case F11:
                case F12:
                    ON_FAILURE_RETURN(handle_function_key(&state));
                    break;

                // case CTRLKEY('E'):
                case UP:
                    ON_FAILURE_RETURN(handle_up(&state));
                    break;

                // case CTRLKEY('X'):
                case DOWN:
                    ON_FAILURE_RETURN(handle_down(&state));
                    break;

                case TAB:
                    ON_FAILURE_RETURN(prompt_handle_tab(&state));
                    break;

                default:
                    ON_FAILURE_RETURN(handle_other(&state));
                    break;
            }

            if (state.save_line) goto saveline;

            // Shuffle down the buffer to get the next character.
            memmove(state.buf, state.buf + 1, sizeof(state.buf) - 1);
        } while (*state.buf);

        if (state.char_index == strlen(inpbuf)) {
            state.insert = false;
        }
    }

saveline:
    display_puts("\r\n");

    put_history_item(inpbuf);

    return kOk;
}
