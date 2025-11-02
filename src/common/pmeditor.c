/*-*****************************************************************************

MMBasic for Linux (MMB4L)

pmeditor.c

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
#include "error.h"
#include "file.h"
#include "fonttbl.h"
#include "keycodes.h"
#include "logger.h"
#include "memory.h"
#include "mmb4l.h"
#include "mmtime.h"
#include "pmeditor.h"
#include "pmeditor_private.h"
#include "program.h"
#include "prompt.h"
#include "streamio.h"
#include "../core/commandtbl.h"
#include "../core/MMBasic.h"
#include "../core/tokentbl.h"

#define OPTION_CONTINUATION     false
#define OPTION_COLOUR_CODE      true

typedef enum {
    kEditMode,
    kMarkMode,
} EditorMode;

typedef enum {
    kMarkBreak,
    kMarkContinue,
    kMarkEnd,
} MarkState;

static PmEditor *self = NULL;

/**
 * Sets the display cursor position and updates internal cursor coordinates.
 *
 * @param  x  X-coordinate in characters, starting at 0 (left edge).
 * @param  y  Y-coordinate in characters, starting at 0 (top edge).
 * @return    kOk on success, or an error code on failure.
 */
static MmResult pmeditor_set_cursor_pos(int x, int y) {
    ON_FAILURE_RETURN(display_set_cursor_pos(false, x, y));
    self->cx = x;
    self->cy = y;
    return kOk;
}

/**
 * Sets the syntax highlighting color for the current character.
 *
 * This function manages different highlight states (normal text, keywords,
 * comments, strings, numbers) based on the character being displayed.
 * Must be called sequentially from the start of each line to maintain
 * correct state tracking.
 *
 * IMPORTANT: Only call this function via the PmEditor#highlight_fn pointer so
 *            that unit-tests can override it.
 *
 * @param  highlight  The type of highlighting to apply.
 * @return            kOk on success, or kInternalFault for invalid highlight type.
 */
MmResult pmeditor_highlight(HighlightType highlight) {
    MmGraphicsColour argb = RGB_ANSI_WHITE;

    switch (highlight) {
        case kHighlightNormal:
            argb = RGB_ANSI_WHITE;
            break;
        case kHighlightComment:
            argb = RGB_ANSI_YELLOW;
            break;
        case kHighlightKeyword:
            argb = RGB_ANSI_CYAN;
            break;
        case kHighlightQuote:
            argb = RGB_ANSI_MAGENTA;
            break;
        case kHighlightNumber:
            argb = RGB_ANSI_GREEN;
            break;
        case kHighlightLine:
            argb = RGB_ANSI_MAGENTA;
            break;
        case kHighlightStatus:
            argb = RGB_ANSI_WHITE;
            break;
        case kHighlightError:
            argb = RGB_ANSI_WHITE;
            break;
        default:
            return kInternalFault;
    }

    return display_colour_fg(argb);
}

/**
 * Finds the longest line length in the text buffer.
 *
 * Scans through the entire text buffer to determine which line has the
 * most characters. Handles line continuation characters if OPTION_CONTINUATION
 * is enabled.
 *
 * @param  text    Pointer to the text buffer to scan.
 * @param  linein  Output parameter that receives the line number (0-based) of
 *                 the longest line.
 * @return         The length in characters of the longest line.
 */
static int pmeditor_find_longest_line_length(const char *text, int *linein) {
    int current_length = 0;
    int max_length = 0;
    const char *ptr = text;
    int line = 0;
    while (*ptr) {
        if (*ptr == '\n') {
            line++;
            if (ptr > text && *(ptr - 1) == '_' && *(ptr - 2) == ' ' && OPTION_CONTINUATION) {
                // Line continuation, do not reset length
            } else {
                // If this line exceeds the max, update
                if (current_length > max_length) {
                    max_length = current_length;
                    *linein = line;
                }
                current_length = 0;  // Reset for a new line
            }
        } else {
            // Increase length for this segment of the line
            current_length++;
        }

        ptr++;
    }

    // Final check in case the last line was the longest
    if (current_length > max_length) {
        max_length = current_length;
    }

    return max_length;
}

/**
 * Saves the editor buffer contents to a file.
 *
 * Creates a .bak backup if the file already exists.
 * Converts LF line endings to CRLF during the save process.
 *
 * @param  filename  Path to the file to save.
 * @return           kOk on success, kFilenameTooLong if backup name too long,
 *                   or other error codes from file operations.
 */
static MmResult pmeditor_save_file(const char *filename) {
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
            streamio_putc(fnbr_bak, streamio_getc(fnbr));
        }
        (void) streamio_close(fnbr);
        (void) streamio_close(fnbr_bak);
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
 * Positions the display cursor to match a position in the text buffer.
 *
 * Calculates the line and column for the given text pointer and moves
 * the cursor accordingly. Does nothing if the line is not currently visible
 * on screen.
 *
 * @param  curp  Pointer to a position in the text buffer.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult pmeditor_position_cursor(char *curp) {
    int line = 0;
    int column = 0;

    for (char *p = self->buf; p < curp; p++) {
        if (*p == '\n') {
            line++;
            column = 0;
        } else {
            column++;
        }
    }

    // Is the line on the page being displayed ?
    if (line < self->py || line >= self->py + self->height) return kOk;

    return pmeditor_set_cursor_pos(column, line - self->py);
}

/**
 * Draws a horizontal line across the full editor width.
 *
 * Uses Unicode non-breaking spaces with underline formatting to create
 * a visual separator line. Uses non-breaking spaces for better terminal
 * compatibility (e.g., Alacritty).
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_draw_line() {
    ON_FAILURE_RETURN(self->highlight_fn(kHighlightLine));
    ON_FAILURE_RETURN(display_underline(true));

    char buf[STRINGSIZE];

    // Use Unicode non-breaking spaces (U+00A0) for better terminal compatibility
    // with underline rendering, e.g. Alacritty does not render underlines for normal spaces.
    const char *nbsp = "\u00A0";  // Non-breaking space in UTF-8
    const int nbsp_len = 2;       // UTF-8 encoding of U+00A0 is 2 bytes

    // Fill buffer with non-breaking spaces
    int pos = 0;
    for (int i = 0; i < self->width && pos < STRINGSIZE - nbsp_len; i++) {
        memcpy(buf + pos, nbsp, nbsp_len);
        pos += nbsp_len;
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
 * @param  mode  The current editor mode.
 * @return       kOk on success, or an error code on failure.
 */
static MmResult pmeditor_print_func_keys(EditorMode mode) {
    const char *p;

    if (mode == kEditMode) {
        if (self->width >= 78) {
            p = "ESC:Exit  F1:Save  F2:Run  F3:Find  F4:Mark  F5:Paste";
        } else if (self->width >= 62) {
            p = "F1:Save F2:Run F3:Find F4:Mark F5:Paste";
        } else {
            p = "EDIT MODE";
        }
    } else {
        if (self->width >= 49) {
            p = "MARK MODE   ESC=Exit  DEL:Delete  F4:Cut  F5:Copy";
        } else {
            p = "MARK MODE";
        }
    }

    const int old_x = self->cx;
    const int old_y = self->cy;
    ON_FAILURE_RETURN(pmeditor_set_cursor_pos(0, self->height));
    ON_FAILURE_RETURN(pmeditor_draw_line());
    ON_FAILURE_RETURN(self->highlight_fn(kHighlightStatus));
    ON_FAILURE_RETURN(display_puts(p));
    ON_FAILURE_RETURN(self->highlight_fn(kHighlightNormal));
    ON_FAILURE_RETURN(display_clear_to_end_of_line());
    ON_FAILURE_RETURN(pmeditor_set_cursor_pos(old_x, old_y));

    return kOk;
}

/**
 * Gets input from the user with a prompt displayed in the status area.
 *
 * Displays a prompt and reads user input character by character, handling
 * backspace and storing the result in the global inpbuf. Supports early
 * exit with ESC or F3 keys.
 *
 * @param  prompt  The prompt string to display to the user.
 * @return         kOk on success, or an error code on failure.
 */
