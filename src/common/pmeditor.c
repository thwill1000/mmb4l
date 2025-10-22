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
#include "memory.h"
#include "mmb4l.h"
#include "mmgetchar.h"
#include "mmtime.h"
#include "pmeditor.h"
#include "program.h"
#include "streamio.h"
#include "../core/commandtbl.h"
#include "../core/MMBasic.h"
#include "../core/tokentbl.h"

#define OPTION_CONTINUATION     false
#define OPTION_COLOUR_CODE      true
#define MAXCLIP 1024

// const char *ANSI_UNDERLINE     = "\033[4m";
// const char *ANSI_RESET         = "\033[0m";
// const char *ANSI_CLEAR_TO_EOL  = "\033[K";
// const char *ANSI_REVERSE_VIDEO = "\033[7m";
// const char *argb_BLACK      = "\033[30m";
// const char *argb_RED        = "\033[31m";
// const char *argb_GREEN      = "\033[32m";
// const char *argb_YELLOW     = "\033[33m";
// const char *argb_BLUE       = "\033[34m";
// const char *argb_MAGENTA    = "\033[35m";
// const char *argb_CYAN       = "\033[36m";
// const char *argb_WHITE      = "\033[37m";

typedef enum {
    kEditMode,
    kMarkMode,
} EditorMode;

typedef enum {
    kHighlightNormal,
    kHighlightComment,
    kHighlightKeyword,
    kHighlightQuote,
    kHighlightNumber,
    kHighlightLine,
    kHighlightStatus,
    kHighlightError,
} HighlightType;

typedef struct {
    const char *fname;      // Name/path of file being edited
    char buf[EDIT_BUFFER_SIZE];  // Buffer used for editing the text
    int num_lines;          // Number of lines of text held in the buffer
    int width;              // Width of the editor screen in characters
    int height;             // Height of the editor screen in characters
    int px;                 // Column at top left hand corner of editor
    int py;                 // Row at top left hand corner of editor
    int cx;                 // Current cursor column (from 0)
    int cy;                 // Current cursor row (from 0)
    char *txtp;             // Position of the cursor in the text being edited
    bool draw_status_line;  // True if the status line needs redrawing on next keystroke
    bool insert;            // True if the editor is in INSERT mode
    int tempx;              // User to track preferred x-position when up/down arrowing
    bool text_changed;      // True if the etxt has been editor and thus may need saving
    int multiline_comment;  // True if we are inside a multi-line comment
    bool mark_mode;         // True if we are in mark mode
    char last_key;          // Last key pressed
    char clipboard[MAXCLIP + 2];  // Clipboard contents
    char keys[MAXCLIP + 2]; // Buffer of incoming keystrokes
    bool exit_flag;         // True if the editor should exit
    char saved_break_key;   // Original value of mmb_options.break_key when editor entered
} PmEditor;

static PmEditor *self = NULL;

/**
 * Sets cursor position.
 *
 * @param  x  x-coordinate, in characters, starting at 0 (left).
 * @param  y  y-coordinate, in characters, starting at 0 (top).
 */ 
static MmResult pmeditor_set_cursor(int x, int y) {
    ON_FAILURE_RETURN(display_set_cursor_pos(false, x, y));
    self->cx = x;
    self->cy = y;
    return kOk;
}

static MmResult pmeditor_highlight(HighlightType highlight) {
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

static int find_longest_line_length(const char *text, int *linein) {
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
        streamio_close(fnbr);
        streamio_close(fnbr_bak);
    }

    const int fnbr = streamio_find_free();
    ON_FAILURE_RETURN(streamio_open(filename, "wb", fnbr));

    // Copy contents of edit buffer to file
    // changing the LF line-endings to CRLF.
    for (const char *p = self->buf; *p; ++p) {
        if (*p == '\n') streamio_putc(fnbr, '\r');
        streamio_putc(fnbr, *p);
    }

    return streamio_close(fnbr);
}

/**
 * Positions the display cursor to match a given position in the text.
 */
static void pmeditor_position_cursor(char *curp) {
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
    if (line < self->py || line >= self->py + self->height) return;

    pmeditor_set_cursor(column, line - self->py);
}

