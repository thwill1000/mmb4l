/*-*****************************************************************************

MMBasic for Linux (MMB4L)

editor.c

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

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "cstring.h"
#include "display.h"
#include "editor.h"
#include "editor_private.h"
#include "error.h"
#include "file.h"
#include "fonttbl.h"
#include "keybuf.h"
#include "keycodes.h"
#include "memory.h"
#include "mmb4l.h"
#include "mmtime.h"
#include "program.h"
#include "prompt.h"
#include "streamio.h"
#include "../core/commandtbl.h"
#include "../core/MMBasic.h"
#include "../core/tokentbl.h"

#define ON_INVALID_CURSOR_RETURN()                                                          \
    do {                                                                                    \
        if (self->cx < 0 || self->cy < 0) {                                                 \
            return INTERNAL_FAULT_EX("invalid cursor position: cx=%d, cy=%d", self->cx,     \
                                     self->cy);                                             \
        }                                                                                   \
    } while (0)

#define ON_INVALID_TXTP_RETURN()                                                                \
    do {                                                                                        \
        if (self->txtp < self->buf || self->txtp >= self->buf + self->buf_sz) {                 \
            return INTERNAL_FAULT_EX("invalid txtp outside buffer: self->txtp=%p", self->txtp); \
        }                                                                                       \
    } while (0)

// Forward declaration of real function implementations
MmResult editor_highlight_impl(Editor *, HighlightType);
MmResult editor_print_func_keys_impl(Editor *);
MmResult editor_print_line_fast_impl(Editor *, char *p);
MmResult editor_print_lines_impl(Editor *, int, int);
MmResult editor_print_msg_impl(Editor *, const char *);
MmResult editor_print_status_impl(Editor *);

// Pointers to functions we want to override in unit-tests
MmResult (*editor_highlight)(Editor *, HighlightType) = editor_highlight_impl;
MmResult (*editor_print_func_keys)(Editor *) = editor_print_func_keys_impl;
MmResult (*editor_print_line_fast)(Editor *, char *) = editor_print_line_fast_impl;
MmResult (*editor_print_lines)(Editor *, int, int) = editor_print_lines_impl;
MmResult (*editor_print_msg)(Editor *, const char *) = editor_print_msg_impl;
MmResult (*editor_print_status)(Editor *) = editor_print_status_impl;

/**
 * Restores all overridable functions to their real implementations.
 */
void editor_restore_fn_pointers() {
    editor_highlight = editor_highlight_impl;
    editor_print_func_keys = editor_print_func_keys_impl;
    editor_print_line_fast = editor_print_line_fast_impl;
    editor_print_lines = editor_print_lines_impl;
    editor_print_msg = editor_print_msg_impl;
    editor_print_status = editor_print_status_impl;
}

/**
 * Initializes a Editor instance with the specified parameters.
 *
 * Zeros out the editor structure, sets initial values, and allocates a single
 * contiguous memory block for the edit buffer, clipboard, and key buffer.
 * The editor height is reduced by 2 rows to accommodate the status line
 * and function key display area.
 *
 * @param  self      Pointer to the Editor structure to initialize.
 * @param  filename  Path to the file being edited (not copied, pointer stored).
 * @param  width     Width of the editor display area in characters.
 * @param  height    Total height available in characters (status area included).
 * @return           kOk on success, kOutOfMemory if allocation fails.
 *
 * @note Use editor_destruct() to free allocated memory.
 * @note Sets insert mode as default and saves the current break key setting.
 */
MmResult editor_construct(Editor *self, const char *filename, int width, int height) {
    CHECK_PARAM(width >= 2 * SOFT_MARGIN);

    memset(self, 0, sizeof(Editor));
    self->buf_sz = EDIT_BUFFER_SIZE;
    self->height = height - 2; // 2 rows for the status line
    self->width = width;
    self->fname = filename;
    self->insert = true;
    self->mode = kEditMode;
    self->text_changed = false;
    self->saved_break_key = (char) SDL_AtomicGet(&mmb_options.break_key);
    self->highlight = kHighlightNormal;
    self->message[0] = '\0';
    self->change_start = NO_CHANGE;
    self->change_end = NO_CHANGE;

    // Allocate dynamic memory, including space for clipboard and key buffers.
    // We use a single allocation to reduce fragmentation.
    self->buf = GetTempMemory(self->buf_sz + 2 * MAXCLIP + 4);
    if (!self->buf) return kOutOfMemory;
    self->clipboard_buf = self->buf + self->buf_sz;
    self->key_buf = self->clipboard_buf + MAXCLIP + 2;

    return kOk;
}

/**
 * Frees memory allocated by editor_construct().
 *
 * Releases the contiguous memory block containing the edit buffer,
 * clipboard, and key buffer.
 *
 * @param  self  Pointer to the Editor instance to clean up.
 * @return       kOk on success.
 */
MmResult editor_destruct(Editor *self) {
    ClearSpecificTempMemory(self->buf);
    return kOk;
}

/**
 * Marks a range of lines as needing redraw.
 *
 * Extends the current change range to include the specified lines.
 * Multiple calls accumulate into a single range for efficient redrawing.
 *
 * @param  self   Pointer to the Editor instance.
 * @param  start  First line to mark as changed (0-based, absolute).
 * @param  end    Last line to mark as changed (0-based, absolute).
 *                Use LAST_LINE to mark to end of document.
 * @return        kOk on success, or an error code on failure.
 *
 * @note end >= num_lines is valid (for marking to the end of the document).
 * @note Change tracking is reset by editor_update_display().
 */
MmResult editor_set_changed_lines(Editor *self, int start, int end) {
    if (end < start) SWAP(int, end, start);
    CHECK_PARAM(self);
    CHECK_PARAM(start >= 0);

    // Initialize or extend the change range
    if (self->change_start == NO_CHANGE) {
        self->change_start = start;
        self->change_end = end;
    } else {
        self->change_start = min(self->change_start, start);
        self->change_end = max(self->change_end, end);
    }
    return kOk;
}

/**
 * Sets the display cursor position.
 *
 * @param  self  Pointer to the Editor instance.
 * @param  x     X-coordinate in characters, starting at 0 (left edge).
 * @param  y     Y-coordinate in characters, starting at 0 (top edge).
 * @return       kOk on success, or an error code on failure.
 *
 * @note Does not change self->cx or self->cy.
 */
static MmResult editor_set_cursor_pos(Editor *self, int x, int y) {
    if (x < 0 || x >= self->width || y < 0 || y >= self->height + 2) {
        LOG_WARN("cursor out of bounds: x=%d, y=%d, width=%d, height=%d", x, y, self->width,
                 self->height);
        x = min(max(0, x), self->width - 1);
        y = min(max(0, y), self->height + 1);
    }
    return display_set_cursor_pos(false, x, y);
}

/**
 * Sets the syntax highlighting color for the current character.
 *
 * This function manages different highlight states (normal text, keywords,
 * comments, strings, numbers) based on the character being displayed.
 * Must be called sequentially from the start of each line to maintain
 * correct state tracking.
 *
 * IMPORTANT: Only call this function via the editor_highlight() wrapper
 *            so that unit-tests can override it.
 *
 * @param  self       Pointer to the Editor instance.
 * @param  highlight  The type of highlighting to apply.
 * @return            kOk on success, or an error code on failure.
 */
MmResult editor_highlight_impl(Editor *self, HighlightType highlight) {
    if (highlight == self->highlight) return kOk;

    MmGraphicsColour fg = RGB_ANSI_DEFAULT;
    MmGraphicsColour bg = RGB_ANSI_DEFAULT;
    bool inverse = false;
    const bool old_inverse = self->highlight & 0x80;

    switch (highlight) {
        case kHighlightNormal:
        case kHighlightStatus:
        case kHighlightError:
            fg = RGB_ANSI_DEFAULT;
            break;
        case kHighlightComment:
            fg = RGB_ANSI_YELLOW;
            break;
        case kHighlightKeyword:
            fg = RGB_ANSI_CYAN;
            break;
        case kHighlightQuote:
            fg = RGB_ANSI_MAGENTA;
            break;
        case kHighlightNumber:
            fg = RGB_ANSI_GREEN;
            break;
        case kHighlightLine:
            fg = RGB_ANSI_MAGENTA;
            break;
        case kHighlightTrailingWhitespace:
            bg = RGB_ANSI_RED;
            break;
        case kHighlightRight:
        case kHighlightMark:
            inverse = true;
            break;
        default:
            return INTERNAL_FAULT_EX("invalid HighlightType: %d", highlight);
    }

    MmResult result = display_colour(fg, bg);
    if (SUCCEEDED(result) && inverse != old_inverse) {
        result = display_inverse(inverse);
    }
    if (SUCCEEDED(result)) self->highlight = highlight;
    return result;
}

/**
 * Finds the longest line in the text buffer.
 *
 * Scans through the entire text buffer to determine which line has the
 * most characters.
 *
 * @param       self    Pointer to the Editor instance.
 * @param[out]  line    Pointer to store the resulting line number (0-based).
 * @param[out]  length  Pointer to store the resulting line length.
 * @return              kOk on success, or an error code on failure.
 */
MmResult editor_find_longest_line(Editor *self, int *line, int *length) {
    int current_length = 0;
    int current_line = 0;
    *line = 0;
    *length = 0;
    const char *p = self->buf;
    while (*p) {
        if (*p == '\n') {
            // If this line exceeds the max, update
            if (current_length > *length) {
                *length = current_length;
                *line = current_line;
            }
            current_length = 0;  // Reset for a new line
            current_line++;
        } else {
            // Increase length for this segment of the line
            current_length++;
        }

        p++;
    }

    // Final check in case the last line was the longest
    if (current_length > *length) {
        *length = current_length;
        *line = current_line;
    }

    return kOk;
}

/**
 * Saves the editor buffer contents to a file.
 *
 * Creates a .bak backup if the file already exists.
 * Converts LF line endings to CRLF during the save process.
 *
 * @param  self      Pointer to the Editor instance.
 * @param  filename  Path to the file to save.
 * @return           kOk on success, kFilenameTooLong if backup name too long,
 *                   or other error codes from file operations.
 */
static MmResult editor_save_file(Editor *self, const char *filename) {
    // If the file already exists then make a backup.
    if (file_exists_regular(filename)) {
        char backup[PATH_MAX];
        if (FAILED(cstring_cpy(backup, filename, PATH_MAX))
                || FAILED(cstring_cat(backup, ".bak", PATH_MAX))) {
            return kFilenameTooLong;
        }
        const int fnbr = streamio_find_free();
        ON_FAILURE_RETURN(streamio_open(filename, "rb", fnbr));
        const int fnbr_bak = streamio_find_free();
        ON_FAILURE_RETURN(streamio_open(backup, "wb", fnbr_bak));
        while (!streamio_eof(fnbr)) {
            (void) streamio_putc(fnbr_bak, streamio_getc(fnbr));
        }
        ON_FAILURE_LOG(streamio_close(fnbr));
        ON_FAILURE_LOG(streamio_close(fnbr_bak));
    }

    const int fnbr = streamio_find_free();
    ON_FAILURE_RETURN(streamio_open(filename, "wb", fnbr));

    // Copy contents of edit buffer to file
    // changing the LF line-endings to CRLF.
    for (const char *p = self->buf; *p; ++p) {
        if (*p == '\n') (void) streamio_putc(fnbr, '\r');
        (void) streamio_putc(fnbr, *p);
    }

    return streamio_close(fnbr);
}