static MmResult pmeditor_get_input(const char *prompt) {
    ON_FAILURE_RETURN(pmeditor_set_cursor_pos(0, self->height + 1));
    ON_FAILURE_RETURN(display_puts(prompt));
    ON_FAILURE_RETURN(display_clear_to_end_of_line());
    ON_FAILURE_RETURN(pmeditor_set_cursor_pos(strlen(prompt), self->height + 1));

    // TODO: Ctrl-C should exit from this.
    // TODO: Prevent buffer overrun, deal with input too long for display.

    char *p = inpbuf;
    for (;; p++) {
        int ch = -1;
        ON_FAILURE_RETURN(prompt_getc(&ch));
        if (ch == '\r') break;
        *p = (char) ch;

        if (*p == SHIFT_FN(F3) || *p == F3 || *p == ESC) {
            p++;  // Include the key in the buffer
            break;
        }

        if (*p == '\b') {
            if (p > inpbuf) {           // Check we're not at start
                p--;                    // Remove previous character
                ON_FAILURE_RETURN(display_puts("\b \b"));  // Erase on screen
            }
            p--;  // Compensate for loop increment
            continue;
        }

        if (isprint(*p)) {
            ON_FAILURE_RETURN(display_putc(*p));
        } else {
            p--;  // Don't store non-printable chars
        }
    }
    *p = 0;  // terminate the input string

    ON_FAILURE_RETURN(pmeditor_print_func_keys(kEditMode));
    ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));

    return kOk;
}

/**
 * Displays a message in the status line area.
 *
 * Shows a message with inverse video (error highlighting) in the status area,
 * typically used for error messages or warnings. Sets a flag to redraw the
 * status line after the next user input.
 *
 * @param  msg  The message string to display.
 * @return      kOk on success, or an error code on failure.
 */
static MmResult pmeditor_display_msg(const char *msg) {
    ON_FAILURE_RETURN(pmeditor_set_cursor_pos(0, self->height + 1));
    ON_FAILURE_RETURN(self->highlight_fn(kHighlightError));
    ON_FAILURE_RETURN(display_inverse(true));
    ON_FAILURE_RETURN(display_puts(msg));
    ON_FAILURE_RETURN(self->highlight_fn(kHighlightNormal));
    ON_FAILURE_RETURN(display_reset());
    ON_FAILURE_RETURN(display_clear_to_end_of_line());
    ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));
    self->draw_status_line = true;
    return kOk;
}

/**
 * Inserts a character into the text buffer at the current position.
 *
 * Shifts all text after the current position down by one character to make
 * room for the new character. Checks for available buffer space before
 * inserting.
 *
 * @param  c  The character to insert.
 * @return    true if successful, false if buffer is full.
 */
static bool pmeditor_insert_char(char c/*, char *multi*/) {
    LOG_DEBUG("entered: c=%c", c);
    char *p;

    for (p = self->buf; *p; p++);  // find the end of the text in memory
    if (p >= self->buf + sizeof(self->buf) - 10) {  // and check that we have the space (allow 10 bytes for slack)
        pmeditor_display_msg(" OUT OF MEMORY ");
        return false;
    }
    for (; p >= self->txtp; p--) *(p + 1) = *p;  // shift everything down
    //*multi = 0;
    p = self->txtp - 1;
    //if ((c == '/' && *p == '*') || (c == '*' && *p == '/')) *multi = 1;
    p += 2;
    //if ((c == '/' && *p == '*') || (c == '*' && *p == '/')) *multi = 1;
    *self->txtp++ = c;  // and insert our char
    return true;
}

/**
 * Prints the current line and column position in the status area.
 *
 * Displays the current cursor position (1-based line and column numbers)
 * and the current insert/overwrite mode (INS/OVR).
 */