/** Prints the function keys in the status bar. */
static void pmeditor_print_func_keys(EditorMode mode) {
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
    pmeditor_set_cursor(0, self->height);
    pmeditor_highlight(kHighlightLine);
    ON_FAILURE_ERROR(display_underline(true));
    {
        char buf[STRINGSIZE];
        memset(buf, ' ', self->width);
        buf[self->width] = '\0';
        display_puts(buf);
    }
    ON_FAILURE_ERROR(display_reset());
    display_puts("\r\n");
    pmeditor_highlight(kHighlightStatus);
    display_puts(p);
    pmeditor_highlight(kHighlightNormal);
    ON_FAILURE_ERROR(display_clear_to_end_of_line());
    pmeditor_set_cursor(old_x, old_y);
}

// get an input string from the user and save into inpbuf
static void pmeditor_get_input(const char *prompt) {
    int i;
    char *p;

    pmeditor_set_cursor(0, self->height + 1);
    display_puts(prompt);
//    MX470Cursor(0, (VRes / gui_font_height) * gui_font_height - gui_font_height);
//    MX470PutS((char *)prompt, gui_fcolour, gui_bcolour);
    for (i = 0; i < self->width - (int) strlen(prompt); i++) {
        display_putc(' ');
    }
    pmeditor_set_cursor(strlen(prompt), self->height + 1);
//    MX470Cursor(strlen((char *)prompt) * gui_font_width,
//                (VRes / gui_font_height) * gui_font_height - gui_font_height);
    for (p = inpbuf; (*p = MMgetchar()) != '\r'; p++) {  // get the input
        if (*p == 0xb3 || *p == F3 || *p == ESC) {
            p++;
            break;
        }  // return if it is SHIFT-F3, F3 or ESC
        if (isprint(*p)) {
            display_putc(*p);  // echo the char
        }
        if (*p == '\b') {
            p--;  // backspace over a backspace
            if (p >= inpbuf) {
                p--;                                           // and the char before
                display_puts("\b \b");                          // erase on the screen
//                MX470PutS("\b \b", gui_fcolour, gui_bcolour);  // erase on the MX470 display
            }
        }
    }
    *p = 0;  // terminate the input string
    pmeditor_print_func_keys(kEditMode);
    pmeditor_position_cursor(self->txtp);
}

/** Displays a message in the status line. */
static void pmeditor_display_msg(const char *msg) {
    pmeditor_set_cursor(0, self->height + 1);
    pmeditor_highlight(kHighlightError);
    ON_FAILURE_ERROR(display_inverse(true));
    display_puts(msg);
    // MX470PutS((char *)msg, BLACK, RED);
    pmeditor_highlight(kHighlightNormal);
    ON_FAILURE_ERROR(display_reset());
    ON_FAILURE_ERROR(display_clear_to_end_of_line());
    pmeditor_position_cursor(self->txtp);
    self->draw_status_line = true;
}

// move the text down by one char starting at the current position in the text
// and insert a character
static int pmeditor_insert_char(char c, char *multi) {
    char *p;

    for (p = self->buf; *p; p++);  // find the end of the text in memory
    if (p >= self->buf + sizeof(self->buf) - 10) {  // and check that we have the space (allow 10 bytes for slack)
        pmeditor_display_msg(" OUT OF MEMORY ");
        return false;
    }
    for (; p >= self->txtp; p--) *(p + 1) = *p;  // shift everything down
    *multi = 0;
    p = self->txtp - 1;
    if ((c == '/' && *p == '*') || (c == '*' && *p == '/')) *multi = 1;
    p += 2;
    if ((c == '/' && *p == '*') || (c == '*' && *p == '/')) *multi = 1;
    *self->txtp++ = c;  // and insert our char
    return true;
}

static void pmeditor_print_status(void) {
    char s[64];
    snprintf(s, 64, "Ln: %d  Col: %d       ",
             self->py + self->cy + 1,
             self->px + self->cx + 1);
    strcpy(s + 19, self->insert ? "INS" : "OVR");

    pmeditor_set_cursor(self->width - 25, self->height + 1);
    pmeditor_highlight(kHighlightStatus);
    display_puts(s);
    pmeditor_highlight(kHighlightNormal);
    pmeditor_position_cursor(self->txtp);
}

static int pmeditor_edit_comp_str(char *p, const char *tkn) {
    while (*tkn && (toupper(*tkn) == toupper(*p))) {
        if (*tkn == '(' && *p == '(') return true;
        if (*tkn == '$' && *p == '$') return true;
        tkn++;
        p++;
    }
    if (*tkn == 0 && !isnamechar(*p)) return true;  // return the string if successful

    return false;  // or NULL if not
}

