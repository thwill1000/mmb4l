/*-*****************************************************************************

MMBasic for Linux (MMB4L)

prompt.c

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
#include <stdio.h>
#include <string.h>

#include "display.h"
#include "file.h"
#include "keybuf.h"
#include "keycodes.h"
#include "logger.h"
#include "mmb4l.h"
#include "mmtime.h"
#include "path.h"
#include "prompt.h"
#include "prompt_private.h"
#include "streamio.h"
#include "utility.h"

#define LINE_TOO_LONG_TO_EDIT  "Line is too long to edit"
#define TAB_CHAR_IN_FN_DEF     "Tab character in function key definition"
#define PROMPT_MAX_LEN  MAXSTRLEN

char prompt_history[sizeof(prompt_history)];
static const char NO_ITEM[] = "";

MmResult prompt_getc(int *ch) {
    // LOG_FN_ENTRY("ch=%p", ch);

    static char prevchar = 0;
    ON_FAILURE_RETURN(display_show_cursor(true));

    MmResult result = kOk;
    for (;;) {
        perform_background_tasks();
        result = display_update_cursor();
        ON_FAILURE_GOTO(result, cleanup);
        *ch = keybuf_get();
        if (*ch == -1) {
            if (!keybuf_isatty()) {
                // For non-TTY input (pipes, files), check if it's actually EOF
                if (feof(stdin)) {
                    result = kStdinExhausted;
                    goto cleanup;
                }
                // If not EOF, it might just be a blocking read that returned -1
                // Check errno to see if it's a real error
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    mmtime_sleep_ns(MICROSECONDS_TO_NANOSECONDS(1));
                    continue;
                }
                // Some other error occurred
                result = kStdinExhausted;
                goto cleanup;
            }
            mmtime_sleep_ns(MICROSECONDS_TO_NANOSECONDS(1));
        } else if (*ch == '\n' && prevchar == '\r') {
            prevchar = 0;
        } else {
            break;
        }
    }
    prevchar = *ch;
    if (*ch == '\n') *ch = '\r';

cleanup:

    ON_FAILURE_LOG(display_show_cursor(false));
    RETURN_RESULT(result);
}

/** Displays the contents of the 'prompt_history' buffer. */
static MmResult dump_history() {
    char s[STRINGSIZE];
    char *p = prompt_history;
    char *start = p;
    ON_FAILURE_RETURN(display_puts("[BEGIN]\r\n"));
    for (; p < prompt_history + sizeof(prompt_history); ++p) {
        if (*p == '\0') {
            int len = p - start;
            if (len == 0) break;
            memset(s, 0, STRINGSIZE);
            memcpy(s, start, len);
            ON_FAILURE_RETURN(display_puts("~"));
            ON_FAILURE_RETURN(display_puts(s));
            ON_FAILURE_RETURN(display_puts("~\r\n"));
            start = p + 1;
        }
    }
    ON_FAILURE_RETURN(display_puts("[END]\r\n"));
    ON_FAILURE_RETURN(display_puts("\r\n"));

    return kOk;
}

/** Gets an item from the 'history' buffer. */
char *prompt_get_history_item(int idx) {
    if (idx < 0) {
        return (char *) NO_ITEM;
    }

    int current = 0;
    char *p = prompt_history;
    char *item;

    for (;;) {
        item = p;
        if (current == idx) break;
        p += strlen(p);
        p++;
        if (p >= prompt_history + sizeof(prompt_history) || *p == '\0') break;
        current++;
    }

    return item;
}

/** Gets the number of items in the 'history' buffer. */
int prompt_get_history_count(void) {
    int count = 0;
    char *p = prompt_history;

    for (;;) {
        if (p >= prompt_history + sizeof(prompt_history) || *p == '\0') break;
        p += strlen(p);
        p++;
        count++;
    }

    return count;
}