static MmResult pmeditor_print_status(void) {
    char s[64];
    snprintf(s, 64, "Ln: %d  Col: %d       ",
             self->py + self->cy + 1,
             self->px + self->cx + 1);
    strcpy(s + 19, self->insert ? "INS" : "OVR");

    ON_FAILURE_RETURN(pmeditor_set_cursor_pos(self->width - 25, self->height + 1));
    self->highlight_fn(kHighlightStatus);
    ON_FAILURE_RETURN(display_puts(s));
    self->highlight_fn(kHighlightNormal);
    ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));
    return kOk;
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
static bool pmeditor_edit_comp_str(char *p, const char *tkn) {
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
 * Sets the syntax highlighting color for the current character being displayed.
 *
 * Maintains state using static variables to track whether currently inside
 * quotes, comments, keywords, or numbers. Must be fed all characters from
 * the start of the line sequentially to maintain correct state.
 *
 * Pass NULL to reset the color state to normal at the start of a new line.
 *
 * @param  self  Pointer to the PmEditor instance.
 * @param  p     Pointer to the current character, or NULL to reset state.
 * @return       kOk on success, or an error code on failure.
 */
MmResult pmeditor_set_colour(PmEditor *self, char *p) {
    int i;
    char **pp;
    static bool intext = false;
    static bool incomment = false;
    static bool inkeyword = false;
    static char *twokeyword = NULL;
    static bool inquote = false;
    static bool innumber = false;
    static bool just_exited_comment = false;

    if (!OPTION_COLOUR_CODE) return kOk;

    // this is a list of keywords that can come after the OPTION and GUI commands
    // the list must be terminated with a NULL
    const char *twokeywordtbl[] = {
        "BASE",    "EXPLICIT", "DEFAULT",    "BREAK",   "AUTORUN", "BAUDRATE", "DISPLAY",
#if defined(GUICONTROLS)
        "BUTTON",  "SWITCH",   "CHECKBOX",   "RADIO",   "LED",     "FRAME",    "NUMBERBOX",
        "SPINBOX", "TEXTBOX",  "DISPLAYBOX", "CAPTION", "DELETE",  "DISABLE",  "HIDE",
        "ENABLE",  "SHOW",     "FCOLOUR",    "BCOLOUR", "REDRAW",  "BEEP",     "INTERRUPT",
#endif
        NULL};

    // this is a list of common keywords that should be highlighted as such
    // the list must be terminated with a NULL
    const char *specialkeywords[] = {
        "SELECT", "INTEGER", "FLOAT", "STRING", "DISPLAY", "SDCARD", "OUTPUT", "APPEND", "WRITE",
        "SLAVE", "TARGET", "PROGRAM",
        NULL};

    // cmdfile everything back to normal
    if (p == NULL) {
        innumber = inquote = inkeyword = incomment = intext = just_exited_comment = false;
        twokeyword = NULL;
        if (self->comment_level == 0) {
            ON_FAILURE_RETURN(self->highlight_fn(kHighlightNormal));
        }
        return kOk;
    }

    // Check for the start of a multiline comment
    if (*p == '/' && p[1] == '*' && !inquote) {
        if (self->comment_level == 0) {
            ON_FAILURE_RETURN(self->highlight_fn(kHighlightComment));
        }
        self->comment_level++;
        return kOk;
    }

    // Check for the end of a multiline comment
    // Watch for the edge case /*/ sequence which does not end a comment
    if (p >= self->buf && *p == '/' && *(p - 1) == '*' && *(p - 2) != '/' && !inquote) {
        if (self->comment_level > 0) {
            self->comment_level--;
            if (self->comment_level == 0) {
                just_exited_comment = true;
            }
        }
        return kOk;
    }

    // Within a multiline comment all chars are comments
    if (self->comment_level > 0) {
        // Don't change highlight if already in comment
        return kOk;
    }

    // check for a comment char
    if (*p == '\'' && !inquote) {
        incomment = true;
        return self->highlight_fn(kHighlightComment);
    }
    if (*p == '/' && p[1] == '*' && !inquote) {
        char *q = p;
        if (*(--q) == (char)'\n') {
            ON_FAILURE_RETURN(self->highlight_fn(kHighlightComment));
            self->comment_level = true;
        }
        return kOk;
    }

    // once in a comment all following chars must be comments also
    if (incomment || self->comment_level) return kOk;

    // check for a quoted string
    if (*p == '\"') {
        if (!inquote) {
            inquote = true;
            return self->highlight_fn(kHighlightQuote);
        } else {
            inquote = false;
            return kOk;
        }
    }

    if (inquote) return kOk;

    // if we are displaying a keyword check that it is still actually in the keyword and cmdfile if
    // not
    if (inkeyword) {
        if (isnamechar(*p) || *p == '$') return kOk;
        inkeyword = false;
        return self->highlight_fn(kHighlightNormal);
    }

    // if we are displaying a number check that we are still actually in it and cmdfile if not
    // this is complicated because numbers can be in hex or scientific notation
    if (innumber) {
        if (!isdigit(*p) && !(toupper(*p) >= 'A' && toupper(*p) <= 'F') && toupper(*p) != 'O' &&
            toupper(*p) != 'H' && *p != '.') {
            innumber = false;
            return self->highlight_fn(kHighlightNormal);
        } else {
            return kOk;
        }
        // check if we are starting a number
    } else if (!intext) {
        if (isdigit(*p) || *p == '&' || ((*p == '-' || *p == '+' || *p == '.') && isdigit(p[1]))) {
            innumber = true;
            return self->highlight_fn(kHighlightNumber);
        }
        // check if this is an 8 digit hex number as used in CFunctions
        for (i = 0; i < 8; i++)
            if (!isxdigit(p[i])) break;
        if (i == 8 && (p[8] == ' ' || p[8] == '\'' || p[8] == 0)) {
            innumber = true;
            return self->highlight_fn(kHighlightNumber);
        }
    }

    // check if this is the start of a keyword
    if (isnamechar(*p) && !intext) {
        for (i = 0; i < commandtbl_size - 1; i++) {  // check the command table for a match
            if (pmeditor_edit_comp_str((char *)p, (char *)commandtbl[i].name) != 0 ||
                ((pmeditor_edit_comp_str((char *)&p[1], (char *)&commandtbl[i].name[1]) != 0) && *p == '.' &&
                 *commandtbl[i].name == '_')) {
                if (pmeditor_edit_comp_str((char *)p, "REM") != 0) {  // special case, REM is a comment
                    ON_FAILURE_RETURN(self->highlight_fn(kHighlightComment));
                    incomment = true;
                } else {
                    ON_FAILURE_RETURN(self->highlight_fn(kHighlightKeyword));
                    inkeyword = true;
                    if (pmeditor_edit_comp_str((char *)p, "GUI") || pmeditor_edit_comp_str((char *)p, "OPTION")) {
                        twokeyword = p;
                        while (isalnum(*twokeyword)) twokeyword++;
                        while (*twokeyword == ' ') twokeyword++;
                    }
                    return kOk;
                }
            }
        }
        for (i = 0; i < tokentbl_size - 1; i++) {  // check the token table for a match
            if (pmeditor_edit_comp_str((char *)p, (char *)tokentbl[i].name) != 0) {
                inkeyword = true;
                return self->highlight_fn(kHighlightKeyword);
            }
        }

        // check for the second keyword in two keyword commands
        if (p == twokeyword) {
            for (pp = (char **)twokeywordtbl; *pp; pp++)
                if (pmeditor_edit_comp_str((char *)p, (char *)*pp)) break;
            if (*pp) {
                inkeyword = true;
                return self->highlight_fn(kHighlightKeyword);
            }
        }
        if (p >= twokeyword) twokeyword = NULL;

        // check for a range of common keywords
        for (pp = (char **)specialkeywords; *pp; pp++)
            if (pmeditor_edit_comp_str((char *)p, (char *)*pp)) break;
        if (*pp) {
            inkeyword = true;
            return self->highlight_fn(kHighlightKeyword);
        }
    }

    // try to keep track of if we are in general text or not
    // this is to avoid recognising keywords or numbers inside variables
    if (isnamechar(*p)) {
        if (just_exited_comment) {
            just_exited_comment = false;
            ON_FAILURE_RETURN(self->highlight_fn(kHighlightNormal));
        }
        intext = true;
    } else {
        intext = false;
        ON_FAILURE_RETURN(self->highlight_fn(kHighlightNormal));
    }

    return kOk;
}

/**
 * Finds the start of a specific line in the text buffer.
 *
 * Scans through the buffer counting newlines until reaching the specified
 * line number. Also tracks multi-line comment state for syntax highlighting.
 *
 * @param  self  Pointer to the PmEditor instance.
 * @param  line  The line number to find (0-based).
 * @return       Pointer to the start of the line, or NULL if line < 0 or self is NULL.
 */
char *pmeditor_find_line(PmEditor *self, int line/*, int *comment_level*/) {
    if (!self || line < 0) return NULL;

    const int NORMAL = 0;
    const int IN_QUOTE = 1;
    const int IN_SL_COMMENT = 2;
    int state = NORMAL;
    self->comment_level = 0;
    char *p = self->buf;

    // TODO: Handle CMM2 #COMMENT {START|END} construct
    // TODO: Handle REM statement

    while (line && *p) {
        switch (*p) {
            case '\n':
                line--;
                state = NORMAL;
                break;
            case '/':
                if (state == NORMAL && p[1] == '*') {
                    // Entered a multiline comment
                    self->comment_level++;
                    p++;
                }
                break;
            case '*':
                if (state == NORMAL && p[1] == '/') {
                    // Exited a multiline comment
                    if (self->comment_level > 0) self->comment_level--;
                    p++;
                }
                break;
            case '\"':
                if (state == NORMAL) {
                    state = IN_QUOTE;
                } else if (state == IN_QUOTE) {
                    state = NORMAL;
                }
                break;
            case '\'':
                if (state == NORMAL) {
                    state = IN_SL_COMMENT;
                }
                break;
            default:
                break;
        }
        p++;
    }

    return p;
}

/**
 * Prints a single line from the text buffer to the display.
 *
 * Renders the specified line with appropriate syntax highlighting if enabled.
 * If the line is beyond the end of the text, just clears to end of line.
 *
 * @param  line  The line number to print (0-based, relative to start of buffer).
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_print_line(int line) {
    LOG_DEBUG("entered: line=%d", line);
    int i;
    // int comment_level = -1;

    char *p = pmeditor_find_line(self, line/*, &self->comment_level*/);
    LOG_DEBUG("comment_level=%d", self->comment_level);
    if (OPTION_COLOUR_CODE) {
        // if we are colour coding we need to redraw the whole line
        ON_FAILURE_RETURN(display_putc_noflush('\r'));  // display the chars after the editing point
        // i = self->width - 1;         // I think this is wrong. Does not show last character in line
        // G.A.
        i = self->width;
        if (self->comment_level > 0) {
            ON_FAILURE_RETURN(self->highlight_fn(kHighlightComment));
        }
    } else {
        // if we are NOT colour coding we can start drawing at the current cursor position
        i = self->cx;
        while (i-- && *p && *p != '\n') p++;  // find the editing point in the buffer
        i = self->width - self->cx;
    }

    // Display the line from here to the end of the line or the screen width
    while (i && *p && *p != '\n') {
        if (OPTION_COLOUR_CODE) {
            ON_FAILURE_RETURN(pmeditor_set_colour(self, p));
        }
        ON_FAILURE_RETURN(display_putc_noflush(*p++));
        i--;
    }

    ON_FAILURE_RETURN(display_clear_to_end_of_line());
    ON_FAILURE_RETURN(pmeditor_set_colour(self, NULL));
    self->cx = self->width - 1;

    return kOk;
}

