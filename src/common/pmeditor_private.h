/*-*****************************************************************************

MMBasic for Linux (MMB4L)

pmeditor_private.h

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

#if !defined(MMB4L_PMEDITOR_PRIVATE)
#define MMB4L_PMEDITOR_PRIVATE

#include <stdbool.h>
#include <stdint.h>

#include "mmresult.h"
#include "program.h" // for EDIT_BUFFER_SIZE

#define MAXCLIP 1024
#define REDRAW_NOTHING  -1
#define REDRAW_SCREEN   INT32_MAX

typedef enum {
    kHighlightUnspecified = 0,
    kHighlightNormal,
    kHighlightComment,
    kHighlightKeyword,
    kHighlightQuote,
    kHighlightNumber,
    kHighlightLine,
    kHighlightStatus,
    kHighlightError,
    kHighlightTrailingWhitespace,
    kHighlightMark,
} HighlightType;

typedef struct SyntaxState {
    int multiline_comment;  ///< Tracks multiline comment level
    bool incomment;         ///< In single line comment
    bool inquote;           ///< In double quotes
    bool inkeyword;         ///< In keyword
    bool innumber;          ///< In number
    bool intext;            ///< In text
    char *twokeyword;
} SyntaxState;

typedef struct {
    int py;      ///< Row at top left hand corner of editor
    int cx;      ///< Cursor column (from 0)
    int cy;      ///< Cursor row (from 0)
    char *txtp;  ///< Character position of the standard cursor
    char *mark;  ///< Character position of the mark cursor
} PmEditorPos;

typedef enum {
    kModeUnspecified = 0,
    kEditMode,
    kMarkMode,
} PmEditorMode;

typedef struct {
    const char *fname;      // Name/path of file being edited
    char buf[EDIT_BUFFER_SIZE];  // Buffer used for editing the text
    int buf_len;            // Length of the buffer, currently always EDIT_BUFFER_SIZE
    int num_lines;          // Number of lines of text held in the buffer
    int width;              // Width of the editor screen in characters
    int height;             // Height of the editor screen in characters
    int py;                 // Row at top left hand corner of editor
    int cx;                 // Current cursor column (from 0)
    int cy;                 // Current cursor row (from 0)
    char *txtp;             // Position of the cursor in the text being edited
    bool message_shown;     // True if a message is currently being shown
    bool insert;            // True if the editor is in INSERT mode
    int preferred_x;        // User to track preferred x-position when up/down arrowing
    bool text_changed;      // True if the text has been editor and thus may need saving
    PmEditorMode mode;      // Edit mode or Mark mode ?
    char last_key;          // Last key pressed
    char clipboard[MAXCLIP + 2];  // Clipboard contents
    char keys[MAXCLIP + 2]; // Buffer of incoming keystrokes
    bool exit_flag;         // True if the editor should exit
    char saved_break_key;   // Original value of mmb_options.break_key when editor entered
    char *mark;             // Current position of the mark in mark mode
    HighlightType highlight; // Current highlight
    char *mark_lb;          // Lower bound of the selection in mark mode
    char *mark_ub;          // Upper bound of the selection in mark mode
} PmEditor;

// Copies position fields between PmEditorPos and PmEditor structures
// Works in both directions: COPY_POS(dst, src)
#define COPY_POS(dst, src) { \
    (dst).py = (src).py; \
    (dst).cy = (src).cy; \
    (dst).txtp = (src).txtp; \
    (dst).mark = (src).mark; \
}

// Extracts position fields into a PmEditorPos initializer
#define POS_FROM(src) { \
    .py = (src).py, \
    .cx = (src).cx, \
    .cy = (src).cy, \
    .txtp = (src).txtp, \
    .mark = (src).mark \
}

// By changing these function pointers unit-tests can override "display"
// behaviour.
extern MmResult (*pmeditor_display_msg)(PmEditor *, const char *);
extern MmResult (*pmeditor_highlight)(PmEditor *, HighlightType);
extern MmResult (*pmeditor_print_line)(PmEditor *, int);
extern MmResult (*pmeditor_print_lines)(PmEditor *, unsigned, unsigned);
extern MmResult (*pmeditor_print_screen)(PmEditor *);

MmResult pmeditor_cmd_backspace(PmEditor *self);
MmResult pmeditor_cmd_char(PmEditor *self);
MmResult pmeditor_cmd_down(PmEditor *self);
MmResult pmeditor_cmd_end(PmEditor *self);
MmResult pmeditor_cmd_home(PmEditor *self);
MmResult pmeditor_cmd_left(PmEditor *self);
MmResult pmeditor_cmd_page_up(PmEditor *self);
MmResult pmeditor_cmd_page_down(PmEditor *self);
MmResult pmeditor_cmd_right(PmEditor *self);
MmResult pmeditor_cmd_up(PmEditor *self);
MmResult pmeditor_delete_char(PmEditor *self, int *redraw);
char *pmeditor_back_in_line(PmEditor *self, char *start, size_t num_chars);
char *pmeditor_find_in_line(PmEditor *self, const char *needle, char *start, size_t max_len);
char *pmeditor_find_line(PmEditor *self, int line, int *comment_level);
MmResult pmeditor_get_highlight(PmEditor *self, SyntaxState *syntax, char *p, HighlightType *highlight);
MmResult pmeditor_find_longest_line(PmEditor *self, int *line, int *length);
MmResult pmeditor_init(PmEditor *self, const char *filename, int width, int height);
MmResult pmeditor_init_syntax_state(PmEditor *self);
MmResult pmeditor_insert_char(PmEditor *self, char ch, int *redraw);
MmResult pmeditor_mark_delete(PmEditor *self);
MmResult pmeditor_mark_down(PmEditor *self);
MmResult pmeditor_mark_end(PmEditor *self);
MmResult pmeditor_mark_home(PmEditor *self);
MmResult pmeditor_mark_left(PmEditor *self);
MmResult pmeditor_mark_up(PmEditor *self);
MmResult pmeditor_mark_right(PmEditor *self);
MmResult pmeditor_overwrite_char(PmEditor *self, char ch, int *redraw);
MmResult pmeditor_position_cursor(PmEditor *self, char *curp);
MmResult pmeditor_print_selection(PmEditor *self, PmEditorPos *old_pos);
char *pmeditor_start_of_line_n(PmEditor *self, int line);
void pmeditor_restore_fn_pointers();

/**
 * Finds the end of the line containing the given position.
 *
 * @param  self  Pointer to the PmEditor instance (unused).
 * @param  p     Pointer to any position within the current line.
 * @return       Pointer to the newline character at the end of the line,
 *               or to '\0' if at the last line.
 */