// this function does the syntax colour coding
// p = pointer to the current character to be printed
//     or NULL if the colour coding is to be cmdfile to normal
//
// it keeps track of where it is in the line using static variables
// so it must be fed all chars from the start of the line
static void pmeditor_set_colour(char *p) {
    int i;
    char **pp;
    static int intext = false;
    static int incomment = false;
    static int inkeyword = false;
    static char *twokeyword = NULL;
    static int inquote = false;
    static int innumber = false;

    if (!OPTION_COLOUR_CODE) return;

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
        //        ".PROGRAM", ".END PROGRAM", ".SIDE", ".LABEL" , ".LINE",".WRAP", ".WRAP TARGET",
        NULL};

    // cmdfile everything back to normal
    if (p == NULL) {
        innumber = inquote = inkeyword = incomment = intext = false;
        twokeyword = NULL;
        if (!self->multiline_comment) {
            // gui_fcolour = GUI_C_NORMAL;
            pmeditor_highlight(kHighlightNormal);
        }
        return;
    }

    if (*p == '*' && p[1] == '/' && !inquote) {
        self->multiline_comment = 2;
        return;
    }

    if (*p == '/' && !inquote && self->multiline_comment == 2) {
        self->multiline_comment = false;
        return;
    }

    // check for a comment char
    if (*p == '\'' && !inquote) {
        pmeditor_highlight(kHighlightComment);
        incomment = true;
        return;
    }
    if (*p == '/' && p[1] == '*' && !inquote) {
        char *q = p;
        if (*(--q) == (char)'\n') {
            pmeditor_highlight(kHighlightComment);
            self->multiline_comment = true;
        }
        return;
    }

    // once in a comment all following chars must be comments also
    if (incomment || self->multiline_comment) return;

    // check for a quoted string
    if (*p == '\"') {
        if (!inquote) {
            inquote = true;
            pmeditor_highlight(kHighlightQuote);
            return;
        } else {
            inquote = false;
            return;
        }
    }

    if (inquote) return;

    // if we are displaying a keyword check that it is still actually in the keyword and cmdfile if
    // not
    if (inkeyword) {
        if (isnamechar(*p) || *p == '$') return;
        pmeditor_highlight(kHighlightNormal);
        inkeyword = false;
        return;
    }

    // if we are displaying a number check that we are still actually in it and cmdfile if not
    // this is complicated because numbers can be in hex or scientific notation
    if (innumber) {
        if (!isdigit(*p) && !(toupper(*p) >= 'A' && toupper(*p) <= 'F') && toupper(*p) != 'O' &&
            toupper(*p) != 'H' && *p != '.') {
            pmeditor_highlight(kHighlightNormal);
            innumber = false;
            return;
        } else {
            return;
        }
        // check if we are starting a number
    } else if (!intext) {
        if (isdigit(*p) || *p == '&' || ((*p == '-' || *p == '+' || *p == '.') && isdigit(p[1]))) {
            pmeditor_highlight(kHighlightNumber);
            innumber = true;
            return;
        }
        // check if this is an 8 digit hex number as used in CFunctions
        for (i = 0; i < 8; i++)
            if (!isxdigit(p[i])) break;
        if (i == 8 && (p[8] == ' ' || p[8] == '\'' || p[8] == 0)) {
            pmeditor_highlight(kHighlightNumber);
            innumber = true;
            return;
        }
    }

    // check if this is the start of a keyword
    if (isnamechar(*p) && !intext) {
        for (i = 0; i < commandtbl_size - 1; i++) {  // check the command table for a match
            if (pmeditor_edit_comp_str((char *)p, (char *)commandtbl[i].name) != 0 ||
                ((pmeditor_edit_comp_str((char *)&p[1], (char *)&commandtbl[i].name[1]) != 0) && *p == '.' &&
                 *commandtbl[i].name == '_')) {
                if (pmeditor_edit_comp_str((char *)p, "REM") != 0) {  // special case, REM is a comment
                    pmeditor_highlight(kHighlightComment);
                    incomment = true;
                } else {
                    pmeditor_highlight(kHighlightKeyword);
                    inkeyword = true;
                    if (pmeditor_edit_comp_str((char *)p, "GUI") || pmeditor_edit_comp_str((char *)p, "OPTION")) {
                        twokeyword = p;
                        while (isalnum(*twokeyword)) twokeyword++;
                        while (*twokeyword == ' ') twokeyword++;
                    }
                    return;
                }
            }
        }
        for (i = 0; i < tokentbl_size - 1; i++) {  // check the token table for a match
            if (pmeditor_edit_comp_str((char *)p, (char *)tokentbl[i].name) != 0) {
                pmeditor_highlight(kHighlightKeyword);
                inkeyword = true;
                return;
            }
        }

        // check for the second keyword in two keyword commands
        if (p == twokeyword) {
            for (pp = (char **)twokeywordtbl; *pp; pp++)
                if (pmeditor_edit_comp_str((char *)p, (char *)*pp)) break;
            if (*pp) {
                pmeditor_highlight(kHighlightKeyword);
                inkeyword = true;
                return;
            }
        }
        if (p >= twokeyword) twokeyword = NULL;

        // check for a range of common keywords
        for (pp = (char **)specialkeywords; *pp; pp++)
            if (pmeditor_edit_comp_str((char *)p, (char *)*pp)) break;
        if (*pp) {
            pmeditor_highlight(kHighlightKeyword);
            inkeyword = true;
            return;
        }
    }

    // try to keep track of if we are in general text or not
    // this is to avoid recognising keywords or numbers inside variables
    if (isnamechar(*p)) {
        intext = true;
    } else {
        intext = false;
        pmeditor_highlight(kHighlightNormal);
    }
}