/**
 * Redraws the entire editor screen.
 *
 * Prints all visible lines starting from the top-left corner specified by
 * self->px and self->py.
 * 
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_print_screen(void) {
    ON_FAILURE_RETURN(pmeditor_set_cursor_pos(0, 0));
    for (int i = 0; i < self->height; i++) {
        ON_FAILURE_RETURN(pmeditor_print_line(i + self->py));
        ON_FAILURE_RETURN(display_puts("\r\n"));
        self->cx = 0;
        self->cy = i + 1;
    }

    // Consume any keystrokes accumulated while redrawing the screen
    while (console_getc() != -1) {}

    return kOk;
}

/**
 * Scrolls the editor display up by one line.
 *
 * Moves the viewport up (showing newer content at bottom), increments the
 * page offset, and redraws the newly visible bottom line.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_scroll_up(void) {
    ON_FAILURE_RETURN(pmeditor_set_cursor_pos(0, self->height));  // Move to end of the editing area
    ON_FAILURE_RETURN(display_clear_to_end_of_screen());
    ON_FAILURE_RETURN(display_scroll_up());
    self->py++;
    ON_FAILURE_RETURN(pmeditor_set_cursor_pos(0, self->height - 1));
    ON_FAILURE_RETURN(pmeditor_print_line(self->height - 1 + self->py));
    ON_FAILURE_RETURN(pmeditor_print_func_keys(kEditMode));
    ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));

    // Consume any keystrokes accumulated while redrawing the screen
    while (console_getc() != -1) {}

    return kOk;
}

/**
 * Scrolls the editor display down by one line.
 *
 * Moves the viewport down (showing older content at top), decrements the
 * page offset, and redraws the newly visible top line.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_scroll_down(void) {
    ON_FAILURE_RETURN(pmeditor_set_cursor_pos(0, self->height));  // Move to end of the editing area
    ON_FAILURE_RETURN(display_clear_to_end_of_screen());
    ON_FAILURE_RETURN(display_scroll_down());
    self->py--;
    ON_FAILURE_RETURN(pmeditor_set_cursor_pos(0, 0));
    ON_FAILURE_RETURN(pmeditor_print_line(self->py));
    ON_FAILURE_RETURN(pmeditor_print_func_keys(kEditMode));
    ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));

    // Consume any keystrokes accumulated while redrawing the screen
    while (console_getc() != -1) {}

    return kOk;
}

/**
 * Deletes the marked text from the buffer.
 *
 * Removes all text between self->mark and self->txtp, updating the line
 * count and setting the text changed flag.
 *
 * @return  kMarkEnd to exit mark mode after deletion.
 */
static MarkState pmeditor_mark_delete(void) {
    char *p;
    if (self->mark < self->txtp) {
        p = self->txtp;
        self->txtp = self->mark;
        self->mark = p;  // swap txtp and mark
    }
    for (p = self->txtp; p < self->mark; p++) {
        if (*p == '\n') self->num_lines--;
    }
    for (p = self->txtp; *self->mark;) *p++ = *self->mark++;
    *p++ = 0;
    *p++ = 0;
    self->text_changed = true;
    pmeditor_position_cursor(self->txtp);
    return kMarkEnd;
}

/**
 * Copies or cuts the marked text to the clipboard.
 *
 * Copies the text between self->mark and self->txtp to the clipboard buffer.
 * If cut is true, also deletes the marked text after copying.
 *
 * @param  cut  If true, delete the marked text after copying (cut operation).
 *              If false, leave the text in place (copy operation).
 * @return      kMarkEnd to exit mark mode, kMarkContinue if buffer too large.
 */
static MarkState pmeditor_mark_copy_or_cut(bool cut) {
    if (self->txtp - self->mark > MAXCLIP || self->mark - self->txtp > MAXCLIP) {
        pmeditor_display_msg(" MARKED TEXT EXCEEDS CLIPBOARD BUFFER SIZE");
        return kMarkContinue;
    }

    char *p;
    int cb_index = 0;
    if (self->mark <= self->txtp) {
        p = self->mark;
        while (p < self->txtp) self->clipboard[cb_index++] = *p++;
    } else {
        p = self->txtp;
        while (p <= self->mark - 1) self->clipboard[cb_index++] = *p++;
    }
    self->clipboard[cb_index] = '\0';

    if (cut) {
        return pmeditor_mark_delete();
    } else {
        pmeditor_position_cursor(self->txtp);
        return kMarkEnd;
    }
}

/**
 * Copies the marked text to the clipboard without deleting it.
 *
 * @return  kMarkEnd to exit mark mode, kMarkContinue if buffer too large.
 */
static MarkState pmeditor_mark_copy(void) {
    return pmeditor_mark_copy_or_cut(false);
}

/**
 * Cuts the marked text to the clipboard (copy and delete).
 *
 * @return  Result from the delete operation (kMarkEnd or kMarkContinue).
 */
static MarkState pmeditor_mark_cut(void) {
    return pmeditor_mark_copy_or_cut(true);
}

/**
 * Moves the mark down one line in mark mode.
 *
 * Attempts to maintain the same column position on the new line, or moves
 * to the end of the line if it's shorter.
 *
 * @return  kMarkBreak to update display,
 *          kMarkContinue if at end of file or line too long.
 */
static MarkState pmeditor_mark_down(void) {
    if (self->cy == self->height - 1) return kMarkContinue;
    char *p;
    int i;
    for (p = self->mark, i = self->cx; *p != 0 && *p != '\n';
         p++, i++);  // move to the end of this line
    if (*p == 0)
        return kMarkContinue;  // skip if it is at the end of the file
                   // if(i >= self->width) {
    if (i > self->width) {
        pmeditor_display_msg(" LINE IS TOO LONG ");
        return kMarkContinue;
    }
    self->mark = p + 1;  // step over the line terminator to the start of the next line
    for (i = 0; i < self->px + self->cx && *self->mark != '\0' && *self->mark != '\n';
         i++, self->mark++);  // move the cursor to the column
    self->cx = i;
    self->cy++;
    return kMarkBreak;
}

/**
 * Moves the mark to the end of the current line.
 *
 * @return  kMarkBreak to update display,
 *          kMarkContinue if already at end or line too long.
 */
static MarkState pmeditor_mark_end(void) {
    if (*self->mark == '\0') return kMarkContinue;
    char *p;
    int i;
    // Move to the end of the line
    for (p = self->mark, i = self->cx; *p != '\0' && *p != '\n'; p++, i++);

    if (i > self->width) {
        pmeditor_display_msg(" LINE IS TOO LONG ");
        return kMarkContinue;
    }

    self->mark = p;
    return kMarkBreak;
}

/**
 * Handles ESC key in mark mode.
 *
 * Waits briefly to distinguish between a plain ESC and escape sequences
 * (like mouse events) which it ignores.
 *
 * @return  kMarkEnd to exit mark mode, otherwise kMarkContinue.
 */
static MarkState pmeditor_mark_escape(void) {
    // Wait 50ms to see if anything more is coming.
    mmtime_sleep_ns(MILLISECONDS_TO_NANOSECONDS(50));
    if (console_getc() == '[' && console_getc() == 'M') {
        // TODO
        // Received escape code for Tera Term reporting a mouse click.
        // In mark mode we ignore it
        (void) console_getc();
        (void) console_getc();
        (void) console_getc();
        return kMarkContinue;
    }

    return kMarkEnd;
}

/**
 * Moves the mark to the start of the current line in mark mode.
 *
 * @return  kMarkBreak to update display,
 *          kMarkContinue if already at start of buffer.
 */
static MarkState pmeditor_mark_home(void) {
    if (self->mark == self->buf) return kMarkContinue;

    // Step back over the terminator if we are right at the end of the line
    if (*self->mark == '\n') {
        self->mark--;
    }

    // Move to the beginning of the line
    while (self->mark != self->buf && *self->mark != '\n') {
        self->mark--;
    }

    // Skip if no more lines above this one
    // TODO: understand this
    if (*self->mark == '\n') {
        self->mark++;
    }

    return kMarkBreak;
}

/**
 * Moves the mark left by one character in mark mode.
 *
 * @return  kMarkBreak to update display,
 *          kMarkContinue if at start of line, end of buffer, or line width limit reached.
 */
static MarkState pmeditor_mark_left(void) {
    if (self->cx >= self->width || *self->mark == '\0' || *self->mark == '\n') return kMarkContinue;
    self->mark--;
    self->cx--;
    return kMarkBreak;
}

/**
 * Moves the mark right by one character in mark mode.
 *
 * @return  kMarkBreak to update display,
 *          kMarkContinue if at end of line, end of buffer, or line width limit reached.
 */
static MarkState pmeditor_mark_right(void) {
    if (self->cx >= self->width || *self->mark == '\0' || *self->mark == '\n') return kMarkContinue;
    self->mark++;
    self->cx++;
    return kMarkBreak;
}

/**
 * Moves the mark up one line in mark mode.
 *
 * Attempts to maintain the same column position on the new line, or moves
 * to the end of the line if it's shorter.
 *
 * @return  kMarkBreak to update display,
 *          kMarkContinue if at top of screen or line too long.
 */