/**
 * Inserts a string into the start of the 'history' buffer.
 * The buffer is a sequence of strings separated by a zero byte.
 * using the up arrow users can call up the last few commands executed.
 */
void prompt_put_history_item(const char *item) {
    if (strcmp(prompt_history, item) == 0) return;  // Don't store duplicates.
    size_t slen = strlen(item);
    if (slen < 1 || slen > sizeof(prompt_history) - 1) return;
    slen++;

    // Shift the contents of the buffer to the right.
    for (size_t i = sizeof(prompt_history) - 1; i >= slen; i--) {
        prompt_history[i] = prompt_history[i - slen];
    }

    // Insert new string at the beginning.
    strcpy(prompt_history, item);

    // Zero the end of the buffer.
    for (size_t i = sizeof(prompt_history) - 1; prompt_history[i]; i--) {
        prompt_history[i] = '\0';
    }
}

/**
 * Resolves and canonicalizes the history file path.
 *
 * If no filepath is provided (NULL or empty string), uses the default history
 * file location: ~/.mmbasic/mmbasic.history. Otherwise, canonicalizes the
 * provided path by resolving relative paths, symlinks, and removing redundant
 * separators.
 *
 * @param filepath  Path to history file, or NULL/empty for default location.
 * @param buf       Buffer to store the canonical path.
 * @param sz        Size of buffer in bytes.
 * @return          kOk on success, error code on failure.
 *
 * @note The resulting path in buf is always an absolute, canonical path.
 */
static MmResult prompt_normalize_history_file_path(const char *filepath, char *buf, size_t sz) {
    if (!filepath || filepath[0] == '\0') {
        char tmp[PATH_MAX] = { '\0' };
        ON_FAILURE_RETURN(file_get_config_dir(tmp, sizeof(tmp)));
        ON_FAILURE_RETURN(file_append_path(tmp, "mmbasic.history", sizeof(tmp)));
        ON_FAILURE_RETURN(path_get_canonical(tmp, buf, sz));
    } else {
        ON_FAILURE_RETURN(path_get_canonical(filepath, buf, sz));
    }
    return kOk;
}

MmResult prompt_restore_history(const char *filepath) {
    char canonical_path[PATH_MAX];
    ON_FAILURE_RETURN(
        prompt_normalize_history_file_path(filepath, canonical_path, sizeof(canonical_path)));
    LOG_INFO("restoring history from %s", canonical_path);

    int fnbr = streamio_find_free();
    ON_FAILURE_RETURN(streamio_open(canonical_path, "r", fnbr));

    // Read items, one per line
    int count = 0;
    char item[STRINGSIZE];
    while (!streamio_eof(fnbr)) {
        MmResult result = streamio_readln(fnbr, item, sizeof(item));
        if (FAILED(result)) {
            ON_FAILURE_LOG(streamio_close(fnbr));
            return result;
        }
        prompt_put_history_item(item);
        count++;
    }

    ON_FAILURE_LOG(streamio_close(fnbr));

#if defined(NDEBUG)
    (void) count;
#endif

    LOG_INFO("restored %d history items", count);
    RETURN_RESULT(kOk);
}

MmResult prompt_save_history(const char *filepath) {
    char canonical_path[PATH_MAX];
    ON_FAILURE_RETURN(
        prompt_normalize_history_file_path(filepath, canonical_path, sizeof(canonical_path)));
    LOG_INFO("saving history to %s", canonical_path);

    int fnbr = streamio_find_free();
    ON_FAILURE_RETURN(streamio_open(canonical_path, "w", fnbr));

    // Write each item on its own line, most recent item last
    int count = prompt_get_history_count();
    for (int i = count - 1; i >= 0; --i) {
        const char *item = prompt_get_history_item(i);
        size_t len = strlen(item);
        // Write the item with a newline
        if (streamio_write(fnbr, item, len) < len || streamio_write(fnbr, "\n", 1) < 1) {
            ON_FAILURE_LOG(streamio_close(fnbr));
            return INTERNAL_FAULT_EX("streamio_write() failed");
        }
    }

    ON_FAILURE_LOG(streamio_close(fnbr));

    LOG_INFO("saved %d history items", count);
    RETURN_RESULT(kOk);
}