/**
 * Adjusts the horizontal viewport to keep the cursor visible.
 *
 * Implements horizontal scrolling with a soft margin of SOFT_MARGIN characters.
 * When the cursor approaches the left or right edge of the screen, shifts
 * columns between cx (cursor column) and px (viewport offset) to maintain
 * comfortable viewing distance from screen edges.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 *
 * @note Does not trigger redraws - caller must mark lines as changed.
 * @note Scroll indicators ('<' and '>') are rendered by editor_print_line_p().
 */
MmResult editor_adjust_viewport(Editor *self) {
    // LOG_DEBUG("self->cx=%d, self->width=%d", self->cx, self->width);
    ON_INVALID_CURSOR_RETURN();
    if (self->width < 2 * SOFT_MARGIN) {
        LOG_ERROR("viewport too narrow: self->width=%d", self->width);
        return kOk;
    }
    while (self->cx >= self->width - SOFT_MARGIN) {
        self->cx--;
        self->px++;
    }
    while (self->cx <= SOFT_MARGIN && self->px != 0) {
        self->cx++;
        self->px--;
    }
    while (self->cy >= self->height - 1) {
        self->py++;
        self->cy--;
    }

    return kOk;
}

/**
 * Synchronizes the position of the display cursor with a position in the
 * text buffer.
 *
 * Calculates the line and column for the given text pointer and moves
 * the cursor accordingly. Does nothing if the line is not currently visible
 * on screen.
 *
 * @param  self  Pointer to the Editor instance.
 * @param  pbuf  Pointer to a position in the text buffer.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_sync_cursor_to_buffer(Editor *self, char *pbuf) {
    int line = 0;
    int column = 0;
    ON_FAILURE_RETURN(editor_get_line_and_column(self, pbuf, &line, &column));

    // If the line is not currently visible in the viewport then do nothing.
    // if (line < self->py || line >= self->py + self->height) return kOk;

    self->cx = column;
    self->cy = line - self->py;
    self->px = 0;
    ON_FAILURE_RETURN(editor_adjust_viewport(self));

    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Draws a horizontal line across the full editor width.
 *
 * Uses Unicode non-breaking spaces with underline formatting to create
 * a visual separator line. Uses non-breaking spaces for better terminal
 * compatibility (e.g., Alacritty).
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_draw_line(Editor *self) {
    ON_FAILURE_RETURN(editor_highlight(self, kHighlightLine));
    ON_FAILURE_RETURN(display_underline(true));

    char buf[STRINGSIZE];

#if defined(_WIN32)
    const char *space = " ";
    const int space_len = 1;
#else
    // On Linux use Unicode non-breaking spaces (U+00A0) for better terminal
    // compatibility with underline rendering, e.g. Alacritty does not render
    // underlines for normal spaces.
    const char *space = "\u00A0";  // Non-breaking space in UTF-8
    const int space_len = 2;       // UTF-8 encoding of U+00A0 is 2 bytes
#endif

    // Fill buffer with non-breaking spaces
    int pos = 0;
    for (int i = 0; i < self->width && pos < STRINGSIZE - space_len; i++) {
        memcpy(buf + pos, space, space_len);
        pos += space_len;
    }
    buf[pos] = '\0';

    ON_FAILURE_RETURN(display_puts(buf));
    ON_FAILURE_RETURN(display_reset());
    ON_FAILURE_RETURN(display_puts("\r\n"));

    return kOk;
}

/**
 * Prints the function key help text in the status bar.
 *
 * Displays different key bindings depending on whether the editor is in
 * 'edit' mode or 'mark' mode. Adapts the displayed text to terminal width.
 *
 * IMPORTANT: Only call this function via the editor_print_func_keys() wrapper
 *            so that unit-tests can override it.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_print_func_keys_impl(Editor *self) {
    const char *p;

    switch (self->mode) {
        case kEditMode:
        case kExitMode:
            if (self->width >= 78) {
                p = "ESC:Exit  F1:Save  F2:Run  F3:Find  F4:Mark  F5:Paste";
            } else if (self->width >= 62) {
                p = "F1:Save F2:Run F3:Find F4:Mark F5:Paste";
            } else {
                p = "EDIT MODE";
            }
            break;

        case kMarkMode:
            if (self->width >= 49) {
                p = "MARK MODE   ESC=Exit  DEL:Delete  F4:Cut  F5:Copy";
            } else {
                p = "MARK MODE";
            }
            break;

        default:
            return INTERNAL_FAULT_EX("invalid EditorMode: %d", self->mode);
    }

    ON_FAILURE_RETURN(editor_set_cursor_pos(self, 0, self->height));
    ON_FAILURE_RETURN(editor_draw_line(self));
    ON_FAILURE_RETURN(editor_highlight(self, kHighlightStatus));
    ON_FAILURE_RETURN(display_puts(p));
    ON_FAILURE_RETURN(editor_highlight(self, kHighlightNormal));
    ON_FAILURE_RETURN(display_clear_to_end_of_line());

    // Restore cursor position
    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Gets input from the user with a prompt displayed in the status area.
 *
 * Displays a prompt and reads user input character by character, handling
 * backspace and storing the result in the global inpbuf. Supports early
 * exit with ESC or F3 keys.
 *
 * @param  self    Pointer to the Editor instance.
 * @param  prompt  The prompt string to display to the user.
 * @return         kOk on success, or an error code on failure.
 */
static MmResult editor_get_input(Editor *self, const char *prompt) {
    ON_FAILURE_RETURN(editor_set_cursor_pos(self, 0, self->height + 1));
    ON_FAILURE_RETURN(display_puts(prompt));
    ON_FAILURE_RETURN(display_clear_to_end_of_line());

    const size_t prompt_len = strlen(prompt);
    self->cx = (int) prompt_len;
    self->cy = self->height + 1;
    ON_FAILURE_RETURN(editor_set_cursor_pos(self, self->cx, self->cy));

    char *p = inpbuf;
    char *const max_p = inpbuf + INPBUF_SIZE - 1; // Pre-calculate limit
    *p = '\0';

    bool exit_loop = false;
    while (!exit_loop) {
        int ch = -1;
        ON_FAILURE_RETURN(prompt_getc(&ch));
        if (ch == self->saved_break_key) ch = ESC;

        switch (ch) {
            case '\r':
                exit_loop = true;
                break;

            case ESC:
                *inpbuf = '\0';
                exit_loop = true;
                break;

            case F3:
            case SHIFT_FN(F3):
                inpbuf[0] = (char) ch;
                inpbuf[1] = '\0';
                exit_loop = true;
                break;

            case BKSP:
                if (p > inpbuf) {
                    p--;   // Remove previous character
                    *p = '\0';  // Always keep inpbuf '\0' terminated
                    self->cx--;
                    ON_FAILURE_RETURN(display_puts("\b \b"));  // Erase on screen
                    ON_FAILURE_RETURN(display_flush());
                } else {
                    ON_FAILURE_RETURN(display_bell());
                }
                break;

            default:
                // Check 1: Is it a printable character?
                // Check 2: Is there room in the physical buffer?
                // Check 3: Is there room on the screen line?
                if (!isprint(ch) || p >= max_p || self->cx >= self->width - 1) {
                    ON_FAILURE_RETURN(display_bell());
                    break;
                }

                *p++ = (char) ch;
                *p = '\0';  // Keep inpbuf '\0' terminated
                self->cx++;
                ON_FAILURE_RETURN(display_putc(ch));
                ON_FAILURE_RETURN(display_flush());
                break;
        }
    }

    return editor_print_func_keys(self);
}

/**
 * Displays a message in the status line area.
 *
 * Shows a message with inverse video (error highlighting) in the status area,
 * typically used for error messages or warnings.
 *
 * IMPORTANT: Only call this function via the editor_print_msg() wrapper
 *            so that unit-tests can override it.
 *
 * @param  self  Pointer to the Editor instance.
 * @param  msg   The message string to display.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_print_msg_impl(Editor *self, const char *msg) {
    ON_FAILURE_RETURN(editor_set_cursor_pos(self, 0, self->height + 1));
    ON_FAILURE_RETURN(editor_highlight(self, kHighlightError));
    ON_FAILURE_RETURN(display_inverse(true));
    ON_FAILURE_RETURN(display_puts(msg));
    ON_FAILURE_RETURN(editor_highlight(self, kHighlightNormal));
    ON_FAILURE_RETURN(display_reset());
    ON_FAILURE_RETURN(display_clear_to_end_of_line());
    ON_FAILURE_RETURN(editor_set_cursor_pos(self, self->cx, self->cy));
    return kOk;
}

/**
 * Moves a pointer back by a specified number of characters within the current line.
 *
 * @param  self       Pointer to the Editor instance.
 * @param  start      The starting position pointer within the text buffer.
 * @param  num_chars  The number of characters to move back.
 * @return            Pointer moved back by num_chars, or to the start of the line if
 *                    num_chars exceeds the distance to line start. Returns NULL on error.
 */
char *editor_back_in_line(Editor *self, char *start, size_t num_chars) {
    if (self == NULL) {
        LOG_ERROR("invalid null parameter: self");
        return NULL;
    }

    if (start == NULL) {
        LOG_ERROR("invalid null parameter: start");
        return NULL;
    }

    // Validate that start is within buffer bounds
    if (start < self->buf || start >= self->buf + self->buf_sz) {
        LOG_ERROR("start position outside buffer bounds");
        return NULL;
    }

    // Early return for zero movement
    if (num_chars == 0) {
        return start;
    }

    // Find the start of the current line
    char *line_start = start;
    while (line_start > self->buf && *(line_start - 1) != '\n') {
        line_start--;
    }

    // Calculate how far we can actually move back
    size_t max_move = start - line_start;
    size_t actual_move = (num_chars > max_move) ? max_move : num_chars;

    return start - actual_move;
}

/**
 * Finds a case-insensitive substring within the current line of the text buffer.
 *
 * Searches for the given needle string within the line where the cursor
 * (start) is positioned. The search is performed only on the current line,
 * bounded by newline characters or the buffer boundaries.
 *
 * @param  self     Pointer to the Editor instance.
 * @param  needle   The case-insensitive substring to search for.
 * @param  start    The starting position pointer within the text buffer.
 * @param  max_len  The maximum length to search within the line.
 * @return          Pointer to the start of the found substring, or NULL if not found.
 *
 * @note The search is case-insensitive.
 * @note This function does not modify the buffer.
 * @note The current line is defined as text between newline characters,
 *       or from the buffer start/end if no newlines are present.
 */
