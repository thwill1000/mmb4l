/*-*****************************************************************************

MMBasic for Linux (MMB4L)

prompt.c

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

#include "mmb4l.h"
#include "console.h"
#include "error.h"
#include "file.h"
#include "options.h"
#include "path.h"
#include "prompt.h"
#include "utility.h"

#define HISTORY_SIZE  4 * STRINGSIZE
#define ERROR_LINE_TOO_LONG_TO_EDIT  error_throw_ex(kStringTooLong, "Line is too long to edit")
#define ERROR_TAB_CHAR_IN_FN_DEF     error_throw_ex(kError, "Tab character in function key definition")

static char history[HISTORY_SIZE];
static const char NO_ITEM[] = "";

/** Displays the contents of the 'history' buffer. */
static void dump_history() {
    char s[STRINGSIZE];
    char *p = history;
    char *start = p;
    MMPrintString("[BEGIN]\r\n");
    for (; p < history + HISTORY_SIZE; ++p) {
        if (*p == '\0') {
            int len = p - start;
            if (len == 0) break;
            memset(s, 0, STRINGSIZE);
            memcpy(s, start, len);
            MMPrintString("~");
            MMPrintString(s);
            MMPrintString("~\r\n");
            start = p + 1;
        }
    }
    MMPrintString("[END]\r\n");
    MMPrintString("\r\n");
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

static void handle_backspace(PromptState *pstate) {
    if (pstate->char_index <= 0) return;

    int i = pstate->char_index - 1;
    for (char *p = inpbuf + i; *p; p++) {
        *p = *(p + 1);  // remove the char from inpbuf
    }
    while (pstate->char_index) {
        console_putc('\b');
        pstate->char_index--;
    }  // go to the beginning of the line
    MMPrintString(inpbuf);
    console_putc(' ');
    console_putc('\b');  // display the line and erase the last char
    for (pstate->char_index = strlen(inpbuf); pstate->char_index > i; pstate->char_index--) {
        console_putc('\b');  // return the cursor to the right position
    }
}

static void handle_delete(PromptState *pstate) {
    if (pstate->char_index >= strlen(inpbuf)) return;

    int i = pstate->char_index;
    for (char *p = inpbuf + i; *p; p++) {
        *p = *(p + 1);  // remove the char from inpbuf
    }
    while (pstate->char_index) {
        console_putc('\b');
        pstate->char_index--;
    }  // go to the beginning of the line
    MMPrintString(inpbuf);
    console_putc(' ');
    console_putc('\b');  // display the line and erase the last char
    for (pstate->char_index = strlen(inpbuf); pstate->char_index > i; pstate->char_index--) {
        console_putc('\b');  // return the cursor to the right position
    }
}

static void prompt_update_inpbuf(PromptState *pstate, char *new_inpbuf) {
    // Update characters in input buffer.
    strcpy(inpbuf, new_inpbuf);

    // Erase existing input from the console.
    for (int i = 0; i < pstate->char_index; ++i) console_putc('\b');
    for (int i = 0; i < pstate->char_index; ++i) console_putc(' ');
    for (int i = 0; i < pstate->char_index; ++i) console_putc('\b');

    // Display the new contents of the input buffer.
    MMPrintString(inpbuf);

    // Handle the new input buffer being too long.
    if (strlen(inpbuf) + pstate->start_line >= pstate->max_chars) {
        ERROR_LINE_TOO_LONG_TO_EDIT;
    }

    // Update 'char_index' to reflect new input buffer contents.
    pstate->char_index = strlen(inpbuf);
}

static void handle_down(PromptState *pstate) {
    assert(pstate->history_idx >= -1);
    if (pstate->history_idx > -1) {
        pstate->history_idx--;
        if (pstate->history_idx == -1) {
            prompt_update_inpbuf(pstate, pstate->backup);
        } else {
            prompt_update_inpbuf(pstate, get_history_item(pstate->history_idx));
        }
    }
}

static void handle_end(PromptState *pstate) {
    while (pstate->char_index < strlen(inpbuf)) {
        console_putc(inpbuf[pstate->char_index++]);
    }
}

static void handle_function_key(PromptState *pstate) {
    if (pstate->buf[0] >= F1 && pstate->buf[0] <= F12) {
        strcpy(pstate->buf + 1, mmb_options.fn_keys[pstate->buf[0] - F1]);

        // Currently allowing tab characters in function key definitions
        // would royally screw things up because it will interact with path
        // completion which also manipulates pstate->buf.
        // TODO: either support it or prevent it in OPTION F<NUM>.
        char *p = pstate->buf;
        while (*p) {
            if (*p == TAB) ERROR_TAB_CHAR_IN_FN_DEF;
            p++;
        }
    }
}

static void handle_home(PromptState *pstate) {
    if (pstate->char_index <= 0) return;

    if (pstate->char_index == strlen(inpbuf)) {
        pstate->insert = true;
    }

    while (pstate->char_index) {
        console_putc('\b');
        pstate->char_index--;
    }
}

static void handle_insert(PromptState *pstate) {
    pstate->insert = !pstate->insert;
}

static void handle_left(PromptState *pstate) {
    if (pstate->char_index <= 0) return;

    if (pstate->char_index == strlen(inpbuf)) {
        pstate->insert = true;
    }
    console_putc('\b');
    pstate->char_index--;
}

static void handle_newline(PromptState *pstate) {
    pstate->save_line = 1;
}

static void handle_other(PromptState *pstate) {
    if (pstate->buf[0] < ' ' || pstate->buf[0] >= 0x7f) return;

    int j = strlen(inpbuf);

    if (pstate->insert) {
        if (strlen(inpbuf) >= pstate->max_chars - 1) return;  // sorry, line full
        for (char *p = inpbuf + strlen(inpbuf); j >= pstate->char_index; p--, j--) {
            *(p + 1) = *p;
        }
        inpbuf[pstate->char_index] = pstate->buf[0];  // insert the char
        MMPrintString(&inpbuf[pstate->char_index]);   // display new part of
                                                      // the line
        pstate->char_index++;
        for (j = strlen(inpbuf); j > pstate->char_index; j--) {
            console_putc('\b');  // return the cursor to the right position
        }
    } else {
        inpbuf[strlen(inpbuf) + 1] = 0;  // incase we are adding to the end
                                         // of the string
        inpbuf[pstate->char_index++] = pstate->buf[0];  // overwrite the char
        console_putc(pstate->buf[0]);                      // display it
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
}

static void handle_right(PromptState *pstate) {
    if (pstate->char_index >= strlen(inpbuf)) return;

    console_putc(inpbuf[pstate->char_index]);
    pstate->char_index++;
}

void prompt_handle_tab(PromptState *pstate) {
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
    if (pstate->buf[1] == '\0') console_bell();
}

static void handle_up(PromptState *pstate) {
    assert(pstate->history_idx >= -1);
    if (pstate->history_idx + 1 < get_history_count()) {
        if (pstate->history_idx == -1) strcpy(pstate->backup, inpbuf);
        pstate->history_idx++;
        prompt_update_inpbuf(pstate, get_history_item(pstate->history_idx));
    }
}

void prompt_get_input(void) {
    int width, height;
    if (FAILED(console_get_size(&width, &height))) ERROR_INTERNAL_FAULT;

    PromptState state = { 0 };
    state.char_index = strlen(inpbuf); // get the current cursor position in the line
    state.start_line = MMCharPos - 1;  // save the current cursor position
    state.max_chars = width;
    state.history_idx = -1;

    MMPrintString(inpbuf);  // display the contents of the input buffer (if any)

    if (strlen(inpbuf) >= state.max_chars) {
        ERROR_LINE_TOO_LONG_TO_EDIT;
    }

    while (1) {
        state.buf[0] = MMgetchar();
        state.buf[1] = '\0';

        do {
            switch (state.buf[0]) {
                case '\r':
                case '\n':
                    handle_newline(&state);
                    break;

                case '\b':
                    handle_backspace(&state);
                    break;

                // case CTRLKEY('S'):
                case LEFT:
                    handle_left(&state);
                    break;

                // case CTRLKEY('D'):
                case RIGHT:
                    handle_right(&state);
                    break;

                // case CTRLKEY(']'):
                case DEL:
                    handle_delete(&state);
                    break;

                // case CTRLKEY('N'):
                case INSERT:
                    handle_insert(&state);
                    break;

                // case CTRLKEY('U'):
                case HOME:
                    handle_home(&state);
                    break;

                // case CTRLKEY('K'):
                case END:
                    handle_end(&state);
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
                    handle_function_key(&state);
                    break;

                // case CTRLKEY('E'):
                case UP:
                    handle_up(&state);
                    break;

                // case CTRLKEY('X'):
                case DOWN:
                    handle_down(&state);
                    break;

                case TAB:
                    prompt_handle_tab(&state);
                    break;

                default:
                    handle_other(&state);
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
    MMPrintString("\r\n");

    put_history_item(inpbuf);
}