static MarkState pmeditor_mark_up(void) {
    if (self->cy <= 0) return kMarkContinue;
    char *p = self->mark;
    int i;
    if (*p == '\n') p--;  // step back over the terminator if we are right at the end of the line
    while (p != self->buf && *p != '\n') p--;  // move to the beginning of the line
    if (p != self->buf) {
        p--;  // step over the terminator to the end of the previous line
        for (i = 0; p != self->buf && *p != '\n'; p--, i++);  // move to the beginning of that line
        if (*p == '\n') p++;                                  // and position at the start
        // if (i >= self->width) {
        if (i > self->width) {
            pmeditor_display_msg(" LINE IS TOO LONG ");
            return kMarkContinue;
        }
    }
    self->mark = p;
    for (i = 0; i < self->px + self->cx && *self->mark != '\0' && *self->mark != '\n';
         i++, self->mark++);  // move the cursor to the column
    self->cx = i;
    self->cy--;
    return kMarkBreak;
}

/**
 * Converts a keystroke to its canonical form for command processing.
 *
 * Maps various Ctrl key combinations and special keys to their standard
 * command equivalents (e.g., Ctrl-E to UP, Ctrl-X to DOWN).
 *
 * @param  key  The raw keystroke from the keyboard.
 * @return      The canonical command key.
 */
static char pmeditor_canonical_key(char key) {
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
 * Dispatches mark mode commands to their handler functions.
 *
 * Routes mark mode keystrokes to the appropriate handler based on the
 * command key pressed.
 *
 * @param  cmd  The command key to process.
 * @return      MarkState indicating whether to continue, break, or end mark mode.
 */
static MarkState pmeditor_mark_dispatch(char cmd) {
// clang-format off
    switch (cmd) {
        case ESC:   return pmeditor_mark_escape();
        case UP:    return pmeditor_mark_up();
        case DOWN:  return pmeditor_mark_down();
        case LEFT:  return pmeditor_mark_left();
        case RIGHT: return pmeditor_mark_right();
        case HOME:  return pmeditor_mark_home();
        case END:   return pmeditor_mark_end();
        case F4:    return pmeditor_mark_cut();
        case F5:    return pmeditor_mark_copy();
        case DEL:   return pmeditor_mark_delete();
        default:    return kMarkContinue;
    }
// clang-format on
}

/**
 * Implements mark mode for text selection.
 *
 * Enters a sub-loop handling mark mode commands, displaying selected text
 * with inverse video, and processing cut/copy/delete operations.
 */
static MmResult pmeditor_mark_loop() {
    char *p, *oldmark;
    int x, y, oldx, oldy, txtpx, txtpy;
    ON_FAILURE_RETURN(pmeditor_print_func_keys(kMarkMode));
    self->mark = self->txtp;
    oldmark = self->mark;
    txtpx = oldx = self->cx;
    txtpy = oldy = self->cy;

    while (true) {
        int c;
        do {
            ON_FAILURE_RETURN(display_show_cursor(true));
            c = console_getc();
        } while (c == -1);
        ON_FAILURE_RETURN(display_show_cursor(false));

        self->keys[0] = pmeditor_canonical_key(c);
        self->keys[1] = '\0';

        if (self->draw_status_line) {
            ON_FAILURE_RETURN(pmeditor_print_func_keys(kMarkMode));
            ON_FAILURE_RETURN(pmeditor_print_status());
            self->draw_status_line = false;
        }

        MarkState mark_result = pmeditor_mark_dispatch(self->keys[0]);

        switch (mark_result) {
            case kMarkContinue:
                continue;
            case kMarkEnd:
                self->cx = txtpx;
                self->cy = txtpy;
                return kOk;
            default:
                break;
        }

        x = self->cx;
        y = self->cy;
        self->mark_mode = true;
        // first unmark the area not marked as a result of the keystroke
        if (oldmark < self->mark) {
            ON_FAILURE_RETURN(pmeditor_position_cursor(oldmark));
            p = oldmark;
            while (p < self->mark) {
                if (*p == '\n') {
                    ON_FAILURE_RETURN(display_putc_noflush('\r'));
                }
                ON_FAILURE_RETURN(display_putc_noflush(*p++));
            }
        } else if (oldmark > self->mark) {
            ON_FAILURE_RETURN(pmeditor_position_cursor(self->mark));
            p = self->mark;
            while (oldmark > p) {
                if (*p == '\n') {
                    ON_FAILURE_RETURN(display_putc_noflush('\r'));
                }
                ON_FAILURE_RETURN(display_putc_noflush(*p++));
            }
        }
        ON_FAILURE_RETURN(display_flush());
        oldmark = self->mark;
        oldx = x;
        oldy = y;

        // now draw the marked area
        if (self->mark < self->txtp) {
            ON_FAILURE_RETURN(pmeditor_position_cursor(self->mark));
            ON_FAILURE_RETURN(display_inverse(true));
            p = self->mark;
            while (p < self->txtp) {
                if (*p == '\n') {
                    ON_FAILURE_RETURN(display_putc_noflush('\r'));
                }
                ON_FAILURE_RETURN(display_putc_noflush(*p++));
            }
        } else if (self->mark > self->txtp) {
            ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));
            ON_FAILURE_RETURN(display_inverse(true));
            p = self->txtp;
            while (p < self->mark) {
                if (*p == '\n') {
                    ON_FAILURE_RETURN(display_putc_noflush('\r'));
                }
                ON_FAILURE_RETURN(display_putc_noflush(*p++));
            }
        }
        self->mark_mode = false;
        ON_FAILURE_RETURN(display_reset());

        oldx = x;
        oldy = y;
        oldmark = self->mark;
        ON_FAILURE_RETURN(pmeditor_position_cursor(self->mark));
    }
}

/**
 * Handles the newline/Enter key command.
 *
 * Inserts a newline character and implements auto-indentation by copying
 * leading spaces from the previous line. Redraws the screen to show the
 * new line structure.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_newline(/*char *multi*/) {
    int i;
    char *tp;

    // first count the spaces at the beginning of the line
    if (self->txtp != self->buf &&
        (*self->txtp == '\n' ||
         *self->txtp == 0)) {  // we only do this if we are at the end of the line
        for (tp = self->txtp - 1, i = 0; *tp != '\n' && tp >= self->buf; tp--)
            if (*tp != ' ')
                i = 0;  // not a space
            else
                i++;  // potential space at the start
        if (tp == self->buf && *tp == ' ')
            i++;  // correct for a counting error at the start of the buffer
        if (self->keys[1] != 0)
            i = 0;  // do not insert spaces if buffer too small or has something in
                    // it
        else
            self->keys[i + 1] = 0;        // make sure that the end of the buffer is zeroed
        while (i) self->keys[i--] = ' ';  // now, place our spaces in the typeahead buffer
    }
    if (!pmeditor_insert_char('\n'/*, multi*/)) return kOk;  // insert the newline
    self->text_changed = true;
    self->num_lines++;
    if (!(self->cy < self->height - 1))  // if we are NOT at the bottom
        self->py++;                     // otherwise scroll
    ON_FAILURE_RETURN(pmeditor_print_screen());  // redraw everything

    return pmeditor_position_cursor(self->txtp);
}