/**
 * Finds the start of a given line in the text buffer.
 * 
 * @param line     The line number to find (0-based).
 * @param inmulti  Pointer to an integer that will be set to true if the line
 *                 starts inside a multi-line comment.
 */
static char *pmeditor_find_line(int line, int *inmulti) {
    *inmulti = false;
    char *p = self->buf;
    char *q = p;
    skipspace(q);
    if (q[0] == '/' && q[1] == '*') *inmulti = true;
    if (q[0] == '*' && q[1] == '/') *inmulti = false;
    while (line && *p) {
        if (*p == '\n') {
            if (*inmulti == 2) *inmulti = false;
            line--;
            q = &p[1];
            skipspace(q);
            if (q[0] == '/' && q[1] == '*') *inmulti = true;
            if (q[0] == '*' && q[1] == '/') *inmulti = 2;
        }
        p++;
    }
    return p;
}

// print a line starting at the current cursor column (self->px).
// if the line is beyond the end of the text then just clear to the end of line
// enters with the line number to be printed
static void pmeditor_print_line(int line) {
    int i;
    int inmulti = false;

    char *p = pmeditor_find_line(line, &inmulti);
    if (OPTION_COLOUR_CODE) {
        // if we are colour coding we need to redraw the whole line
        display_putc_noflush('\r');  // display the chars after the editing point
        // i = self->width - 1;         // I think this is wrong. Does not show last character in line
        // G.A.
        i = self->width;
    } else {
        // if we are NOT colour coding we can start drawing at the current cursor position
        i = self->cx;
        while (i-- && *p && *p != '\n') p++;  // find the editing point in the buffer
        i = self->width - self->cx;
    }

    while (i && *p && *p != '\n') {
        if (OPTION_COLOUR_CODE) {
            if (!inmulti) {
                pmeditor_set_colour((char *)p);
            } else {
                pmeditor_highlight(kHighlightComment);
            }
        }
        display_putc_noflush(*p++);  // display the chars after the editing point
        i--;
    }

    ON_FAILURE_ERROR(display_clear_to_end_of_line());
    pmeditor_set_colour(NULL);
    self->cx = self->width - 1;
}

// print a full screen starting with the top left corner specified by self->px, self->py
// this draws the full screen including blank areas so there is no need to clear the screen first
// it then returns the cursor to its original position
static void pmeditor_print_screen(void) {
    pmeditor_set_cursor(0, 0);
    for (int i = 0; i < self->height; i++) {
        pmeditor_print_line(i + self->py);
        display_puts("\r\n");
        self->cx = 0;
        self->cy = i + 1;
    }

    // Consume any keystrokes accumulated while redrawing the screen
    while (console_getc() != -1) {}
}

/** Scrolls editor up one line. */
static void pmeditor_scroll_up(void) {
    pmeditor_set_cursor(0, self->height);  // Move to end of the editing area
    ON_FAILURE_ERROR(display_clear_to_end_of_screen());
    ON_FAILURE_ERROR(display_scroll_up());
    // display_puts("\033[J");                // Clear to end of screen
    // display_puts("\033[1S");               // Scroll up one line
    self->py++;
    pmeditor_set_cursor(0, self->height - 1);
    pmeditor_print_line(self->height - 1 + self->py);
    pmeditor_print_func_keys(kEditMode);
    pmeditor_position_cursor(self->txtp);

    // Consume any keystrokes accumulated while redrawing the screen
    while (console_getc() != -1) {}
}