static MmResult handle_backspace(PromptState *pstate) {
    if (pstate->char_index <= 0) {
        return display_bell();
    }

    // Remove the character from inpbuf
    pstate->char_index--;
    for (char *p = inpbuf + pstate->char_index; *p; p++) {
        *p = *(p + 1);
    }

    // Redraw inpbuf from the removal point
    ON_FAILURE_RETURN(display_cursor_left(1, true));
    ON_FAILURE_RETURN(display_puts(inpbuf + pstate->char_index));
    ON_FAILURE_RETURN(display_putc(' '));
    ON_FAILURE_RETURN(display_cursor_left(strlen(inpbuf) - pstate->char_index + 1, true));

    return kOk;
}

static MmResult handle_delete(PromptState *pstate) {
    if (pstate->char_index >= strlen(inpbuf)) {
        return display_bell();
    }

    // Remove the character from inpbuf
    for (char *p = inpbuf + pstate->char_index; *p; p++) {
        *p = *(p + 1);
    }

    // Redraw inpbuf from the removal point
    ON_FAILURE_RETURN(display_puts(inpbuf + pstate->char_index));
    ON_FAILURE_RETURN(display_putc(' '));
    ON_FAILURE_RETURN(display_cursor_left(strlen(inpbuf) - pstate->char_index + 1, true));

    return kOk;
}

static MmResult prompt_update_inpbuf(PromptState *pstate, char *new_inpbuf) {
    const size_t len = strlen(inpbuf);

    // Update characters in input buffer.
    strcpy(inpbuf, new_inpbuf);

    // Erase existing input from the display.
    if (pstate->char_index != 0) {
        ON_FAILURE_RETURN(display_cursor_left(pstate->char_index, true));
        for (size_t i = 0; i < len; ++i) ON_FAILURE_RETURN(display_putc(' '));
        ON_FAILURE_RETURN(display_cursor_left(pstate->char_index, true));
    }

    // Display the new contents of the input buffer.
    ON_FAILURE_RETURN(display_puts(inpbuf));
    ON_FAILURE_RETURN(display_flush());

    // Handle the new input buffer being too long.
    if (strlen(inpbuf) > PROMPT_MAX_LEN) {
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
            ON_FAILURE_RETURN(prompt_update_inpbuf(pstate, prompt_get_history_item(pstate->history_idx)));
        }
    }

    return kOk;
}

static MmResult handle_end(PromptState *pstate) {
    while (pstate->char_index < strlen(inpbuf)) {
        ON_FAILURE_RETURN(display_putc(inpbuf[pstate->char_index++]));
    }
    RETURN_RESULT(display_flush());
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

    ON_FAILURE_RETURN(display_cursor_left(pstate->char_index, true));
    pstate->char_index = 0;

    return kOk;
}

static MmResult handle_insert(PromptState *pstate) {
    pstate->insert = !pstate->insert;
    return kOk;
}

static MmResult handle_left(PromptState *pstate) {
    if (pstate->char_index <= 0) {
        return display_bell();
    }

    if (pstate->char_index != strlen(inpbuf)) {
        pstate->insert = true;
    }

    ON_FAILURE_RETURN(display_cursor_left(1, true));
    pstate->char_index--;

    return kOk;
}

static MmResult handle_newline(PromptState *pstate) {
    pstate->finished = true;
    pstate->buf[0] = '\0';
    return kOk;
}