/**
 * Handles the UP arrow key command.
 *
 * Moves the cursor up one line, attempting to maintain the same column
 * position. Scrolls the viewport if necessary.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_up() {
    // If in the top row of the first page then do nothing
    if (self->cy == 0 && self->py == 0) return kOk;

    // If at the end of the line then step back one character
    if (*self->txtp == '\n') self->txtp--;

    // Step back until reach last character of the previous line,
    // or the first character of the document
    while (self->txtp != self->buf && *self->txtp != '\n') self->txtp--;

    // Step back to the first character of the previous line,
    // or the first character of the document
    if (self->txtp != self->buf) {
        self->txtp--;
        while (self->txtp != self->buf && *self->txtp != '\n') self->txtp--;
        if (*self->txtp == '\n') self->txtp++;
    }

    // Move to the same column as we were previously (self->tempx),
    // or the end of the line
    int i;
    for (i = 0; i < self->px + self->tempx && *self->txtp != 0 && *self->txtp != '\n';
         i++, self->txtp++);

    if (self->cy > 2 || self->py == 0) {
        // If we are more than two lines from the top then move the cursor up
        if (self->cy > 0) {
            ON_FAILURE_RETURN(pmeditor_set_cursor_pos(i, self->cy - 1));
        }
    } else if (self->py > 0) {
        // Otherwise scroll the document down
        self->cx = i;
        ON_FAILURE_RETURN(pmeditor_scroll_down());
    }

    return pmeditor_position_cursor(self->txtp);
}

/**
 * Handles the DOWN arrow key command.
 *
 * Moves the cursor down one line, attempting to maintain the same column
 * position. Scrolls the viewport if necessary.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_down() {
    // Find the end of the current line, or document
    char *p = self->txtp;
    while (*p != 0 && *p != '\n') p++;

    // If the current line is the last line of the document then do nothing
    if (*p == 0) return kOk;

    // Find the start of the next line
    p++;

    // Move to the same column as we were previously (self->tempx),
    // or the end of the line
    int i;
    for (i = 0; i < self->px + self->tempx && *p != 0 && *p != '\n'; i++, p++);
    self->txtp = p;

    if (self->cy < self->height - 3 || self->py + self->height == self->num_lines) {
        // If we are less than two lines from the bottom then move the cursor down
        if (self->cy < self->height - 1) pmeditor_set_cursor_pos(i, self->cy + 1);
    } else if (self->py + self->height < self->num_lines) {
        // Otherwise scroll the document up
        self->cx = i;
        ON_FAILURE_RETURN(pmeditor_scroll_up());
    }

    return pmeditor_position_cursor(self->txtp);
}

/**
 * Handles the LEFT arrow key command.
 *
 * Moves the cursor left one character. If at the start of a line, wraps
 * to the end of the previous line.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_left() {
    // If at the beginning of the document then do nothing
    if (self->txtp == self->buf) {
        return kOk;
    }

    // If at the beginning of a line then move to the end of the previous line
    if (*(self->txtp - 1) == '\n') {
        self->keys[1] = UP;
        self->keys[2] = END;
        self->keys[3] = '\0';
        return kOk;
    }

    // Move cursor back one character
    self->txtp--;

    return pmeditor_position_cursor(self->txtp);
}

/**
 * Handles the RIGHT arrow key command.
 *
 * Moves the cursor right one character. If at the end of a line, wraps
 * to the start of the next line. Prevents moving past the line width limit.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_right() {
    // If we are at the end of the document then do nothing
    if (*self->txtp == '\0') {
        return kOk;
    }

    // If at the end of a line then move to the beginning of the next line
    if (*self->txtp == '\n') {
        self->keys[1] = HOME;
        self->keys[2] = DOWN;
        self->keys[3] = '\0';
        return kOk;
    }

    if (self->cx >= self->width) {
        return pmeditor_display_msg((char *)" LINE IS TOO LONG ");
    }

    // Move cursor forward one character
    self->txtp++;

    return pmeditor_position_cursor(self->txtp);
}

/**
 * Handles the DELETE key command.
 *
 * Deletes the character at the current cursor position. If deleting a
 * newline, redraws the entire screen. Handles multi-line comment markers
 * specially to trigger screen refresh for syntax highlighting.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_delete() {
    if (*self->txtp == 0) return kOk;

    char *p = self->txtp;
    char c = *p;
    char currdel = *p;
    char nextdel = 0;
    char lastdel = 0;

    if (p != self->buf + sizeof(self->buf) - 1) {
        nextdel = p[1];
    } else {
        nextdel = 0;
    }

    if (p != self->buf) {
        lastdel = *(--p);
        p++;
    } else {
        lastdel = 0;
    }

    while (*p) {
        p[0] = p[1];
        p++;
    }

    if (c == '\n') {
        ON_FAILURE_RETURN(pmeditor_print_screen());
        self->num_lines--;
    } else {
        ON_FAILURE_RETURN(pmeditor_print_line(self->py + self->cy));
    }

    self->text_changed = true;
    ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));
    if (OPTION_COLOUR_CODE) {
        if ((currdel == '/' && nextdel == '*') ||
            (currdel == '*' && nextdel == '/') ||
            (currdel == '/' && lastdel == '*') ||
            (currdel == '*' && lastdel == '/')) {
            ON_FAILURE_RETURN(pmeditor_print_screen());
        }
    }

    return kOk;
}

/**
 * Handles the BACKSPACE key command.
 *
 * Deletes the character before the cursor. Implements smart tab deletion:
 * if multiple spaces precede the cursor at tab boundaries, deletes back
 * to the previous tab stop. Wraps to the end of the previous line if at
 * the start of a line.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_backspace() {
    if (self->txtp == self->buf) return kOk;

    if (*(self->txtp - 1) == '\n') {  // if at the beginning of the line wrap around
        self->keys[1] = UP;
        self->keys[2] = END;
        self->keys[3] = DEL;
        self->keys[4] = 0;
        return kOk;
    }

    char *p;

    // find how many spaces are between the cursor and the start of the line
    for (p = self->txtp - 1; *p == ' ' && p != self->buf; p--);
    if ((p == self->buf || *p == '\n') && self->txtp - p > 1) {
        int i = self->txtp - p - 1;
        // we have have the number of continuous spaces between the cursor and the
        // start of the line now figure out the number of backspaces to the nearest
        // tab stop

        i = (i % mmb_options.tab);
        if (i == 0) i = mmb_options.tab;
        // load the corresponding number of deletes in the type ahead buffer
        self->keys[i + 1] = 0;
        while (i--) {
            self->keys[i + 1] = DEL;
            self->txtp--;
        }
        // and let the delete case take care of deleting the characters
        return pmeditor_position_cursor(self->txtp);
    }
    // this is just a normal backspace (not a tabbed backspace)
    self->txtp--;
    ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));

    return pmeditor_cmd_delete();
}

/**
 * Handles the INSERT key command.
 *
 * Toggles between insert mode and overwrite mode.
 *
 * @return  kOk (always succeeds).
 */
static MmResult pmeditor_cmd_insert() {
    self->insert = !self->insert;
    return kOk;
}