char *editor_find_in_line(Editor *self, const char *needle, char *start, size_t max_len) {
    if (self == NULL) {
        LOG_ERROR("invalid null parameter: self");
        return NULL;
    }

    if (needle == NULL) {
        LOG_ERROR("invalid null parameter: needle");
        return NULL;
    }

    if (*needle == '\0') {
        LOG_ERROR("invalid empty parameter: needle");
        return NULL;
    }

    if (start == NULL) {
        LOG_ERROR("invalid null parameter: start");
        return NULL;
    }

    // Ensure start is within the buffer bounds
    if (start < self->buf || start >= self->buf + self->buf_sz) {
        LOG_ERROR("start position outside buffer bounds");
        return NULL;
    }

    // Early return for zero max_len
    if (max_len == 0) {
        return NULL;
    }

    size_t needle_len = strlen(needle);
    if (needle_len > max_len) {
        return NULL;
    }

    // Find end of line from start position
    const char *buffer_end = self->buf + self->buf_sz;
    const char *line_end = start;

    while (line_end < buffer_end && *line_end != '\0' && *line_end != '\n') {
        line_end++;
    }

    // Calculate actual search area considering max_len and buffer bounds
    size_t line_len = line_end - start;
    size_t search_len = (max_len < line_len) ? max_len : line_len;

    // Ensure we don't search beyond buffer bounds
    const char *search_end = start + search_len;
    if (search_end > buffer_end) {
        search_end = buffer_end;
        search_len = search_end - start;
    }

    // Check if needle can fit in search area
    if (needle_len > search_len) {
        return NULL;
    }

    // Manual case-insensitive substring search
    // Safe loop bounds: ensure we don't go past search_end - needle_len
    const char *max_start_pos = search_end - needle_len;
    for (char *p = start; p <= max_start_pos; p++) {
        bool match = true;
        for (size_t i = 0; i < needle_len; i++) {
            if (tolower((unsigned char)p[i]) != tolower((unsigned char)needle[i])) {
                match = false;
                break;
            }
        }
        if (match) {
            return p;
        }
    }

    return NULL;
}

/**
 * Is the given character printable?
 */
static inline bool editor_is_printable(char ch) {
    return ch == '\n' || (ch >= ' ' && ch <= '~');
//    return ch >= ' ' && ch <= '~';
}

/**
 * Inserts a character into the text buffer at the current position.
 *
 * Shifts all text after the current position down by one character to make
 * room for the new character. Non-printable characters are ignored. Checks for
 * available buffer space and enforces line length limits before inserting.
 *
 * When syntax highlighting is enabled, detects interactions that affect
 * multiline comment markers. These cases trigger a full screen redraw since
 * they can change syntax highlighting for many subsequent lines.
 *
 * @param       self    Pointer to the Editor instance.
 * @param       ch      The character to insert.
 * @return              kOk on success, or an error code on failure.
 */
static MmResult editor_insert_char(Editor *self, char ch) {
    ON_INVALID_TXTP_RETURN();

    // Ignore non-printable characters
    if (!editor_is_printable(ch)) return kOk;

    // Limit line length
    if (ch != '\n' && editor_line_length(self, self->txtp) >= MAX_LINE_LENGTH) {
        return mmresult_ex(kEditorError, EMSG_LINE_TOO_LONG);
    }

    // Check that the buffer is not full
    char *p;
    for (p = self->buf; *p; p++); // Find end of text
    if (p >= self->buf + self->buf_sz - 1) return mmresult_ex(kEditorError, EMSG_EDIT_BUFFER_FULL);

    // Inserting a newline always requires a redraw
    const int current_line = self->py + self->cy;
    bool multiple_line_change = false;
    if (ch == '\n') multiple_line_change = true;

    // Check for interactions that make or break multiline comments
    if (!multiple_line_change && mmb_options.syntax_highlight) {
        char previous = (self->txtp > self->buf) ? *(self->txtp - 1) : '\0';
        switch (ch) {
            case '/':
                if (previous == '*' || *self->txtp == '*') {
                    // Inserting / before or after *
                    multiple_line_change = true;
                }
                break;
            case '*':
                if (previous == '/' || *self->txtp == '/') {
                    // Inserting * before or after /
                    multiple_line_change = true;
                }
                break;
            case '\'':
                if (editor_find_in_line(self, "/*", self->txtp, MAX_LINE_LENGTH) != NULL) {
                    // Inserting \ before /*
                    multiple_line_change = true;
                }
                break;
            case '"':
                if (editor_find_in_line(self, "/*", self->txtp, MAX_LINE_LENGTH) != NULL) {
                    // Inserting " before /*
                    multiple_line_change = true;
                }
                break;
            default:
                break;
        }
    }

    // Shift everything up one place to make room
    for (; p >= self->txtp; p--) {
        *(p + 1) = *p;
    }

    // Insert the character into the buffer
    *self->txtp++ = ch;
    self->text_changed = true;

    // Check for a completed REM command before /*
    if (!multiple_line_change
            && mmb_options.syntax_highlight
            && editor_find_in_line(
                self,
                "REM",
                editor_back_in_line(self, self->txtp, 3),
                5)
            && (editor_find_in_line(self, "/*", self->txtp, MAX_LINE_LENGTH) != NULL)) {
        multiple_line_change = true;
    }

    if (multiple_line_change) {
        ON_FAILURE_RETURN(editor_set_changed_lines(self, current_line, LAST_LINE));
    } else {
        ON_FAILURE_RETURN(editor_set_changed_lines(self, current_line, current_line));
    }

    // Update the cursor position
    if (ch == '\n') {
        self->cx = 0;
        self->px = 0;
        self->cy++;
        self->num_lines++;
    } else {
        self->cx++;
    }
    ON_FAILURE_RETURN(editor_adjust_viewport(self));

    return kOk;
}

/**
 * Prints the current line and column position in the status area.
 *
 * Displays the current cursor position (1-based line and column numbers)
 * and the current insert/overwrite mode (INS/OVR).
 *
 * IMPORTANT: Only call this function via the editor_print_status() wrapper
 *            so that unit-tests can override it.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_print_status_impl(Editor *self) {
    char s[64];
    snprintf(s, 64, "Ln: %d  Col: %d       ",
             self->py + self->cy + 1,
             self->cx + self->px + 1);
    strcpy(s + 19, self->insert ? "INS" : "OVR");

    ON_FAILURE_RETURN(editor_set_cursor_pos(self, self->width - 25, self->height + 1));
    ON_FAILURE_RETURN(editor_highlight(self, kHighlightStatus));
    ON_FAILURE_RETURN(display_puts(s));
    ON_FAILURE_RETURN(editor_highlight(self, kHighlightNormal));

    // Restore cursor position
    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Compares a string against a keyword token for syntax highlighting.
 *
 * Performs case-insensitive comparison, stopping at word boundaries or
 * special characters like '(' or '$'.
 *
 * @param  p    Pointer to the string in the text buffer to check.
 * @param  tkn  The keyword token to compare against.
 * @return      true if the string matches the token, false otherwise.
 */
static bool editor_strcmp(char *p, const char *tkn) {
    while (*tkn && (toupper(*tkn) == toupper(*p))) {
        if (*tkn == '(' && *p == '(') return true;
        if (*tkn == '$' && *p == '$') return true;
        tkn++;
        p++;
    }
    if (*tkn == 0 && !isnamechar(*p)) return true;  // return the string if successful

    return false;  // or NULL if not
}

/**
 * Safely returns the character at the given position in the buffer.
 *
 * Checks if the pointer is within buffer bounds before dereferencing.
 *
 * @param  self  Pointer to the Editor instance.
 * @param  p     Pointer to position in the text buffer.
 * @return       The character at the position, or '\0' if out of bounds.
 */
static inline char editor_safe_char(Editor *self, char *p) {
    if (p < self->buf || p >= self->buf + self->buf_sz) {
        return '\0';
    }
    return *p;
}

// This is a list of keywords that can come after the OPTION and GUI commands
// the list must be terminated with a NULL
const char *TWO_KEYWORD_TBL[] = {
    "BASE",    "EXPLICIT", "DEFAULT",    "BREAK",   "AUTORUN", "BAUDRATE", "DISPLAY",
#if defined(GUICONTROLS)
    "BUTTON",  "SWITCH",   "CHECKBOX",   "RADIO",   "LED",     "FRAME",    "NUMBERBOX",
    "SPINBOX", "TEXTBOX",  "DISPLAYBOX", "CAPTION", "DELETE",  "DISABLE",  "HIDE",
    "ENABLE",  "SHOW",     "FCOLOUR",    "BCOLOUR", "REDRAW",  "BEEP",     "INTERRUPT",
#endif
    NULL};

// This is a list of common keywords that should be highlighted as such
// the list must be terminated with a NULL
const char *SPECIAL_KEYWORDS[] = {
    "SELECT", "INTEGER", "FLOAT",  "STRING", "DISPLAY",
    "SDCARD", "OUTPUT",  "APPEND", "WRITE",  "SLAVE",
    "TARGET", "PROGRAM", NULL};

/**
 * Gets the syntax highlighting color for the current character.
 *
 * @param       self       Pointer to the Editor instance.
 * @param       p          Pointer to the current character.
 * @param[out]  highlight  On exit, the syntax highlighting to use.
 * @return                 kOk on success, or an error code on failure.
 */