/** Scrolls editor down one line. */
static void pmeditor_scroll_down(void) {
    pmeditor_set_cursor(0, self->height);  // Move to end of the editing area
    ON_FAILURE_ERROR(display_clear_to_end_of_screen());
    ON_FAILURE_ERROR(display_scroll_down());
    // display_puts("\033[J");                // Clear to end of screen
    // display_puts("\033[1T");               // Scroll down one line
    self->py--;
    pmeditor_set_cursor(0, 0);
    pmeditor_print_line(self->py);
    pmeditor_print_func_keys(kEditMode);
    pmeditor_position_cursor(self->txtp);

    // Consume any keystrokes accumulated while redrawing the screen
    while (console_getc() != -1) {}
}

// mark mode
// implement the mark mode (when the user presses F4)
static void pmeditor_mark_mode() {
    char *p, *mark, *oldmark;
    int c = -1, x, y, i, oldx, oldy, txtpx, txtpy, errmsg = false;
    pmeditor_print_func_keys(kMarkMode);
    oldmark = mark = self->txtp;
    txtpx = oldx = self->cx;
    txtpy = oldy = self->cy;
    while (1) {
        c = -1;
            c = console_getc();
        if (c != -1 && errmsg) {
            pmeditor_print_func_keys(kMarkMode);
            errmsg = false;
        }
        switch (c) {
            case ESC:
                // Wait 50ms to see if anything more is coming.
                mmtime_sleep_ns(MILLISECONDS_TO_NANOSECONDS(50));
                if (console_getc() == '[' && console_getc() == 'M') {
                    // received escape code for Tera Term reporting a mouse click.  in mark mode we
                    // ignore it
                    console_getc();
                    console_getc();
                    console_getc();
                    break;
                }
                self->cx = txtpx;
                self->cy = txtpy;  // just an escape key
                return;

            case UP:
                if (self->cy <= 0) continue;
                p = mark;
                if (*p == '\n')
                    p--;  // step back over the terminator if we are right at the end of the line
                while (p != self->buf && *p != '\n') p--;  // move to the beginning of the line
                if (p != self->buf) {
                    p--;  // step over the terminator to the end of the previous line
                    for (i = 0; p != self->buf && *p != '\n';
                         p--, i++);       // move to the beginning of that line
                    if (*p == '\n') p++;  // and position at the start
                    // if(i >= self->width) {
                    if (i > self->width) {
                        pmeditor_display_msg(" LINE IS TOO LONG ");
                        errmsg = true;
                        continue;
                    }
                }
                mark = p;
                for (i = 0; i < self->px + self->cx && *mark != 0 && *mark != '\n';
                     i++, mark++);  // move the cursor to the column
                self->cx = i;
                self->cy--;
                break;

            case DOWN:
                if (self->cy == self->height - 1) continue;
                for (p = mark, i = self->cx; *p != 0 && *p != '\n';
                     p++, i++);         // move to the end of this line
                if (*p == 0) continue;  // skip if it is at the end of the file
                                        // if(i >= self->width) {
                if (i > self->width) {
                    pmeditor_display_msg(" LINE IS TOO LONG ");
                    errmsg = true;
                    continue;
                }
                mark = p + 1;  // step over the line terminator to the start of the next line
                for (i = 0; i < self->px + self->cx && *mark != 0 && *mark != '\n';
                     i++, mark++);  // move the cursor to the column
                self->cx = i;
                self->cy++;
                break;

            case LEFT:
                if (self->cx == self->px) continue;
                mark--;
                self->cx--;
                break;

            case RIGHT:
                if (self->cx >= self->width || *mark == 0 || *mark == '\n') continue;
                mark++;
                self->cx++;
                break;

            case HOME:
                if (mark == self->buf) break;
                if (*mark == '\n')
                    mark--;  // step back over the terminator if we are right at the end of the line
                while (mark != self->buf && *mark != '\n')
                    mark--;                 // move to the beginning of the line
                if (*mark == '\n') mark++;  // skip if no more lines above this one
                break;

            case END:
                if (*mark == 0) break;
                for (p = mark, i = self->cx; *p != 0 && *p != '\n';
                     p++, i++);  // move to the end of this line
                // if(i >= self->width) {
                if (i > self->width) {
                    pmeditor_display_msg(" LINE IS TOO LONG ");
                    errmsg = true;
                    continue;
                }
                mark = p;
                break;

            case F4:  // Cut
            case F5:  // Copy
                if (self->txtp - mark > MAXCLIP || mark - self->txtp > MAXCLIP) {
                    pmeditor_display_msg(" MARKED TEXT EXCEEDS CLIPBOARD BUFFER SIZE");
                    errmsg = true;
                    break;
                }
                int cb_index = 0;
                if (mark <= self->txtp) {
                    p = mark;
                    while (p < self->txtp) self->clipboard[cb_index++] = *p++;
                } else {
                    p = self->txtp;
                    while (p <= mark - 1) self->clipboard[cb_index++] = *p++;
                }
                self->clipboard[cb_index] = '\0';
                if (c == F5) {
                    pmeditor_position_cursor(self->txtp);
                    return;
                }
                // fall through

            case DEL:
                if (mark < self->txtp) {
                    p = self->txtp;
                    self->txtp = mark;
                    mark = p;  // swap txtp and mark
                }
                for (p = self->txtp; p < mark; p++)
                    if (*p == '\n') self->num_lines--;
                for (p = self->txtp; *mark;) *p++ = *mark++;
                *p++ = 0;
                *p++ = 0;
                self->text_changed = true;
                pmeditor_position_cursor(self->txtp);
                return;
            case 9999:
                break;
            default:
                continue;
        }

        x = self->cx;
        y = self->cy;
        self->mark_mode = true;
        // first unmark the area not marked as a result of the keystroke
        if (oldmark < mark) {
            pmeditor_position_cursor(oldmark);
            p = oldmark;
            while (p < mark) {
                if (*p == '\n') {
                    display_putc_noflush('\r');
                }
                display_putc_noflush(*p++);
            }
        } else if (oldmark > mark) {
            pmeditor_position_cursor(mark);
            p = mark;
            while (oldmark > p) {
                if (*p == '\n') {
                    display_putc_noflush('\r');
                }
                display_putc_noflush(*p++);
            }
        }
        display_flush();
        oldmark = mark;
        oldx = x;
        oldy = y;

        // now draw the marked area
        if (mark < self->txtp) {
            pmeditor_position_cursor(mark);
            ON_FAILURE_ERROR(display_inverse(true));
            p = mark;
            while (p < self->txtp) {
                if (*p == '\n') {
                    display_putc_noflush('\r');
                }
                display_putc_noflush(*p++);
            }
        } else if (mark > self->txtp) {
            pmeditor_position_cursor(self->txtp);
            ON_FAILURE_ERROR(display_inverse(true));
            p = self->txtp;
            while (p < mark) {
                if (*p == '\n') {
                    display_putc_noflush('\r');
                }
                display_putc_noflush(*p++);
            }
        }
        self->mark_mode = false;
        ON_FAILURE_ERROR(display_reset());

        oldx = x;
        oldy = y;
        oldmark = mark;
        pmeditor_position_cursor(mark);
    }
}