/**
 * Handles the HOME key command.
 *
 * First press: moves to the start of the current line.
 * Second consecutive press: moves to the start of the file and redraws screen.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_home() {
    // If we are at the start of the document then do nothing
    if (self->txtp == self->buf) return kOk;

    // If this is the second time HOME has been pressed in succession then jump
    // to the start of the file
    if (self->last_key == HOME) {
        self->cy = 0;
        self->cy = 0;
        self->px = 0;
        self->py = 0;
        self->txtp = self->buf;
        ON_FAILURE_RETURN(pmeditor_print_screen());
        ON_FAILURE_RETURN(pmeditor_print_func_keys(kEditMode));
        ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));
        return kOk;
    }

    // If this is the end of the line then step back one character
    if (*self->txtp == '\n') self->txtp--;

    // Move to the beginning of the line
    while (self->txtp != self->buf && *self->txtp != '\n') self->txtp--;
    if (*self->txtp == '\n') self->txtp++;

    return pmeditor_position_cursor(self->txtp);
}

/**
 * Handles the END key command.
 *
 * First press: moves to the end of the current line.
 * Second consecutive press: moves to the end of the file and redraws screen.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_end() {
    // If we are at the end of the document then do nothing
    if (*self->txtp == 0) return kOk;

    // If this is the second time HOME has been pressed in succession then jump
    // to the end of the file
    if (self->last_key == END) {
        // Count lines (i) and find the start of the last line (p)
        int i = 0;
        char *p = self->txtp = self->buf;
        while (*self->txtp != 0) {
            if (*self->txtp == '\n') {
                p = self->txtp + 1;
                i++;
            }
            self->txtp++;
        }

        // Set the editor to show the last page of text,
        // and move the cursor to the last line
        if (i >= self->height) {
            self->py = i - self->height + 1;
            ON_FAILURE_RETURN(pmeditor_print_screen());
            self->cy = self->height - 1;
        } else {
            self->cy = i;
        }
        self->txtp = p;
        self->cx = 0;
    }

    // Move cursor to the end of the line
    while (self->cx < self->width && *self->txtp != 0 && *self->txtp != '\n') {
        self->txtp++;
        ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));
    }

    if (self->cx > self->width) {
        return pmeditor_display_msg(" LINE IS TOO LONG ");
    }

    return kOk;
}

/**
 * Handles the PAGE UP key command.
 *
 * Scrolls up by one screenful (or to the top of file if less than a
 * screenful remains). If already at top, queues HOME HOME to go to start.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_page_up() {
    // If already showing the top of the text then move to start of text
    if (self->py == 0) {
        self->keys[1] = HOME;
        self->keys[2] = HOME;
        self->keys[3] = '\0';
        return kOk;
    }

    int num_lines = 0;
    if (self->py >= self->height - 1) {
        // Move up a full screenfull
        num_lines = self->height + 1;
        self->py -= self->height;
    } else {
        // Move up less than a full screenfull
        num_lines = self->py + 1;
        self->py = 0;
    }

    // Move up 'num_lines'
    while (num_lines--) {
        if (*self->txtp == '\n') self->txtp--;
        while (self->txtp != self->buf && *self->txtp != '\n') self->txtp--;
        if (self->txtp == self->buf) break;
    }

    // Move to start of the line
    if (self->txtp != self->buf) self->txtp++;

    // Move to the same column as we were previously, or the end of the line
    for (int i = 0; i < self->px + self->cx && *self->txtp != 0 && *self->txtp != '\n';
         i++, self->txtp++);

    ON_FAILURE_RETURN(pmeditor_print_screen());

    return pmeditor_position_cursor(self->txtp);
}

/**
 * Handles the PAGE DOWN key command.
 *
 * Scrolls down by one screenful (or to the bottom of file if less than a
 * screenful remains). If already at bottom, queues END END to go to end.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_page_down() {
    // If already showing the bottom of the text then move to end of text
    if (self->num_lines <= self->py + self->height + 1) {
        self->keys[1] = END;
        self->keys[2] = END;
        self->keys[3] = '\0';
        return kOk;
    }

    int num_lines = 0;
    if (self->num_lines - self->py - self->height >= self->height) {
        // Move down a full screenfull
        self->py += self->height;
        num_lines = self->height;
    } else {
        // Move down less than a screenfull
        num_lines = self->num_lines - self->height - self->py;
        self->py = self->num_lines - self->height;
    }

    // Compensate if we are right at the end of the line
    if (*self->txtp == '\n') num_lines--;

    while (num_lines--) {
        // Step over the terminator if we are at the end of the line
        if (*self->txtp == '\n') self->txtp++;
        // Move to the end of the line
        while (*self->txtp != 0 && *self->txtp != '\n') self->txtp++;
        if (*self->txtp == '\0') break;  // No more lines after this one
    }

    // Move to start of the line
    if (self->txtp != self->buf) self->txtp++;

    // Move to the same column as we were previously, or the end of the line
    for (int i = 0; i < self->px + self->cx && *self->txtp != 0 && *self->txtp != '\n';
         i++, self->txtp++);

    ON_FAILURE_RETURN(pmeditor_print_screen());

    return pmeditor_position_cursor(self->txtp);
}

/**
 * Handles the TAB key command.
 *
 * Inserts spaces to the next tab stop position based on mmb_options.tab
 * setting. Places the spaces in the keyboard buffer for processing.
 *
 * @return  kOk (always succeeds).
 */
static MmResult pmeditor_cmd_tab() {
    strcpy(self->keys, "        ");
    self->keys[mmb_options.tab - ((self->px + self->cx) % mmb_options.tab)] = '\0';
    return kOk;
}

/**
 * Handles the F1 key command (save and exit).
 *
 * Validates that no lines exceed 255 characters, saves the file if modified,
 * and exits the editor. Clears and resets the display.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_save_and_exit() {
    int line_num = 0;
    const int line_len = pmeditor_find_longest_line_length((char *)self->buf, &line_num);
    if (line_len > 255) {
        char msg[32] = {};
        sprintf(msg, " LINE %d TOO LONG", line_len);
        return pmeditor_display_msg(msg);
    }

    // Clear and reset display
    ON_FAILURE_RETURN(display_cls());
    ON_FAILURE_RETURN(self->highlight_fn(kHighlightNormal));
    ON_FAILURE_RETURN(display_reset());

    // Save program
    if (self->text_changed && self->fname) {
        ON_FAILURE_RETURN(pmeditor_save_file(self->fname));
    }

    self->exit_flag = true;

    return kOk;
}

/**
 * Handles the F2 key command (save and run).
 *
 * Saves the file and prepares to run the program. Clears the runtime and
 * prepares the program for execution.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_save_and_run() {
    ON_FAILURE_RETURN(pmeditor_cmd_save_and_exit());
    if (!self->exit_flag) return kOk;

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
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_exit() {
#if 0
    // Wait 50ms to see if anything more is coming.
    mmtime_sleep_ns(MILLISECONDS_TO_NANOSECONDS(50));

    if (console_getc() == '[' && console_getc() == 'M') {
        // Received escape code for Tera Term reporting a mouse click or scroll
        // wheel movement
        int c, x, y;
        c = console_getc();
        x = console_getc() - '!';
        y = console_getc() - '!';
        if (c == 'e' || c == 'a') {  // Tera Term - SHIFT + mouse-wheel-rotate-down
            self->keys[1] = UP;
            self->keys[2] = '\0';
        } else if (c == 'd' || c == '`') {  // Tera Term - SHIFT + mouse-wheel-rotate-up
            self->keys[1] = DOWN;
            self->keys[2] = '\0';
        } else if (c == ' ' && x >= 0 && x < self->width && y >= 0 &&
                   y < self->height) {  // c == ' ' means mouse down and no shift, ctrl,
                                       // etc
            // first position on the y axis
            while (*self->txtp != 0 && y > self->cy)  // assume we have to move down the screen
                if (*self->txtp++ == '\n') self->cy++;
            while (self->txtp != self->buf && y < self->cy)  // assume we have to move up the screen
                if (*--self->txtp == '\n') self->cy--;
            while (self->txtp != self->buf && *(self->txtp - 1) != '\n')
                self->txtp--;  // move to the beginning of the line
            for (self->cx = 0; self->cx < x && *self->txtp && *self->txtp != '\n'; self->cx++)
                self->txtp++;  // now position on the x axis
            pmeditor_position_cursor(self->txtp);
        }
        return kOk;
    }
#endif
    // This must be an ordinary escape (not part of an escape code)
    if (self->text_changed) {
        ON_FAILURE_RETURN(pmeditor_get_input("Exit and discard all changes (Y/N): "));
        if (toupper(*inpbuf) != 'Y') return kOk;
    }

    self->exit_flag = true;

    return kOk;
}

/**
 * Searches for the next occurrence of the search string.
 *
 * Continues searching from the current position, wrapping around to the
 * beginning if necessary. Updates the display to show the found text.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_search_again() {
    char *p = self->txtp;
    if (*p == 0) p = self->buf - 1;
    int i = strlen((char *)tknbuf);
    while (1) {
        p++;
        if (p == self->txtp) break;
        if (*p == 0) p = self->buf;
        if (p == self->txtp) break;
        if (memcmp(p, tknbuf, i) == 0) break;
    }
    if (p == self->txtp) {
        return pmeditor_display_msg(" NOT FOUND ");
    }
    int y;
    for (y = 0, self->txtp = self->buf; self->txtp != p;
         self->txtp++) {  // find the line and column of the string
        if (*self->txtp == '\n') {
            y++;  // y is the line
        }
    }
    self->py = y - self->height / 2;  // self->py is the line displayed at the top
    if (self->py < 0) self->py = 0;   // compensate if we are near the start
    ON_FAILURE_RETURN(pmeditor_print_screen());

    return pmeditor_position_cursor(self->txtp);
}

/**
 * Handles the F3 key command (find).
 *
 * Prompts the user for a search string and performs the search. Stores
 * the search string for use with "search again" (Shift-F3).
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_search() {
    ON_FAILURE_RETURN(pmeditor_get_input("Find (Use SHIFT-F3 to repeat): "));
    if (*inpbuf == 0 || *inpbuf == ESC) return kOk;
    if (!(*inpbuf == SHIFT_FN(F3) || *inpbuf == F3)) strcpy(tknbuf, inpbuf);
    return pmeditor_cmd_search_again();
}

/**
 * Handles the F4 key command (enter mark mode).
 *
 * Enters mark mode for text selection, allowing the user to select text
 * for cut/copy/delete operations. Redraws the screen when exiting mark mode.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_mark() {
    ON_FAILURE_RETURN(pmeditor_mark_loop());
    ON_FAILURE_RETURN(pmeditor_print_screen());
    ON_FAILURE_RETURN(pmeditor_print_func_keys(kEditMode));
    ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));
    return kOk;
}

/**
 * Handles the F5 key command (paste from clipboard).
 *
 * Pastes the contents of the clipboard at the current cursor position by
 * placing clipboard contents in the keyboard buffer for processing.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_paste() {
    if (*self->clipboard == 0) {
        return pmeditor_display_msg(" CLIPBOARD IS EMPTY ");
    }
    int i;
    for (i = 0; self->clipboard[i]; i++) self->keys[i + 1] = self->clipboard[i];
    self->keys[i + 1] = 0;
    return kOk;
}

/**
 * Handles regular printable character input.
 *
 * Inserts or overwrites the character depending on insert mode. Ignores
 * non-printable characters. Enforces line length limit. Redraws the current
 * line after insertion.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_char(/*char *multi*/) {
    char c = self->keys[0];
    LOG_DEBUG("entered: c=%c", c);

    // Ignore non-printable characters
    if (c < ' ' || c > '~') return kOk;

    // Limit line length
    if (self->cx >= self->width) {
        return pmeditor_display_msg(" LINE IS TOO LONG ");
    }

    self->text_changed = true;
    if (self->insert || *self->txtp == '\n' || *self->txtp == 0) {
        // Insert character
        if (!pmeditor_insert_char(c/*, multi*/)) return kOk;
    } else {
        // Overwrite character
        *self->txtp++ = c;
    }

    // Redraw the edited line
    ON_FAILURE_RETURN(pmeditor_print_line(self->py + self->cy));

    // Update the display cursor
    ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));

    self->tempx = self->cy;  // used to track the preferred cursor position
    // if (multi && OPTION_COLOUR_CODE) pmeditor_print_screen();
    return kOk;
}