MmResult editor_get_highlight(Editor *self, SyntaxState *syntax, char *p,
                              HighlightType *highlight) {
    CHECK_PARAM(p != NULL);

    if (!mmb_options.syntax_highlight) {
        *highlight = kHighlightNormal;
        return kOk;
    }

    // Check for the start of a multiline comment
    const char next = editor_safe_char(self, p + 1);
    if (*p == '/' && next == '*' && !syntax->inquote) {
        *highlight = kHighlightComment;
        syntax->multiline_comment++;
        return kOk;
    }

    // Check for the end of a multiline comment
    // Watch for the edge case /*/ sequence which does not end a comment
    const char previous = editor_safe_char(self, p - 1);
    const char previous2 = editor_safe_char(self, p - 2);
    if (*p == '/' && previous == '*' && previous2 != '/' && !syntax->inquote) {
        if (syntax->multiline_comment > 0) {
            syntax->multiline_comment--;
        }
        *highlight = kHighlightComment; // Still highlighted as comment.
        return kOk;
    }

    // Check for trailing whitespace
    if (*p == ' ') {
        char *q = p + 1;
        while (*q == ' ') q++;
        if (*q == '\n' || *q == '\0') {
            *highlight = kHighlightTrailingWhitespace;
            return kOk;
        }
    }

    // Within a multiline comment all chars are comments
    if (syntax->multiline_comment > 0) {
        *highlight = kHighlightComment;
        return kOk;
    }

    // Check for a single-line comment char
    if (*p == '\'' && !syntax->inquote) {
        syntax->incomment = true;
        *highlight = kHighlightComment;
        return kOk;
    }

    // Once in a comment all the following chars must be comments
    if (syntax->incomment || syntax->multiline_comment) {
        *highlight = kHighlightComment;
        return kOk;
    }

    // Check for a quoted string
    if (*p == '\"') {
        syntax->inquote = !syntax->inquote;
        *highlight = kHighlightQuote;
        return kOk;
    }

    // Once in a string all the following chars must be part of that string
    if (syntax->inquote) {
        *highlight = kHighlightQuote;
        return kOk;
    }

    // Check that we are still in a keyword
    if (syntax->inkeyword) {
        if (isnamechar(*p) || *p == '$') {
            *highlight = kHighlightKeyword;
        } else {
            syntax->inkeyword = false;
            *highlight = kHighlightNormal;
        }
        return kOk;
    }

    // Check that we are still in a number
    if (syntax->innumber) {
        const char upper = toupper(*p);
        if (!isdigit(*p) && !(upper >= 'A' && upper <= 'F') && upper != 'O' && upper != 'H' && *p != '.') {
            syntax->innumber = false;
            *highlight = kHighlightNormal;
        } else {
            *highlight = kHighlightNumber;
        }
        return kOk;
    }

    // Check if we are staring a number
    if (!syntax->intext) {
        if (isdigit(*p) || *p == '&' || ((*p == '-' || *p == '+' || *p == '.') && isdigit(p[1]))) {
            syntax->innumber = true;
            *highlight = kHighlightNumber;
            return kOk;
        }
        // Check if this is an 8 digit hex number as used in CFunctions
        int i = 0;
        for (i = 0; i < 8; i++) {
            if (!isxdigit(p[i])) break;
        }
        if (i == 8 && (p[8] == ' ' || p[8] == '\'' || p[8] == 0)) {
            syntax->innumber = true;
            *highlight = kHighlightNumber;
            return kOk;
        }
    }

    // Check if we are starting a keyword
    if (isnamechar(*p) && !syntax->intext) {
        // Check the command table for a match
        for (int i = 0; i < commandtbl_size - 1; i++) {
            if (editor_strcmp(p, commandtbl[i].name) != 0 ||
                ((editor_strcmp(&p[1], &commandtbl[i].name[1]) != 0) && *p == '.' &&
                 *commandtbl[i].name == '_')) {
                if (editor_strcmp(p, "REM") != 0) {  // special case, REM is a comment
                    syntax->incomment = true;
                    *highlight = kHighlightComment;
                    return kOk;
                } else {
                    *highlight = kHighlightKeyword;
                    syntax->inkeyword = true;
                    if (editor_strcmp(p, "GUI") || editor_strcmp(p, "OPTION")) {
                        syntax->twokeyword = p;
                        while (isalnum(*syntax->twokeyword)) syntax->twokeyword++;
                        while (*syntax->twokeyword == ' ') syntax->twokeyword++;
                    }
                    return kOk;
                }
            }
        }

        // Check the function/token table for a match
        for (int i = 0; i < tokentbl_size - 1; i++) {
            if (editor_strcmp(p, tokentbl[i].name) != 0) {
                syntax->inkeyword = true;
                *highlight = kHighlightKeyword;
                return kOk;
            }
        }

        // Check for the second keyword in two keyword commands
        if (p == syntax->twokeyword) {
            char **pp;
            for (pp = (char **) TWO_KEYWORD_TBL; *pp; pp++) {
                if (editor_strcmp(p, *pp)) break;
            }
            if (*pp) {
                syntax->inkeyword = true;
                *highlight = kHighlightKeyword;
                return kOk;
            }
        }
        if (p >= syntax->twokeyword) syntax->twokeyword = NULL;

        // Check for a range of common keywords
        {
            char **pp;
            for (pp = (char **) SPECIAL_KEYWORDS; *pp; pp++) {
                if (editor_strcmp(p, *pp)) break;
            }
            if (*pp) {
                syntax->inkeyword = true;
                *highlight = kHighlightKeyword;
                return kOk;
            }
        }
    }

    // Keep track of if we are in general text or not;
    // this is to avoid recognising keywords or numbers inside variables
    syntax->intext = isnamechar(*p);

    *highlight = kHighlightNormal;
    return kOk;
}

/**
 * Advances to the start of the next line while tracking comment state.
 *
 * Scans through the current line character by character, maintaining awareness
 * of:
 * - Multiline comments (/ * ... * /) and their nesting level
 * - Single-line comments (starting with ' or REM)
 * - Quoted strings (to avoid treating comment markers inside strings as actual
 *   comments)
 *
 * This function is essential for syntax highlighting as it propagates the
 * multiline comment state from one line to the next. The comment_level is
 * updated as the function encounters / * (increment) and * / (decrement)
 * tokens outside of strings and single-line comments.
 *
 * Special Cases Handled:
 * - Escaped quotes inside strings (preceded by \)
 * - REM keyword (case-insensitive, requires word boundaries)
 * - Comment markers inside quoted strings (ignored)
 * - Single-line comments prevent multiline comment detection for rest of line
 *
 * @param          self        Pointer to the Editor instance.
 * @param          p           Pointer to current position in the text buffer.
 * @param[in,out]  comment_level  Pointer to integer tracking multiline comment
 *                                nesting depth.
 * @return                     Pointer to the first character of the next line
 *                             (character after '\n'), or pointer to '\0' if at
 *                             end of buffer. Returns NULL if any parameter is
 *                             NULL or if buffer end is reached unexpectedly.
 */
char *editor_find_next(Editor *self, char *p, int *comment_level) {
    if (!self || !p || !comment_level) return NULL;
    if (*p == '\0') return p;

    const int NORMAL = 0;        ///< Processing regular code
    const int IN_QUOTE = 1;      ///< Inside a quoted string
    const int IN_SL_COMMENT = 2; ///< Inside a single-line comment
    const int STATE_EOL = 3;     ///< Reached end of line

    int state = NORMAL;

    // TODO: Handle CMM2 #COMMENT {START|END} construct

    while (*p != '\0' && state != STATE_EOL) {
        switch (*p) {
            case '\n':
                state = STATE_EOL;
                break;
            case '/':
                if (state == NORMAL && p[1] == '*') {
                    (*comment_level)++;
                    p++;
                }
                break;
            case '*':
                if (state == NORMAL && *comment_level > 0 && p[1] == '/') {
                    (*comment_level)--;
                    p++;
                }
                break;
            case '\"':
                if (state == NORMAL) {
                    state = IN_QUOTE;
                } else if (state == IN_QUOTE && (p == self->buf || p[-1] != '\\')) {
                    state = NORMAL;
                }
                break;
            case '\'':
                if (state == NORMAL) {
                    state = IN_SL_COMMENT;
                }
                break;
            case 'R':
            case 'r':
                if (state == NORMAL && (editor_find_in_line(self, "EM", p + 1, 2) != NULL)) {
                    // I suspect this may ignore some valid REM comments
                    char previous = editor_safe_char(self, p - 1);
                    char next = editor_safe_char(self, p + 3);
                    if (!isalnum(previous) && !isalnum(next)) {
                        state = IN_SL_COMMENT;
                    }
                }
                break;
            default:
                break;
        }
        p++;
    }

    // Return pointer to start of next line
    return p;
}

/**
 * Finds the start of line N with multiline comment tracking (for rendering).
 *
 * Scans from buffer start while maintaining multiline comment nesting state.
 * Use this before rendering lines to ensure correct syntax highlighting.
 * This is slower than editor_start_of_line_n() due to the state tracking.
 *
 * @param       self           Pointer to the Editor instance.
 * @param       line           Line number (0-based).
 * @param[out]  comment_level  On exit, the multiline comment nesting depth
 *                             at the start of the requested line.
 * @return                     Pointer to first character of line N,
 *                             or NULL if line < 0 or self is NULL.
 */
char *editor_find_line_ex(Editor *self, int line, int *comment_level) {
    if (line < 0) return NULL;
    *comment_level = 0;
    char *p = self->buf;
    for (; line > 0; line--) {
        p = editor_find_next(self, p, comment_level);
    }
    return p;
}

/**
 * Gets the bounds and length of the current text selection.
 *
 * Determines the lower and upper bounds of the selection between mark and txtp,
 * regardless of which is greater. This normalizes the selection so that start
 * is always at or before end in the buffer, making it easier to iterate or copy
 * the selected region.
 *
 * @param       self   Pointer to the Editor instance.
 * @param[out]  start  Set to the lower address (earlier in buffer).
 * @param[out]  end    Set to the higher address (later in buffer).
 * @return             The length of the selection in bytes (end - start).
 *                     Returns 0 if mark == txtp (zero-length selection).
 *
 * @note The returned pointers point into self->buf and should not be freed.
 * @note For a zero-length selection (mark == txtp), start and end will be equal.
 * @note The length is always non-negative since end >= start by construction.
 */
size_t editor_get_selection(Editor *self, char **start, char **end) {
    if (self->txtp > self->mark) {
        // If txtp > mark then selection includes mark but not txtp
        *start = self->mark;
        *end = self->txtp - 1;
        return *end - *start + 1;
    } else if (self->txtp < self->mark) {
        // If txtp < mark then selection includes txtp but not mark
        *start = self->txtp;
        *end = self->mark - 1;
        return *end - *start + 1;
    } else {
        // Zero-length selection
        *start = self->txtp;
        *end = self->txtp;
        return 0;
    }
}

/**
 * Prints a line of text starting from the given position.
 *
 * Displays text from the specified buffer position up to the end of the line
 * or screen width, whichever comes first. Applies syntax highlighting if enabled,
 * including handling of multiline comments and text selection highlighting.
 *
 * When syntax highlighting is disabled, prints from the cursor position (cx)
 * to accommodate horizontal scrolling of the display.
 *
 * @param  self            Pointer to the Editor instance.
 * @param  p               Pointer to the start position in the buffer to print from.
 * @param  comment_level   Initial multiline comment nesting level for syntax
 *                         highlighting. Use 0 if not inside a multiline comment.
 * @param  offset          Horizontal offset for printing (used for scrolling).
 * @return                 kOk on success, or an error code on failure.
 *
 * @note When syntax highlighting is enabled, the entire line is printed from
 *       the given position regardless of cursor column.
 * @note When syntax highlighting is disabled, printing starts from the cursor
 *       column position (cx) to support horizontal scrolling.
 * @note Text within the selection bounds (mark_lb to mark_ub) is highlighted
 *       with inverse video, overriding syntax highlighting.
 */
static MmResult editor_print_line_p(Editor *self, char *p, int comment_level, int offset) {
    // Get selection bounds if required
    char *selection_lb = NULL;
    char *selection_ub = NULL;
    if (self->mode == kMarkMode) (void) editor_get_selection(self, &selection_lb, &selection_ub);

    // Initialise structure used to maintain syntax highlighting state
    SyntaxState syntax = {
        .incomment = false,
        .inquote = false,
        .inkeyword = false,
        .innumber = false,
        .intext = false,
        .twokeyword = NULL,
        .multiline_comment = comment_level
    };

    // We redraw the whole line, so move to the LHS of the display
    ON_FAILURE_RETURN(display_putc('\r'));

    // Display the line from here to the end of the line or the screen width
    for (int x = 0; x < self->width + offset && *p && *p != '\n'; x++) {
        HighlightType new_highlight = kHighlightUnspecified;
        if (mmb_options.syntax_highlight) {
            ON_FAILURE_RETURN(editor_get_highlight(self, &syntax, p, &new_highlight));
        } else {
            new_highlight = kHighlightNormal;
        }

        // If the text is selected, override the highlight type (except for the cursor position).
        // Note we still need to have run editor_get_highlight() to maintain syntax state.
        if (self->mode == kMarkMode && p != self->txtp && p >= selection_lb && p <= selection_ub) {
            new_highlight = kHighlightMark;
        }

        if (new_highlight != self->highlight) {
            ON_FAILURE_RETURN(editor_highlight(self, new_highlight));
        }

        if (x < offset) {
            p++;
            continue;
        }

        char ch = *p++;
        if (x == offset && offset != 0) {
            ON_FAILURE_RETURN(editor_highlight(self, kHighlightRight));
            ch = '<';
        } else if (x == self->width + offset - 1 && *p != '\0' && *p != '\n') {
            ON_FAILURE_RETURN(editor_highlight(self, kHighlightRight));
            ch = '>';
        }

        ON_FAILURE_RETURN(display_putc(ch));
    }

    // Reset syntax highlighting and clear display to end of line
    ON_FAILURE_RETURN(editor_highlight(self, kHighlightNormal));
    ON_FAILURE_RETURN(display_clear_to_end_of_line());

    return kOk;
}