static MmResult pmeditor_cmd_newline(char *multi) {
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
    if (!pmeditor_insert_char('\n', multi)) return kOk;  // insert the newline
    self->text_changed = true;
    self->num_lines++;
    if (!(self->cy < self->height - 1))  // if we are NOT at the bottom
        self->py++;                     // otherwise scroll
    pmeditor_print_screen();           // redraw everything
    pmeditor_position_cursor(self->txtp);

    return kOk;
}

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
        if (self->cy > 0) pmeditor_set_cursor(i, self->cy - 1);
    } else if (self->py > 0) {
        // Otherwise scroll the document down
        self->cx = i;
        pmeditor_scroll_down();
    }

    pmeditor_position_cursor(self->txtp);

    return kOk;
}

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
        if (self->cy < self->height - 1) pmeditor_set_cursor(i, self->cy + 1);
    } else if (self->py + self->height < self->num_lines) {
        // Otherwise scroll the document up
        self->cx = i;
        pmeditor_scroll_up();
    }

    pmeditor_position_cursor(self->txtp);

    return kOk;
}

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
    pmeditor_position_cursor(self->txtp);

    return kOk;
}

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
        pmeditor_display_msg((char *)" LINE IS TOO LONG ");
        return kOk;
    }

    // Move cursor forward one character
    self->txtp++;
    pmeditor_position_cursor(self->txtp);

    return kOk;
}

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
        pmeditor_print_screen();
        self->num_lines--;
    } else {
        pmeditor_print_line(self->py + self->cy);
    }

    self->text_changed = true;
    pmeditor_position_cursor(self->txtp);
    if (currdel == '/' && nextdel == '*' && OPTION_COLOUR_CODE) pmeditor_print_screen();
    if (currdel == '*' && nextdel == '/' && OPTION_COLOUR_CODE) pmeditor_print_screen();
    if (currdel == '/' && lastdel == '*' && OPTION_COLOUR_CODE) pmeditor_print_screen();
    if (currdel == '*' && lastdel == '/' && OPTION_COLOUR_CODE) pmeditor_print_screen();

    return kOk;
}

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
        pmeditor_position_cursor(self->txtp);
        return kOk;
    }
    // this is just a normal backspace (not a tabbed backspace)
    self->txtp--;
    pmeditor_position_cursor(self->txtp);

    return pmeditor_cmd_delete();
}