static MmResult handle_other(PromptState *pstate) {
    // LOG_FN_ENTRY("char='%c'", pstate->buf[0]);

    if (pstate->buf[0] < ' ' || pstate->buf[0] >= 0x7f) RETURN_RESULT(kOk);

    if (pstate->insert) {
        if (strlen(inpbuf) >= PROMPT_MAX_LEN) {
            RETURN_RESULT(display_bell());
        }

        // Shuffle all characters past the insertion point in the inpbuf up one
        char *pinsert = inpbuf + pstate->char_index;
        for (char *p = inpbuf + strlen(inpbuf); p >= pinsert; p--) {
            *(p + 1) = *p;
        }

        // Insert the new character
        *pinsert = pstate->buf[0];

        // Redraw the input buffer from the insertion point
        ON_FAILURE_RETURN(display_puts(inpbuf + pstate->char_index));
        pstate->char_index++;

        // Return the cursor to the correct position
        ON_FAILURE_RETURN(display_cursor_left(strlen(inpbuf) - pstate->char_index, true));
    } else {
        if (pstate->char_index == PROMPT_MAX_LEN) {
            RETURN_RESULT(display_bell());
        }

        inpbuf[strlen(inpbuf) + 1] = '\0';  // incase we are adding to the end
                                            // of the string
        inpbuf[pstate->char_index++] = pstate->buf[0];    // overwrite the char
        ON_FAILURE_RETURN(display_putc(pstate->buf[0]));  // display it
        ON_FAILURE_RETURN(display_flush());
    }

    RETURN_RESULT(kOk);
}

static MmResult handle_right(PromptState *pstate) {
    if (pstate->char_index >= strlen(inpbuf)) RETURN_RESULT(kOk);

    ON_FAILURE_RETURN(display_putc(inpbuf[pstate->char_index]));
    ON_FAILURE_RETURN(display_flush());
    pstate->char_index++;

    RETURN_RESULT(kOk);
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

    if (FAILED(path_complete(pstart, pstate->buf + 1, sizeof(pstate->buf) - 1))) {
        pstate->buf[1] = '\0';
    }

    if (pstate->buf[1] == '\0') return display_bell();

    return kOk;
}

static MmResult handle_up(PromptState *pstate) {
    assert(pstate->history_idx >= -1);

    if (pstate->history_idx + 1 < prompt_get_history_count()) {
        if (pstate->history_idx == -1) strcpy(pstate->backup, inpbuf);
        pstate->history_idx++;
        ON_FAILURE_RETURN(prompt_update_inpbuf(pstate, prompt_get_history_item(pstate->history_idx)));
    }

    return kOk;
}

MmResult prompt_get_input(void) {
    // LOG_FN_ENTRY();

    PromptState state = { 0 };
    state.char_index = strlen(inpbuf);
    state.history_idx = -1;

    // Display the contents of the input buffer (if any)
    ON_FAILURE_RETURN(display_puts(inpbuf));
    ON_FAILURE_RETURN(display_flush());
    // LOG_DEBUG("[%s]", inpbuf);
    // LOG_DEBUG("max chars = %d", state.max_chars);

    if (strlen(inpbuf) > PROMPT_MAX_LEN) {
        RETURN_RESULT(mmresult_ex(kStringTooLong, LINE_TOO_LONG_TO_EDIT));
    }

    while (1) {
        int ch = -1;
        ON_FAILURE_RETURN(prompt_getc(&ch));
        assert(ch != -1);
        // LOG_DEBUG("ch='%c'", ch);
        state.buf[0] = (char) ch;
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

            // Shuffle down the buffer to get the next character.
            memmove(state.buf, state.buf + 1, sizeof(state.buf) - 1);
        } while (*state.buf);

        if (state.finished) break;

        if (state.char_index == strlen(inpbuf)) {
            state.insert = false;
        }

        ON_FAILURE_RETURN(display_wrapline());
    }

    ON_FAILURE_RETURN(display_puts("\r\n"));

    prompt_put_history_item(inpbuf);

    RETURN_RESULT(kOk);
}