/**
 * Prints a line of text without syntax highlighting (fast path).
 *
 * This is an optimized rendering function used when syntax highlighting is
 * disabled. Unlike editor_print_line_p(), this function:
 * - Skips syntax state tracking entirely
 * - Only renders from the cursor column (cx) onward to the end of line or
 *   screen width, whichever comes first
 * - Does not handle text selection highlighting
 *
 * The function is primarily used for incremental updates when editing a single
 * line in non-syntax-highlighted mode, avoiding the overhead of full-line
 * rendering with state tracking.
 *
 * @param  self  Pointer to the Editor instance.
 * @param  p     Pointer to a position in the text buffer. The function will
 *               find the start of the line containing this position and render
 *               from column cx to the end of the visible area.
 * @return       kOk on success, or an error code on failure.
 *
 * IMPORTANT: Only call this function via the editor_print_lines() wrapper
 *            so that unit-tests can override it.
 *
 * @note The cursor is restored to its original position (cx, cy) after rendering.
 * @note Characters beyond self->width are not displayed.
 * @note This function clears to the end of the line after printing visible text.
 */
MmResult editor_print_line_fast_impl(Editor *self, char *p) {
    char *start = editor_start_of_line(self, p);
    for (int x = p - start; x < self->width && *p && *p != '\n'; x++, p++) {
        ON_FAILURE_RETURN(display_putc(*p));
    }
    ON_FAILURE_RETURN(display_clear_to_end_of_line());
    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Scrolls the editor display up by one line.
 *
 * Moves the viewport up (showing newer content at bottom), increments the
 * page offset, and redraws the newly visible bottom line.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_scroll_up(Editor *self) {
    // Move to end of the editing area
    ON_FAILURE_RETURN(editor_set_cursor_pos(self, 0, self->height));

    // Clear status line
    ON_FAILURE_RETURN(display_clear_to_end_of_screen());

    // Scroll display up
    ON_FAILURE_RETURN(display_scroll_up());

    // Drawing new line at bottom of editing area is handled by caller

    // Restore status line
    ON_FAILURE_RETURN(editor_print_func_keys(self));
    // TODO ON_FAILURE_RETURN(editor_print_status(self));

    // Consume any keystrokes accumulated while scrolling the screen
    keybuf_clear();

    return kOk;
}

/**
 * Scrolls the editor display down by one line.
 *
 * Moves the viewport down (showing older content at top), decrements the
 * page offset, and redraws the newly visible top line.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_scroll_down(Editor *self) {
    // Move to end of the editing area
    ON_FAILURE_RETURN(editor_set_cursor_pos(self, 0, self->height));

    // Clear status lines
    ON_FAILURE_RETURN(display_clear_to_end_of_screen());

    // Scroll display down
    ON_FAILURE_RETURN(display_scroll_down());

    // Drawing new line at top of editing area is handled by caller

    // Restore status line
    ON_FAILURE_RETURN(editor_print_func_keys(self));
    // TODO ON_FAILURE_RETURN(editor_print_status(self));

    // Consume any keystrokes accumulated while redrawing the screen
    keybuf_clear();

    return kOk;
}

/**
 * Deletes the marked text from the buffer.
 *
 * Removes all text between self->mark and self->txtp, updating the line
 * count and setting the text changed flag.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_delete_selection(Editor *self) {
    char *start = NULL;
    char *end = NULL;
    if (editor_get_selection(self, &start, &end) == 0) {
        // Nothing to delete
        self->mode = kEditMode;
        return kOk;
    }

    // Adjust the line count for deleted lines
    char *p;
    for (p = start; p <= end; p++) {
        if (*p == '\n') self->num_lines--;
    }

    // Shuffle the text down copying from the mark pointer to the txtp pointer
    end += 1; // Delete the character at the end position
    for (p = start; *end;) {
        *p++ = *end++;
    }

    // Terminate the text buffer
    *p++ = '\0';
    *p++ = '\0';

    if (self->txtp != start) {
        self->txtp = start;
        ON_FAILURE_RETURN(editor_sync_cursor_to_buffer(self, self->txtp));
    }
    self->text_changed = true;
    self->mode = kEditMode;

    return kOk;
}

/**
 * Copies the marked text to the clipboard without deleting it.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_copy(Editor *self) {
    char *start = NULL;
    char *end = NULL;
    const size_t selection_length = editor_get_selection(self, &start, &end);
    // LOG_DEBUG("start=%p, end=%p, selection_length=%ud", start, end, selection_length);

    if (selection_length > 0) {
        // Check clipboard size limit
        if (selection_length > MAXCLIP) {
            return mmresult_ex(kEditorError, EMSG_CLIPBOARD_OVERFLOW);
        }

        // Copy the selection to clipboard
        memcpy(self->clipboard_buf, start, selection_length);
        self->clipboard_buf[selection_length] = '\0';
    }

    // Exit mark mode
    self->mode = kEditMode;
    return kOk;
}

/**
 * Cuts the marked text to the clipboard (copy and delete).
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_cut(Editor *self) {
    MmResult result = editor_cmd_copy(self);
    if (result != kOk) {
        // Copy failed, do not delete and remain in mark mode
        self->mode = kMarkMode;
        return result;
    }
    return editor_cmd_delete_selection(self);
}

/**
 * Handles ESC key in mark mode.
 *
 * Waits briefly to distinguish between a plain ESC and escape sequences
 * (like mouse events) which it ignores.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_cmd_exit_mark(Editor *self) {
    // Wait 50ms to see if anything more is coming.
    mmtime_sleep_ns(MILLISECONDS_TO_NANOSECONDS(50));
    if (keybuf_get() == '[') {
        // TODO: This is where we would act on input escape sequences,
        //       for the moment we ignore them / empty the input buffer.
        keybuf_clear();
        return kOk;
    }

    self->mode = kEditMode;
    return kOk;
}

/**
 * Converts a keystroke to its canonical form for command processing.
 *
 * Maps various Ctrl key combinations and special keys to their standard
 * command equivalents (e.g., Ctrl-E to UP, Ctrl-X to DOWN).
 *
 * @param  self  Pointer to the Editor instance.
 * @param  key   The raw keystroke from the keyboard.
 * @return       The canonical command key.
 */
static char editor_canonical_key(Editor *self, char key) {
    if (key == self->saved_break_key) {
        return ESC;
    }

// clang-format off
    switch (key) {
        case '\r':         return '\n';
        case CTRLKEY('D'): return RIGHT;
        case CTRLKEY('E'): return UP;
        case CTRLKEY('G'): return SHIFT_FN(F3);
        case CTRLKEY('K'): return END;
        case CTRLKEY('L'): return PDOWN;
        case CTRLKEY('N'): return INSERT;
        case CTRLKEY('P'): return PUP;
        case CTRLKEY('Q'): return F1;
        case CTRLKEY('R'): return F3;
        case CTRLKEY('S'): return LEFT;
        case CTRLKEY('T'): return F4;
        case CTRLKEY('U'): return HOME;
        case CTRLKEY('V'): return F5;
        case CTRLKEY('W'): return F2;
        case CTRLKEY('X'): return DOWN;
        case CTRLKEY('Y'): return F5;
        case CTRLKEY(']'): return DEL;
        default:           return key;
    }
// clang-format on
}

/**
 * Renders one or more consecutive lines to the display with syntax highlighting.
 *
 * This is the primary screen drawing function for the editor. It handles:
 * - Locating the starting line in the buffer
 * - Tracking multiline comment state across lines
 * - Rendering each line with appropriate syntax highlighting
 * - Rendering empty lines when beyond end of buffer (handled by editor_print_line_p)
 * - Consuming accumulated keystrokes to prevent input buffer overflow during
 *   slow rendering
 * - Preserving and restoring the cursor position
 *
 * IMPORTANT: Only call this function via the editor_print_lines() wrapper
 *            so that unit-tests can override it.
 *
 * @param  self   Pointer to the Editor instance.
 * @param  start  The first line to render (0-based, absolute line number).
 *                Must be >= 0 and < num_lines.
 * @param  end    The last line to render (0-based, absolute line number).
 *                May be >= num_lines for rendering empty lines.
 * @return        kOk on success, or an error code on failure.
 */
MmResult editor_print_lines_impl(Editor *self, int start, int end) {
    CHECK_PARAM(self != NULL);
    CHECK_PARAM(start >= 0);
    CHECK_PARAM(start < self->num_lines);
    CHECK_PARAM(end >= start);

    // Early exit if completely beyond viewport
    if (start >= self->py + self->height) {
        LOG_WARN("start line is beyond viewport: start=%d, py=%d, height=%d",
                 start, self->py, self->height);
        return kOk;
    }

    // Clip to viewport boundaries
    start = max(start, self->py);
    end = min(end, self->py + self->height - 1);

    // Move to start of line in viewport
    ON_FAILURE_RETURN(editor_set_cursor_pos(self, 0, start - self->py));

    // Find the starting position in buffer
    int comment_level = 0;
    char *p = editor_find_line_ex(self, start, &comment_level);

    // Render each line
    for (int line = start; line <= end; line++) {
        // editor_print_line_p() handles empty lines when p points to end of buffer
        const int offset = (line == self->py + self->cy) ? self->px : 0;
        ON_FAILURE_RETURN(editor_print_line_p(self, p, comment_level, offset));

        if (line != end) {
            ON_FAILURE_RETURN(display_puts("\r\n"));
        }

        // Advance to next line,
        // p will point to end of buffer if there are no more lines
        if (line != end) {
            p = editor_find_next(self, p, &comment_level);
        }
    }

    // Consume any keystrokes accumulated while drawing
    keybuf_clear();

    // Restore cursor position
    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Reads a keystroke and places it in the keyboard buffer.
 *
 * Blocks until a key is pressed, showing the cursor while waiting. The raw
 * keystroke is converted to canonical form and stored in self->key_buf[0],
 * with self->key_buf[1] set to '\0'.
 *
 * If the keyboard buffer already contains queued keystrokes (from auto-indent
 * or multi-key commands), this function shifts the buffer down to consume the
 * next keystroke instead of reading from the keyboard buffer.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_read_keys(Editor *self) {
    if (self->key_buf[1] != '\0') {
        // Handle queued keystrokes.
        self->key_buf[MAXCLIP + 1] = '\0';
        for (int i = 0; i < MAXCLIP + 1; i++) {
            self->key_buf[i] = self->key_buf[i + 1];
        }
        return kOk;
    }

    // No queued keystrokes - read a fresh keystroke from the keyboard buffer.
    // Show the cursor while waiting for user input.
    int ch = -1;
    ON_FAILURE_RETURN(display_show_cursor(true));

    // Poll until we receive a keystroke.
    // keybuf_get() returns -1 if no key is available.
    // display_update_cursor() ensures cursor blinks/updates while waiting.
    do {
        perform_background_tasks();
        ON_FAILURE_RETURN(display_update_cursor());
        ch = keybuf_get();
    } while (ch == -1);

    // Convert the raw keystroke to canonical form (e.g., Ctrl-E => UP)
    // and store it as a single-character string in the buffer.
    ON_FAILURE_RETURN(display_show_cursor(false));
    self->key_buf[0] = editor_canonical_key(self, (char) ch);
    self->key_buf[1] = '\0';

    return kOk;
}

/**
 * Updates the display based on changes since the last position.
 *
 * Compares the current editor state with the previous position and redraws
 * only the necessary parts of the display (screen, selection, function keys,
 * status line) to reflect any changes.
 *
 * All line numbers (start, end) are absolute line numbers (0-based from
 * start of buffer), not viewport-relative.
 *
 * @param  self  Pointer to the Editor instance.
 * @param  old   Previous cursor and mode state for comparison.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_update_display(Editor *self, Editor *old) {
    // Determine if we need to redraw lines based on structural changes
    if (self->num_lines != old->num_lines || self->mode != old->mode) {
        // Major change: redraw entire viewport
        ON_FAILURE_RETURN(editor_set_changed_lines(self, self->py, LAST_LINE));
    } else if (self->py == old->py + 1) {
        // Scrolled up by one line: hardware scroll + redraw new bottom line
        ON_FAILURE_RETURN(editor_scroll_up(self));
        const int bottom_line = self->py + self->height - 1;
        ON_FAILURE_RETURN(editor_set_changed_lines(self, bottom_line, bottom_line));
    } else if (self->py == old->py - 1) {
        // Scrolled down by one line: hardware scroll + redraw new top line
        ON_FAILURE_RETURN(editor_scroll_down(self));
        ON_FAILURE_RETURN(editor_set_changed_lines(self, self->py, self->py));
    } else if (self->py != old->py) {
        // Jumped multiple lines: redraw entire viewport
        ON_FAILURE_RETURN(editor_set_changed_lines(self, self->py, LAST_LINE));
    }

    if ((self->px != old->px)
            || ((old->px > 0) && (self->py + self->cy != old->py + old->cy))) {
        ON_FAILURE_RETURN(
            editor_set_changed_lines(self, self->py + self->cy, old->py + old->cy));
    }

    // In mark mode, extend redraw range to cover selection changes
    if (self->mode == kMarkMode && self->txtp != old->txtp) {
        const int sel_start = min(self->py + self->cy, old->py + old->cy);
        const int sel_end = max(self->py + self->cy, old->py + old->cy);
        ON_FAILURE_RETURN(editor_set_changed_lines(self, sel_start, sel_end));
    }

    // Draw lines as necessary
    if (self->change_start != NO_CHANGE) {
        if (!mmb_options.syntax_highlight
            && self->mode != kMarkMode
            && self->cy + self->py == self->change_start
            && self->change_start == self->change_end) {
            // Fast non-syntax-highlighting single-line draw
            ON_FAILURE_RETURN(editor_print_line_fast(self, min(self->txtp, old->txtp)));
        } else {
            // Standard syntax-highlighting multi-line draw
            // NOTE: the change start and end are clipped to the viewport boundaries
            //       within editor_print_lines().
            ON_FAILURE_RETURN(editor_print_lines(self, self->change_start, self->change_end));
        }
    }

    // Update status area (message or function keys + status line)
    if (self->message[0] != '\0') {
        ON_FAILURE_RETURN(editor_print_msg(self, self->message));
    } else {
        if (self->mode != old->mode || old->message[0] != '\0') {
            ON_FAILURE_RETURN(editor_print_func_keys(self));
        }

        if (self->mode != old->mode
                || old->message[0] != '\0'
                || self->insert != old->insert
                || self->cx != old->cx
                || self->cy != old->cy
                || self->px != old->px
                || self->py != old->py) {
            ON_FAILURE_RETURN(editor_print_status(self));
        }
    }

    // Clear one-time change flags
    self->change_start = NO_CHANGE;
    self->change_end = NO_CHANGE;

    return kOk;
}

/**
 * Handles the newline/Enter key command.
 *
 * Inserts a newline character and implements auto-indentation by copying
 * leading spaces from the previous line. Redraws the screen to show the
 * new line structure.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_newline(Editor *self) {
    if (self->mode != kEditMode) return display_bell();

    // If the typeahead buffer is empty and we are at the end of a line then
    // count the spaces at the beginning of the line so that we can match the
    // indentation.
    if (self->key_buf[1] == '\0'
            && self->txtp != self->buf
            && (*self->txtp == '\n' || *self->txtp == '\0')) {
        char *p = editor_start_of_line(self, self->txtp);
        int num_spaces = 0;
        while (*p++ == ' ') {
            self->key_buf[++num_spaces] = ' '; // Insert spaces into typeahead buffer
        }
        self->key_buf[num_spaces + 1] = '\0'; // Terminate the typeahead buffer
    }

    // Actually insert the newline character
    return editor_insert_char(self, '\n');
}

/**
 * Attempts to restore the cursor column on a new line.
 *
 * If the stored cursor column (self->stored_cx + self->stored_px) exceeds
 * the length of the new line, moves the cursor to the end of the line.
 * Otherwise, restores the cursor to the stored column position.
 *
 * @param  self         Pointer to the Editor instance.
 * @param  line_start   Pointer to the start of the new line in the buffer.
 * @return              kOk on success, or an error code on failure.
 */
MmResult editor_try_to_restore_column(Editor *self, char *line_start) {
    const int len = editor_line_length(self, line_start);
    if (self->stored_cx + self->stored_px > len) {
        self->cx = len;
        self->px = 0;
        ON_FAILURE_RETURN(editor_adjust_viewport(self));
    } else {
        self->cx = self->stored_cx;
        self->px = self->stored_px;
    }
    self->txtp = line_start + self->cx + self->px;
    return kOk;
}

/**
 * Handles the DOWN arrow key command.
 *
 * Moves the cursor down one line, attempting to maintain the same column
 * position. Scrolls the viewport if necessary.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_down(Editor *self) {
    ON_INVALID_CURSOR_RETURN();

    // Start of next line
    char *p = editor_next_line(self, self->txtp);

    // If the current line is the last line of the document then do nothing
    if (!p) return kOk;

    // Adjust to the same column as we were previously (self->stored_cx),
    // or the end of the line
    ON_FAILURE_RETURN(editor_try_to_restore_column(self, p));

    if (self->cy < self->height - 3 || self->py + self->height == self->num_lines) {
        // If we are less than two lines from the bottom then move the cursor down
        self->cy++;
    } else {
        // Otherwise scroll the document up
        self->py++;
    }

    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Handles the UP arrow key command.
 *
 * Moves the cursor up one line, attempting to maintain the same column
 * position. Scrolls the viewport if necessary.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_up(Editor *self) {
    ON_INVALID_CURSOR_RETURN();

    // Start of previous line
    char *p = editor_previous_line(self, self->txtp);

    // If in the top row of the first page then do nothing
    if (!p || (self->cy == 0 && self->py == 0)) return kOk;

    // Adjust to the same column as we were previously (self->stored_cx),
    // or the end of the line
    ON_FAILURE_RETURN(editor_try_to_restore_column(self, p));

    if (self->cy > 2 || self->py == 0) {
        // If we are more than two lines from the top then move the cursor up
        self->cy--;
    } else {
        // Otherwise scroll the document down
        self->py--;
    }

    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Handles the LEFT arrow key command.
 *
 * Moves the cursor left one character. If at the start of a line, wraps
 * to the end of the previous line.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_left(Editor *self) {
    // If at the beginning of the document then do nothing
    if (self->txtp == self->buf) {
        return kOk;
    }

    // If at the beginning of a line then move to the end of the previous line
    if (*(self->txtp - 1) == '\n') {
        self->key_buf[1] = UP;
        self->key_buf[2] = END;
        self->key_buf[3] = '\0';
        return kOk;
    }

    // Move cursor back one character
    self->txtp--;
    self->cx--;

    // If necessary pan view of current line left
    ON_FAILURE_RETURN(editor_adjust_viewport(self));

    return kOk;
}

/**
 * Handles the RIGHT arrow key command.
 *
 * Moves the cursor right one character. If at the end of a line, wraps
 * to the start of the next line. Prevents moving past the line width limit.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_right(Editor* self) {
    // If we are at the end of the document then do nothing
    if (*self->txtp == '\0') {
        return kOk;
    }

    // If at the end of a line then move to the beginning of the next line
    if (*self->txtp == '\n') {
        self->key_buf[1] = HOME;
        self->key_buf[2] = DOWN;
        self->key_buf[3] = '\0';
        return kOk;
    }

    // Move cursor forward one character
    self->txtp++;
    self->cx++;

    // If necessary pan view of current line right
    ON_FAILURE_RETURN(editor_adjust_viewport(self));

    return kOk;
}

/**
 * Deletes the character at the current cursor position from the buffer.
 *
 * Removes the character at self->txtp and shifts all subsequent text left by
 * one position.
 *
 * When syntax highlighting is enabled, checks if deleting the character breaks
 * or creates multiline comment markers which would require a full screen redraw.
 *
 * @param       self    Pointer to the Editor instance.
 * @return              kOk on success, or an error code on failure.
 */
static MmResult editor_delete_char(Editor *self) {
    CHECK_PARAM(self->txtp >= self->buf);
    CHECK_PARAM(self->txtp < self->buf + self->buf_sz);

    if (*self->txtp == '\0') return kOk;

    const char currdel = *(self->txtp);
    const char nextdel = editor_safe_char(self, self->txtp + 1);
    const char lastdel = editor_safe_char(self, self->txtp - 1);

    // Delete the character from the buffer
    char *p = self->txtp;
    while (*p) {
        p[0] = p[1];
        p++;
    }
    self->text_changed = true;

    // Deleting a newline character requires a screen redraw,
    // otherwise we just redraw the current line ...
    const int current_line = self->py + self->cy;
    bool multiple_line_change = false;
    if (currdel == '\n') {
        self->num_lines--;
        multiple_line_change = true;
    }

    // ... unless we are syntax highlighting in which case we also need to
    // check for multiline comments being invalidated.
    if (!multiple_line_change && mmb_options.syntax_highlight) {
        bool potential_multiline_change = false;
        switch (currdel) {
            case '/':
                if ((nextdel == '*') || (lastdel == '*')) {
                    multiple_line_change = true;
                }
                break;
            case '*':
                if ((nextdel == '/') || (lastdel == '/')) {
                    multiple_line_change = true;
                }
                break;
            case '\'':
                potential_multiline_change = true;
                break;
            case 'R':
            case 'r':
                potential_multiline_change =
                    editor_find_in_line(self, "em", self->txtp, 2) != NULL;
                break;
            case 'E':
            case 'e':
                potential_multiline_change = (tolower(lastdel) == 'r') && (tolower(nextdel) == 'm');
                break;
            case 'M':
            case 'm':
                potential_multiline_change =
                    editor_find_in_line(self, "re", self->txtp - 2, 2) != NULL;
                break;
            default:
                break;
        }

        if (potential_multiline_change) {
            if (editor_find_in_line(self, "/*", self->txtp, MAX_LINE_LENGTH) != NULL) {
                multiple_line_change = true;
            } else if (editor_find_in_line(self, "*/", self->txtp, MAX_LINE_LENGTH) != NULL) {
                multiple_line_change = true;
            }
        }
    }

    if (multiple_line_change) {
        ON_FAILURE_RETURN(editor_set_changed_lines(self, current_line, LAST_LINE));
    } else {
        ON_FAILURE_RETURN(editor_set_changed_lines(self, current_line, current_line));
    }

    return kOk;
}

/**
 * Handles the DELETE key command.
 *
 * Deletes the character at the current cursor position using
 * editor_delete_char().
 * The redraw strategy is determined by what was deleted:
 * - If a newline or multiline comment marker was affected,
 *   redraws the entire screen
 * - If only regular text was deleted, redraws just the current line
 * - If nothing was deleted (at end of buffer), no redraw occurs
 *
 * Syntax highlighting considerations are handled automatically by
 * editor_delete_char(), which detects when multiline comment markers are
 * are affected and triggers a full screen redraw when necessary.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_delete(Editor *self) {
    CHECK_PARAM(self->mode == kEditMode);
    return editor_delete_char(self);
}

/**
 * Handles the BACKSPACE key command.
 *
 * Deletes the character before the cursor. Implements smart tab deletion:
 * if multiple spaces precede the cursor at tab boundaries, deletes back
 * to the previous tab stop. Wraps to the end of the previous line if at
 * the start of a line.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_backspace(Editor *self) {
    CHECK_PARAM(self != NULL);

    if (self->mode != kEditMode) return display_bell();
    if (self->txtp == self->buf) return kOk;

    if (*(self->txtp - 1) == '\n') {  // if at the beginning of the line wrap around
        self->key_buf[1] = UP;
        self->key_buf[2] = END;
        self->key_buf[3] = DEL;
        self->key_buf[4] = '\0';
        return kOk;
    }

    // If deleting spaces at the start of a line then adjust cursor and add
    // DEL keys to the typeahead buffer to delete to the previous tabstop.
    char *p = editor_start_of_line(self, self->txtp);
    int num_spaces = 0;
    while (*p++ == ' ') num_spaces++;
    if (p == self->txtp + 1 && num_spaces > 0) {
        num_spaces %= mmb_options.tab;
        if (num_spaces == 0) num_spaces = mmb_options.tab;
        self->key_buf[num_spaces + 1] = '\0';
        while (num_spaces--) {
            self->key_buf[num_spaces + 1] = DEL;
            self->txtp--;
            self->cx--;
        }
        return editor_adjust_viewport(self);
    }

    self->txtp--;
    if (self->px > 0) {
        self->px--;
    } else {
        self->cx--;
    }

    return editor_cmd_delete(self);
}

/**
 * Handles the INSERT key command.
 *
 * Toggles between insert mode and overwrite mode.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_cmd_insert(Editor *self) {
    if (self->mode != kEditMode) return display_bell();
    self->insert = !self->insert;
    return kOk;
}

/**
 * Moves the cursor to the start of the buffer.
 *
 * Positions the cursor at the first character of the buffer, resets the
 * viewport to show the first page, and redraws the screen if the viewport
 * has moved.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_move_to_start(Editor *self) {
    self->txtp = self->buf;
    self->cx = 0;
    self->cy = 0;
    self->px = 0;
    self->py = 0;
    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Handles the HOME key command.
 *
 * First press: moves to the start of the current line.
 * Second consecutive press: moves to the start of the file and redraws screen.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_home(Editor *self) {
    // If we are at the start of the document then do nothing
    if (self->txtp == self->buf) return kOk;

    // If this is the second time HOME has been pressed in succession
    // then jump to the start of the file
    if (self->last_key == HOME) {
        return editor_move_to_start(self);
    }

    self->txtp = editor_start_of_line(self, self->txtp);
    self->cx = 0;
    self->px = 0;

    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Moves the cursor to the end of the last line in the text buffer.
 *
 * Navigates to the last line of the buffer, positions the cursor at the end
 * of that line, and adjusts the viewport (py) to display the last page of text.
 * If the buffer has more lines than can fit on the screen, positions the last
 * line at the bottom of the screen and redraws the screen if the viewport
 * has moved.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_move_to_end(Editor *self) {
    self->txtp = editor_start_of_line_n(self, self->num_lines - 1);
    self->cy = self->num_lines - 1;
    self->cx = editor_line_length(self, self->txtp);  // End of last line
    self->txtp += self->cx;
    self->px = 0;
    self->py = 0;
    ON_FAILURE_RETURN(editor_adjust_viewport(self));
    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Handles the END key command.
 *
 * First press: moves to the end of the current line.
 * Second consecutive press: moves to the end of the file and redraws screen.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_end(Editor *self) {
    // If we are at the end of the document then do nothing
    if (*self->txtp == '\0') return kOk;

    // If this is the second time END has been pressed in succession then jump
    // to the end of the file
    if (self->last_key == END) {
        return editor_move_to_end(self);
    }

    self->cx = editor_line_length(self, self->txtp);
    self->txtp = editor_end_of_line(self, self->txtp);

    self->px = 0;
    ON_FAILURE_RETURN(editor_adjust_viewport(self));

    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Handles the PAGE UP key command.
 *
 * Scrolls up by one screenful (or to the top of file if less than a
 * screenful remains). If already at top, queues HOME HOME to go to start.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_page_up(Editor *self) {
    // If already showing the top of the text then move to start of text
    if (self->py == 0) {
        self->key_buf[1] = HOME;
        self->key_buf[2] = HOME;
        self->key_buf[3] = '\0';
        return kOk;
    }

    // Determine number of lines we need to move up
    int lines_up = min(self->py, self->height);
    self->py -= lines_up;

    // Move txtp back that number of lines
    while (lines_up--) {
        char *p = editor_previous_line(self, self->txtp);
        if (!p) {
            return INTERNAL_FAULT_EX("number of lines inconsistent");
        }
        self->txtp = p;
    }

    // Adjust cx and txtp to the same column as previously, or the end of the line
    const int len = editor_line_length(self, self->txtp);
    self->cx = min(self->cx, len);
    self->px = 0;
    self->txtp += self->cx;

    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Handles the PAGE DOWN key command.
 *
 * Scrolls down by one screenful (or to the bottom of file if less than a
 * screenful remains). If already at bottom, queues END END to go to end.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_page_down(Editor *self) {
    // If already showing the bottom of the text then move to end of text
    if (self->num_lines <= self->py + self->height + 1) {
        self->key_buf[1] = END;
        self->key_buf[2] = END;
        self->key_buf[3] = '\0';
        return kOk;
    }

    // Determine number of lines we need to move down
    int lines_down = min(self->num_lines - self->height - self->py, self->height);
    self->py += lines_down;

    // Move txtp forward that number of lines
    while (lines_down--) {
        char *p = editor_next_line(self, self->txtp);
        if (!p) {
            return INTERNAL_FAULT_EX("number of lines inconsistent");
        }
        self->txtp = p;
    }

    // Adjust cx and txtp to the same column as previously, or the end of the line
    const int len = editor_line_length(self, self->txtp);
    self->cx = min(self->cx, len);
    self->px = 0;
    self->txtp += self->cx;

    return editor_set_cursor_pos(self, self->cx, self->cy);
}

/**
 * Handles the TAB key command.
 *
 * Inserts spaces to the next tab stop position based on mmb_options.tab
 * setting. Places the spaces in the keyboard buffer for processing.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_cmd_tab(Editor *self) {
    if (self->mode != kEditMode) return display_bell();
    strcpy(self->key_buf, "        ");
    self->key_buf[mmb_options.tab - (self->cx % mmb_options.tab)] = '\0';
    return kOk;
}

/**
 * Handles the F1 key command (save and exit).
 *
 * Validates that no lines exceed MAX_LINE_LENGTH characters,
 * saves the file if modified, and exits the editor. Clears and resets the display.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_cmd_save_and_exit(Editor *self) {
    if (self->mode != kEditMode) return display_bell();

    int line = -1;
    int length = -1;
    ON_FAILURE_RETURN(editor_find_longest_line(self, &line, &length));
    if (length > MAX_LINE_LENGTH) return mmresult_ex(kEditorError, "LINE %d TOO LONG", line);

    // Clear and reset display
    ON_FAILURE_RETURN(display_cls());
    ON_FAILURE_RETURN(editor_highlight(self, kHighlightNormal));
    ON_FAILURE_RETURN(display_reset());

    // Save program
    if (self->text_changed && self->fname) {
        ON_FAILURE_RETURN(editor_save_file(self, self->fname));
    }

    self->mode = kExitMode;

    return kOk;
}

/**
 * Handles the F2 key command (save and run).
 *
 * Saves the file and prepares to run the program. Clears the runtime and
 * prepares the program for execution.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_cmd_save_and_run(Editor *self) {
    if (self->mode != kEditMode) return display_bell();

    ON_FAILURE_RETURN(editor_cmd_save_and_exit(self));
    if (self->mode != kExitMode) return kOk;

    ON_FAILURE_RETURN(ClearRuntime());
    ON_FAILURE_RETURN(PrepareProgram(true));
    if (*ProgMemory == T_NEWLINE) nextstmt = ProgMemory;
    return kOk;
}

/**
 * Handles the ESC key command (exit without saving).
 *
 * If text has been modified, prompts for confirmation before exiting.
 * Otherwise exits immediately.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_cmd_exit(Editor *self) {
    CHECK_PARAM(self->mode == kEditMode);

    // Wait 50ms to see if anything more is coming.
    mmtime_sleep_ns(MILLISECONDS_TO_NANOSECONDS(50));
    if (keybuf_get() == '[') {
        // TODO: This is where we would act on input escape sequences,
        //       for the moment we ignore them / empty the input buffer.
        keybuf_clear();
        return kOk;
    }

    // This must be an ordinary escape (not part of an escape code)
    if (self->text_changed) {
        ON_FAILURE_RETURN(editor_get_input(self, "Exit and discard all changes (Y/N): "));
        if (toupper(*inpbuf) != 'Y') return kOk;
    }

    self->mode = kExitMode;

    return kOk;
}

/**
 * Searches for the next occurrence of the search string.
 *
 * Continues searching from the current position, wrapping around to the
 * beginning if necessary. Updates the display to show the found text.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_search_again(Editor *self) {
    if (self->mode != kEditMode) return display_bell();

    char *p = self->txtp;
    if (*p == 0) p = self->buf - 1;
    size_t len = strlen(tknbuf);
    while (1) {
        p++;
        if (p == self->txtp) break;
        if (*p == 0) p = self->buf;
        if (p == self->txtp) break;
        if (memcmp(p, tknbuf, len) == 0) break;
    }
    if (p == self->txtp) return mmresult_ex(kEditorError, EMSG_NOT_FOUND);
    int y;
    for (y = 0, self->txtp = self->buf; self->txtp != p;
         self->txtp++) {  // find the line and column of the string
        if (*self->txtp == '\n') {
            y++;  // y is the line
        }
    }
    self->py = y - self->height / 2;  // self->py is the line displayed at the top
    if (self->py < 0) self->py = 0;   // compensate if we are near the start

    return editor_sync_cursor_to_buffer(self, self->txtp);
}

/**
 * Handles the F3 key command (find).
 *
 * Prompts the user for a search string and performs the search. Stores
 * the search string for use with "search again" (Shift-F3).
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_cmd_search(Editor *self) {
    if (self->mode != kEditMode) return display_bell();

    ON_FAILURE_RETURN(editor_get_input(self, "Find (Use SHIFT-F3 to repeat): "));
    switch (*inpbuf) {
        case '\0':
        case ESC:
            return kOk;
        case SHIFT_FN(F3):
        case F3:
            break;
        default:
            strcpy(tknbuf, inpbuf);
            break;
    }
    return editor_cmd_search_again(self);
}

/**
 * Handles the F4 key command (enter mark mode).
 *
 * Enters mark mode for text selection, allowing the user to select text
 * for cut/copy/delete operations. Redraws the screen when exiting mark mode.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_cmd_mark(Editor *self) {
    CHECK_PARAM(self->mode == kEditMode);
    self->mode = kMarkMode;
    self->mark = self->txtp;
    return kOk;
}

/**
 * Handles the F5 key command (paste from clipboard).
 *
 * Pastes the contents of the clipboard at the current cursor position by
 * placing clipboard contents in the keyboard buffer for processing.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_paste(Editor *self) {
    CHECK_PARAM(self->mode == kEditMode);

    if (*self->clipboard_buf == '\0') return mmresult_ex(kEditorError, EMSG_CLIPBOARD_EMPTY);

    // Copy clipboard buffer into typeahead buffer
    int i;
    for (i = 0; self->clipboard_buf[i]; i++) {
        self->key_buf[i + 1] = self->clipboard_buf[i];
    }
    self->key_buf[i + 1] = '\0';

    return kOk;
}

/**
 * Handles regular printable character input.
 *
 * Inserts or overwrites the character depending on the current editing mode and
 * cursor position. The behavior varies based on context:
 * - In insert mode: Always inserts the character
 * - In overwrite mode: Overwrites the character unless at a newline or end of
 *   buffer
 * - At newline or end of buffer: Always inserts regardless of mode
 *
 * Non-printable characters and line length limits are handled by the underlying
 * insert/overwrite functions, which will display appropriate error messages.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
 MmResult editor_cmd_char(Editor *self) {
    if (self->mode != kEditMode) return display_bell();

    const char ch = self->key_buf[0];
    if (!editor_is_printable(ch)) return kOk;
    const bool overwrite = !self->insert && *self->txtp != '\n' && *self->txtp != '\0';
    if (overwrite) ON_FAILURE_RETURN(editor_delete_char(self));
    return editor_insert_char(self, ch);
}

/**
 * Handles the F9 key command (redraw screen).
 *
 * Repositions the display cursor to match the text cursor and forces a complete
 * redraw of the editor screen. Use to refresh the display if it becomes
 * corrupted or after terminal resize events.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_cmd_redraw(Editor *self) {
    // ON_FAILURE_RETURN(editor_sync_cursor_to_buffer(self, self->txtp));
    ON_FAILURE_RETURN(editor_print_lines(self, self->py, self->py + self->height - 1));
    ON_FAILURE_RETURN(editor_print_func_keys(self));
    ON_FAILURE_RETURN(editor_print_status(self));
    return kOk;
}

/**
 * Dispatches editor commands to their handler functions.
 *
 * Routes keystroke commands to the appropriate handler based on the command
 * key pressed. This is the main command dispatcher for edit mode.
 *
 * @param  self  Pointer to the Editor instance.
 * @param  cmd   The command key to process.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_cmd_dispatch(Editor *self, char cmd) {
// clang-format off
    switch (cmd) {
        case '\n':     return editor_cmd_newline(self);
        case UP:       return editor_cmd_up(self);
        case DOWN:     return editor_cmd_down(self);
        case LEFT:     return editor_cmd_left(self);
        case RIGHT:    return editor_cmd_right(self);
        case BKSP:     return editor_cmd_backspace(self);
        case DEL: {
            return self->mode == kMarkMode
                    ? editor_cmd_delete_selection(self)
                    : editor_cmd_delete(self);
        }
        case INSERT:   return editor_cmd_insert(self);
        case HOME:     return editor_cmd_home(self);
        case END:      return editor_cmd_end(self);
        case PUP:      return editor_cmd_page_up(self);
        case PDOWN:    return editor_cmd_page_down(self);
        case TAB:      return editor_cmd_tab(self);
        case ESC: {
            return self->mode == kMarkMode
                    ? editor_cmd_exit_mark(self)
                    : editor_cmd_exit(self);
        }
        case F1:       return editor_cmd_save_and_exit(self);
        case F2:       return editor_cmd_save_and_run(self);
        case F3:       return editor_cmd_search(self);
        case SHIFT_FN(F3): return editor_cmd_search_again(self);
        case F4: {
            return self->mode == kMarkMode
                    ? editor_cmd_cut(self)
                    : editor_cmd_mark(self);
        }
        case F5: {
            return self->mode == kMarkMode
                    ? editor_cmd_copy(self)
                    : editor_cmd_paste(self);
        }
        case F6:       return kOk;
        case F7:       return kOk;
        case F8:       return kOk;
        case F9:       return editor_cmd_redraw(self);
        case F10:      return kOk;
        case F11:      return kOk;
        case F12:      return kOk;
        default:       return editor_cmd_char(self);
    }
// clang-format on
}

/**
 * Loads the contents of a file into the editor buffer.
 *
 * Opens the specified file and reads its contents into self->buf, converting
 * CRLF line endings to LF. Counts the total number of lines in the file.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult editor_load_file(Editor *self) {
    self->num_lines = 0;
    char *p = self->buf;
    const int fnbr = streamio_find_free();
    ON_FAILURE_RETURN(streamio_open(self->fname, "rb", fnbr));
    if (!streamio_eof(fnbr)) {
        int ch;
        while ((ch = streamio_getc(fnbr)) != -1) {
            switch (ch) {
                case '\r':
                    continue;
                case '\n':
                    self->num_lines++;
                    break;
                default:
                    break;
            }

            // Handle overrun
            if (p >= self->buf + self->buf_sz - 1) {
                ON_FAILURE_LOG(streamio_close(fnbr));
                return kProgramTooLong;
            }

            *p++ = ch;
        }
    }
    self->num_lines++; // Ending the file ends the current line
    return streamio_close(fnbr);
}

/**
 * Resizes the TTY console to match graphical console dimensions.
 *
 * Ensures the console is at least as large as the editor needs (width and
 * height + 2 for status lines). Only resizes if necessary.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_resize_console(Editor *self) {
    int cw = 0;
    int ch = 0;
    bool resize = false;
    if (SUCCEEDED(console_get_size(&cw, &ch))) {
        if (cw < self->width) {
            cw = self->width;
            resize = true;
        }
        if (ch < self->height + 2) {
            ch = self->height + 2;
            resize = true;
        }
        if (resize) {
            (void) console_set_size(cw, ch);
        }
    }
    return kOk;
}

/**
 * Main keyboard handling loop for the editor.
 *
 * Continuously reads keystrokes from the console, processes them through
 * the command dispatcher, and updates the display. Continues until the
 * exit flag is set.
 *
 * @param  self  Pointer to the Editor instance.
 * @return       kOk on success, or an error code on failure.
 */
MmResult editor_edit_loop(Editor *self) {
    // Copy of the state when the display was last updated
    Editor old = editor_shallow_copy(self);

    while (self->mode != kExitMode) {
        ON_FAILURE_RETURN(editor_read_keys(self));
        MmResult result = editor_cmd_dispatch(self, self->key_buf[0]);
        switch (result) {
            case kOk:
                self->message[0] = '\0';
                break;
            case kEditorError:
                snprintf(self->message, sizeof(self->message), " %s ", mmresult_to_string(result));
                self->key_buf[1] = '\0'; // Ignore any more contents in typeahead buffer
                break;
            default:
                return result;
        }
        self->last_key = self->key_buf[0];

        // Unless moving up or down, update the preferred x-position
        if (self->key_buf[0] != UP && self->key_buf[0] != DOWN) {
            self->stored_cx = self->cx;
            self->stored_px = self->px;
        }

        // We only update the display once we have processed
        // the last key in the buffer
        if (self->key_buf[1] == '\0') {
            ON_FAILURE_RETURN(editor_update_display(self, &old));
            old = editor_shallow_copy(self);
        }
    }

    return kOk;
}

/**
 * Finds the start of line N in the buffer (fast, navigation only).
 *
 * Simple sequential search from buffer start. Use this for cursor
 * positioning, page navigation, and other operations that don't need
 * syntax highlighting state.
 *
 * @param  self  Pointer to the Editor instance.
 * @param  line  Line number (0-based).
 * @return       Pointer to first character of line N, or NULL if not found.
 */
char *editor_start_of_line_n(Editor *self, int line) {
    char *p = self->buf;
    for (int count = 0; count < line; ++count) {
        p = editor_next_line(self, p);
        if (!p) break;
    }
    return p;
}

/**
 * Finds the end of line N in the buffer.
 *
 * @param  self  Pointer to the Editor instance.
 * @param  line  Line number (0-based).
 * @return       Pointer to last character of line N (the character before the '\n' or '\0'),
 *               or NULL if not found.
 */
char *editor_end_of_line_n(Editor *self, int line) {
    return editor_end_of_line(self, editor_start_of_line_n(self, line));
}

MmResult editor_show_internal(Editor *self, int line) {
    ON_FAILURE_RETURN(editor_load_file(self));
    ON_FAILURE_RETURN(editor_resize_console(self));
    self->txtp = editor_start_of_line_n(self, line - 1);
    if (!self->txtp) {
        LOG_ERROR("cannot find line: %d", line - 1);
        self->txtp = self->buf;
    }

    // Determine initial y-viewport (py) and y-cursor (cy) positions
    self->cy = line - 1;
    while (self->cy > self->height) {
        self->py += self->height;
        self->cy -= self->height;
    }

    // Print the initial display
    ON_FAILURE_RETURN(editor_cmd_redraw(self));

    // Disable default break key handling, within the editor the break key
    // will be considered synonymous with ESC.
    SDL_AtomicSet(&mmb_options.break_key, 0);

    MmResult result = editor_edit_loop(self);

    // Tidy up.
    SDL_AtomicSet(&mmb_options.break_key, self->saved_break_key);

    return result;
}

/**
 * Main entry point for the PicoMite editor.
 *
 * Initializes the editor state, loads the specified file, sets up the
 * display, and enters the main keyboard handling loop. Cleans up and
 * restores terminal state on exit.
 *
 * @param  filename  Path to the file to edit.
 * @param  line      Line number to position cursor on (1-based).
 * @return           kOk on success, or an error code on failure.
 */
MmResult editor_show(const char *filename, int line) {
    // mmb_options.syntax_highlight = false;

    int width = -1, height = -1;
    ON_FAILURE_RETURN(display_get_size(false, &width, &height));
    if (width < 2 * SOFT_MARGIN) {
        return mmresult_ex(kError, "Terminal too narrow for EDITor");
    }

    Editor editor;
    ON_FAILURE_RETURN(editor_construct(&editor, filename, width, height));

    MmResult result = editor_show_internal(&editor, line);

    ON_FAILURE_LOG(editor_destruct(&editor));
    ON_FAILURE_LOG(display_reset());
    ON_FAILURE_LOG(display_cls());

    return result;
}