static MmResult pmeditor_cmd_insert() {
    self->insert = !self->insert;
    return kOk;
}

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
        // display_cls();
        pmeditor_print_screen();
        pmeditor_print_func_keys(kEditMode);
        pmeditor_position_cursor(self->txtp);
        return kOk;
    }

    // If this is the end of the line then step back one character
    if (*self->txtp == '\n') self->txtp--;

    // Move to the beginning of the line
    while (self->txtp != self->buf && *self->txtp != '\n') self->txtp--;
    if (*self->txtp == '\n') self->txtp++;

    pmeditor_position_cursor(self->txtp);

    return kOk;
}

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
            pmeditor_print_screen();
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
        pmeditor_position_cursor(self->txtp);
    }

    if (self->cx > self->width) pmeditor_display_msg(" LINE IS TOO LONG ");

    return kOk;
}

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

    pmeditor_print_screen();
    pmeditor_position_cursor(self->txtp);

    return kOk;
}

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

    pmeditor_print_screen();
    pmeditor_position_cursor(self->txtp);

    return kOk;
}

static MmResult pmeditor_cmd_tab() {
    strcpy(self->keys, "        ");
    self->keys[mmb_options.tab - ((self->px + self->cx) % mmb_options.tab)] = '\0';
    return kOk;
}

static MmResult pmeditor_cmd_save_and_exit() {
    int line_num = 0;
    const int line_len = find_longest_line_length((char *)self->buf, &line_num);
    if (line_len > 255) {
        char msg[32] = {};
        sprintf(msg, " LINE %d TOO LONG", line_len);
        pmeditor_display_msg(msg);
        return kOk;
    }

    // Clear and reset display
    display_cls();
    pmeditor_highlight(kHighlightNormal);
    ON_FAILURE_RETURN(display_reset());

    // Save program
    if (self->text_changed && self->fname) {
        ON_FAILURE_RETURN(pmeditor_save_file(self->fname));
    }

    self->exit_flag = true;

    return kOk;
}

static MmResult pmeditor_cmd_save_and_run() {
    ON_FAILURE_RETURN(pmeditor_cmd_save_and_exit());
    if (!self->exit_flag) return kOk;

    ON_FAILURE_RETURN(ClearRuntime());
    ON_FAILURE_RETURN(PrepareProgram(true));
    if (*ProgMemory == T_NEWLINE) nextstmt = ProgMemory;
    return kOk;
}

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
        pmeditor_get_input((char *)"Exit and discard all changes (Y/N): ");
        if (toupper(*inpbuf) != 'Y') return kOk;
    }

    self->exit_flag = true;

    return kOk;
}

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
        pmeditor_display_msg(" NOT FOUND ");
        return kOk;
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
    pmeditor_print_screen();
    pmeditor_position_cursor(self->txtp);
    // pmeditor_set_cursor(x, y);
    return kOk;
}

static MmResult pmeditor_cmd_search() {
    pmeditor_get_input("Find (Use SHIFT-F3 to repeat): ");
    if (*inpbuf == 0 || *inpbuf == ESC) return kOk;
    if (!(*inpbuf == 0xb3 || *inpbuf == F3)) strcpy(tknbuf, inpbuf);
    return pmeditor_cmd_search_again();
}

static MmResult pmeditor_cmd_mark() {
    pmeditor_mark_mode(&self->keys[1]);
    pmeditor_print_screen();
    pmeditor_print_func_keys(kEditMode);
    pmeditor_position_cursor(self->txtp);
    return kOk;
}

static MmResult pmeditor_cmd_paste() {
    if (*self->clipboard == 0) {
        pmeditor_display_msg(" CLIPBOARD IS EMPTY ");
        return kOk;
    }
    int i;
    for (i = 0; self->clipboard[i]; i++) self->keys[i + 1] = self->clipboard[i];
    self->keys[i + 1] = 0;
    return kOk;
}