/**
 * Dispatches editor commands to their handler functions.
 *
 * Routes keystroke commands to the appropriate handler based on the command
 * key pressed. This is the main command dispatcher for edit mode.
 *
 * @param  cmd  The command key to process.
 * @return      kOk on success, or an error code on failure.
 */
static MmResult pmeditor_cmd_dispatch(char cmd/*, char *multi*/) {
// clang-format off
    switch (cmd) {
        case '\n':     return pmeditor_cmd_newline(/*multi*/);
        case UP:       return pmeditor_cmd_up();
        case DOWN:     return pmeditor_cmd_down();
        case LEFT:     return pmeditor_cmd_left();
        case RIGHT:    return pmeditor_cmd_right();
        case BKSP:     return pmeditor_cmd_backspace();
        case DEL:      return pmeditor_cmd_delete();
        case INSERT:   return pmeditor_cmd_insert();
        case HOME:     return pmeditor_cmd_home();
        case END:      return pmeditor_cmd_end();
        case PUP:      return pmeditor_cmd_page_up();
        case PDOWN:    return pmeditor_cmd_page_down();
        case TAB:      return pmeditor_cmd_tab();
        case ESC:      return pmeditor_cmd_exit();
        case F1:       return pmeditor_cmd_save_and_exit();
        case F2:       return pmeditor_cmd_save_and_run();
        case F3:       return pmeditor_cmd_search();
        case SHIFT_FN(F3): return pmeditor_cmd_search_again();
        case F4:       return pmeditor_cmd_mark();
        case F5:       return pmeditor_cmd_paste();
        case F6:       return kOk;
        case F7:       return kOk;
        case F8:       return kOk;
        case F9:       return kOk;
        case F10:      return kOk;
        case F11:      return kOk;
        case F12:      return kOk;
        default:       return pmeditor_cmd_char(/*multi*/);
    }
// clang-format on
}

/**
 * Loads the contents of a file into the editor buffer.
 *
 * Opens the specified file and reads its contents into self->buf, converting
 * CRLF line endings to LF. Counts the total number of lines in the file.
 *
 * @return  kOk on success, or an error code on failure.
 */
static MmResult pmeditor_load_file() {
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
            // TODO: Handle overrun.
            *p++ = ch;
        }
    }
    return streamio_close(fnbr);
}

/**
 * Resizes the TTY console to match graphical console dimensions.
 *
 * Ensures the console is at least as large as the editor needs (width and
 * height + 2 for status lines). Only resizes if necessary.
 *
 * @return  kOk on success, or an error code on failure.
 */
MmResult pmeditor_resize_console() {
    // if (!graphics_current) return kOk;
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
 * @return  kOk on success, or an error code on failure.
 */
MmResult pmeditor_edit_loop() {
    //char multi = false;

    while (true) {
        int c;
        do {
            ON_FAILURE_RETURN(display_show_cursor(true));
            c = console_getc();
        } while (c == -1);
        ON_FAILURE_RETURN(display_show_cursor(false));

        self->keys[0] = c;
        self->keys[1] = '\0';

        if (self->draw_status_line) {
            ON_FAILURE_RETURN(pmeditor_print_func_keys(kEditMode));
            ON_FAILURE_RETURN(pmeditor_print_status());
            self->draw_status_line = false;
        }

        do {
            char *old_txtp = self->txtp;

            self->keys[0] = pmeditor_canonical_key(self->keys[0]);
            // if (buf[0] == BreakKeySave)
            //     buf[0] = ESC;  // if the user tried to break turn it into an escape
            ON_FAILURE_RETURN(pmeditor_cmd_dispatch(self->keys[0]/*, &multi*/));

            if (self->exit_flag) return kOk;

            self->last_key = self->keys[0];
            if (self->keys[0] != UP && self->keys[0] != DOWN) {
                self->tempx = self->cx;
            }

            // Shuffle down the keyboard buffer to get the next character
            self->keys[MAXCLIP + 1] = '\0';
            for (int i = 0; i < MAXCLIP + 1; i++) {
                self->keys[i] = self->keys[i + 1];
            }

            if (self->txtp != old_txtp) {
                ON_FAILURE_RETURN(pmeditor_print_status());
            }
        } while (*self->keys);
    }
}

/**
 * Initializes a PmEditor instance with the specified parameters.
 *
 * Zeros out the editor structure and sets initial values for all fields.
 * The editor height is reduced by 2 rows to accommodate the status line
 * and function key display area. Sets insert mode as the default editing
 * mode and saves the current break key setting for later restoration.
 *
 * @param  self      Pointer to the PmEditor structure to initialize.
 * @param  filename  Path to the file being edited (not copied, pointer stored).
 * @param  width     Width of the editor display area in characters.
 * @param  height    Total height available in characters (status area included).
 * @return           kOk (always succeeds).
 */
MmResult pmeditor_init(PmEditor *self, const char *filename, int width, int height) {
    memset(self, 0, sizeof(PmEditor));
    self->height = height - 2; // 2 rows for the status line
    self->width = width;
    self->fname = filename;
    self->insert = true;
    self->text_changed = false;
    self->saved_break_key = mmb_options.break_key;
    self->comment_level = 0;
    self->highlight_fn = pmeditor_highlight;
    return kOk;
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
MmResult pmeditor_show(const char *filename, int line) {
    line = 10;
    if (self) ClearSpecificTempMemory(self);
    self = GetTempMemory(sizeof(PmEditor));

    int width = -1, height = -1;
    ON_FAILURE_RETURN(display_get_size(false, &width, &height));

    ON_FAILURE_RETURN(pmeditor_init(self, filename, width, height));
    ON_FAILURE_RETURN(pmeditor_load_file());
    ON_FAILURE_RETURN(pmeditor_resize_console());

    self->txtp = pmeditor_find_line(self, line - 1/*, &self->comment_level*/);

    ON_FAILURE_RETURN(pmeditor_print_screen());
    ON_FAILURE_RETURN(pmeditor_print_func_keys(kEditMode));
    ON_FAILURE_RETURN(pmeditor_print_status());
    ON_FAILURE_RETURN(pmeditor_position_cursor(self->txtp));

    // Disable default break key handling, within the editor the break key
    // will be considered synonymous with ESC.
    mmb_options.break_key = 0;

    MmResult result = pmeditor_edit_loop();

    // Tidy up.
    mmb_options.break_key = self->saved_break_key;
    if (SUCCEEDED(result)) {
        ON_FAILURE_RETURN(display_reset());
        ON_FAILURE_RETURN(display_cls());
    }

    return result;
}