static inline char *pmeditor_end_of_line(PmEditor *self, char *p) {
    while (*p != '\0' && *p != '\n') p++;
    return p;
}

/**
 * Finds the start of the next line in the buffer.
 *
 * @param  self  Pointer to the PmEditor instance (unused).
 * @param  p     Pointer to any position within the current line.
 * @return       Pointer to the first character of the next line, or NULL
 *               if at the last line.
 */
static inline char *pmeditor_next_line(PmEditor *self, char *p) {
    while (*p != '\n' && *p != '\0') p++;
    if (*p == '\0') return NULL;
    p++; // Skip newline
    return p;
}

/**
 * Finds the start of the line containing the given position.
 *
 * @param  self  Pointer to the PmEditor instance.
 * @param  p     Pointer to any position within the current line.
 * @return       Pointer to the first character of the line.
 */
static inline char *pmeditor_start_of_line(PmEditor *self, char *p) {
    while (p != self->buf && *(p - 1) != '\n') p--;
    return p;
}

/**
 * Calculates the length of the line containing the given position.
 *
 * @param  self  Pointer to the PmEditor instance.
 * @param  p     Pointer to any position within the line.
 * @return       Total number of characters in the line, excluding the
 *               newline character.
 */
static inline int pmeditor_line_length(PmEditor *self, char *p) {
    char *start = pmeditor_start_of_line(self, p);
    int len = 0;
    while (*start != '\n' && *start != '\0') {
        len++;
        start++;
    }
    return len;
}

/**
 * Finds the start of the previous line in the buffer.
 *
 * @param  self  Pointer to the PmEditor instance.
 * @param  p     Pointer to any position within the current line.
 * @return       Pointer to the first character of the previous line, or NULL
 *               if at the first line.
 */
static inline char *pmeditor_previous_line(PmEditor *self, char *p) {
    // Move to the start of this line
    p = pmeditor_start_of_line(self, p);

    // Return NULL if we were already on the first line
    if (p == self->buf) return NULL;

    // Skip over the '\n' character to move to the end of the previous line
    p--;

    // Move to start of the previous line
    return pmeditor_start_of_line(self, p);
}

#endif // #if !defined(MMB4L_PMEDITOR_PRIVATE)