static MmResult pmeditor_cmd_char(char *multi) {
    char c = self->keys[0];

    // Ignore non-printable characters
    if (c < ' ' || c > '~') return kOk;

    // Limit line length
    if (self->cx >= self->width) {
        pmeditor_display_msg(" LINE IS TOO LONG ");
        return kOk;
    }

    self->text_changed = true;
    if (self->insert || *self->txtp == '\n' || *self->txtp == 0) {
        // Insert character
        if (!pmeditor_insert_char(c, multi)) return kOk;
    } else {
        // Overwrite character
        *self->txtp++ = c;
    }

    // Redraw the edited line
    pmeditor_print_line(self->py + self->cy);

    // Update the display cursor
    pmeditor_position_cursor(self->txtp);

    self->tempx = self->cy;  // used to track the preferred cursor position
    if (multi && OPTION_COLOUR_CODE) pmeditor_print_screen();
    return kOk;
}

static char pmeditor_canonical_key(char key) {
    if (key == self->saved_break_key) {
        return ESC;
    }

// clang-format off
    switch (key) {
        case '\r':         return '\n';
        case CTRLKEY('D'): return RIGHT;
        case CTRLKEY('E'): return UP;
        case CTRLKEY('G'): return SHIFT_F3;
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

static MmResult pmeditor_dispatch_cmd(char cmd, char *multi) {
// clang-format off
    switch (cmd) {
        case '\n':     return pmeditor_cmd_newline(multi);
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
        case SHIFT_F3: return pmeditor_cmd_search_again();
        case F4:       return pmeditor_cmd_mark();
        case F5:       return pmeditor_cmd_paste();
        case F6:       return kOk;
        case F7:       return kOk;
        case F8:       return kOk;
        case F9:       return kOk;
        case F10:      return kOk;
        case F11:      return kOk;
        case F12:      return kOk;
        default:       return pmeditor_cmd_char(multi);
    }
// clang-format on
}

/** Fills the editor buffer from the file. */
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

/** Resizes the TTY console to ensure it is at least as big as the graphical console. */
MmResult pmeditor_resize_console() {
    if (!graphics_current) return kOk;
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

/** Main keyboard handling loop. */
MmResult pmeditor_main_loop() {
    char multi = false;

    while (true) {
        int c;
        do {
            display_show_cursor(true);
            c = console_getc();
        } while (c == -1);
        display_show_cursor(false);

        self->keys[0] = c;
        self->keys[1] = '\0';

        if (self->draw_status_line) {
            pmeditor_print_func_keys(kEditMode);
            pmeditor_print_status();
            self->draw_status_line = false;
        }

        do {
            char *old_txtp = self->txtp;

            self->keys[0] = pmeditor_canonical_key(self->keys[0]);
            // if (buf[0] == BreakKeySave)
            //     buf[0] = ESC;  // if the user tried to break turn it into an escape
            ON_FAILURE_RETURN(pmeditor_dispatch_cmd(self->keys[0], &multi));

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

            if (self->txtp != old_txtp) pmeditor_print_status();
        } while (*self->keys);
    }
}

/**
 * Shows the "PicoMite" editor.
 *
 * @param  filename  File to edit.
 * @param  line      Line to place the cursor on.
 */
MmResult pmeditor_show(const char *filename, int line) {
    line = 10;
    if (self) ClearSpecificTempMemory(self);
    self = GetTempMemory(sizeof(PmEditor));

    int width = -1, height = -1;
    ON_FAILURE_RETURN(display_get_size(false, &width, &height));

    self->height = height - 2; // 2 rows for the status line
    self->width = width;
    self->fname = filename;
    self->insert = true;
    self->text_changed = false;
    self->saved_break_key = mmb_options.break_key;
    self->multiline_comment = 0;

    ON_FAILURE_RETURN(pmeditor_load_file());
    ON_FAILURE_RETURN(pmeditor_resize_console());

    self->txtp = pmeditor_find_line(line - 1, &self->multiline_comment);

    pmeditor_print_screen();
    pmeditor_print_func_keys(kEditMode);
    pmeditor_print_status();
    pmeditor_position_cursor(self->txtp);

    // Disable default break key handling, within the editor the break key
    // will be considered synonymous with ESC.
    mmb_options.break_key = 0;

    MmResult result = pmeditor_main_loop();

    // Tidy up.
    mmb_options.break_key = self->saved_break_key;
    if (SUCCEEDED(result)) {
        ON_FAILURE_RETURN(display_reset());
        ON_FAILURE_RETURN(display_cls());
    }

    return result;
}
