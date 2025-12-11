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
#define LAST_LINE  INT32_MAX
#define NO_CHANGE  -1

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
    kHighlightMark = 0xFF,  // bit 7 set for inverse
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

typedef enum {
    kModeUnspecified = 0,
    kEditMode,
    kMarkMode,
    kExitMode,
} PmEditorMode;

/**
 * Main editor state structure.
 *
 * Manages text buffer, cursor position (cx, cy, txtp), viewport (py),
 * and editing mode. Uses single contiguous allocation for buf,
 * clipboard_buf, and key_buf.
 *
 * Coordinates: (cx, cy) are viewport-relative, py is scroll offset,
 * txtp is absolute buffer position. Actual line = py + cy.
 */
typedef struct {
    // File and buffer
    const char *fname;       ///< Name/path of file being edited
    char *buf;               ///< Buffer containing text (base of single allocation)
    int buf_sz;              ///< Edit buffer size (EDIT_BUFFER_SIZE)
    int num_lines;           ///< Line count in buffer

    // Display
    int width;               ///< Editor width in characters
    int height;              ///< Editor height in characters (excludes status line)

    // Cursor and viewport
    int py;                  ///< Top line displayed (scroll position)
    int cx;                  ///< Cursor column (viewport-relative, 0-based)
    int cy;                  ///< Cursor row (viewport-relative, 0-based)
    char *txtp;              ///< Cursor position in buffer
    int preferred_x;         ///< Preferred column for vertical navigation

    // Editing state
    bool insert;             ///< True for INSERT mode, false for OVERWRITE
    bool text_changed;       ///< True if buffer modified
    PmEditorMode mode;       ///< Edit, Mark, or Exit mode
    char *mark;              ///< Mark position for text selection

    // Change tracking (for efficient redrawing)
    int change_start;        ///< First changed line (NO_CHANGE if none)
    int change_end;          ///< Last changed line (NO_CHANGE if none)

    // Display state
    char message[64];        ///< Message being shown
    HighlightType highlight; ///< Current syntax highlighting

    // Input
    char last_key;           ///< Last key pressed (for double-press detection)
    char *key_buf;           ///< Keystroke buffer (MAXCLIP+2 bytes)
    char saved_break_key;    ///< Original break key (restored on exit)

    // Clipboard
    char *clipboard_buf;     ///< Clipboard storage (MAXCLIP+2 bytes)
} PmEditor;

// By changing these function pointers unit-tests can override "display"
// behaviour.
extern MmResult (*pmeditor_print_msg)(PmEditor *, const char *);
extern MmResult (*pmeditor_highlight)(PmEditor *, HighlightType);
extern MmResult (*pmeditor_print_func_keys)(PmEditor *);
extern MmResult (*pmeditor_print_lines)(PmEditor *, int, int);
extern MmResult (*pmeditor_print_status)(PmEditor *);

MmResult pmeditor_construct(PmEditor *self, const char *filename, int width, int height);
MmResult pmeditor_destruct(PmEditor *self);
MmResult pmeditor_cmd_backspace(PmEditor *self);
MmResult pmeditor_cmd_char(PmEditor *self);
MmResult pmeditor_cmd_copy(PmEditor *self);
MmResult pmeditor_cmd_cut(PmEditor *self);
MmResult pmeditor_cmd_delete_selection(PmEditor *self);
MmResult pmeditor_cmd_down(PmEditor *self);
MmResult pmeditor_cmd_end(PmEditor *self);
MmResult pmeditor_cmd_home(PmEditor *self);
MmResult pmeditor_cmd_left(PmEditor *self);
MmResult pmeditor_cmd_page_up(PmEditor *self);
MmResult pmeditor_cmd_page_down(PmEditor *self);
MmResult pmeditor_cmd_right(PmEditor *self);
MmResult pmeditor_cmd_up(PmEditor *self);
MmResult pmeditor_delete_char(PmEditor *self);
char *pmeditor_back_in_line(PmEditor *self, char *start, size_t num_chars);
char *pmeditor_find_in_line(PmEditor *self, const char *needle, char *start, size_t max_len);
char *pmeditor_find_line_ex(PmEditor *self, int line, int *comment_level);
MmResult pmeditor_get_highlight(PmEditor *self, SyntaxState *syntax, char *p, HighlightType *highlight);
MmResult pmeditor_find_longest_line(PmEditor *self, int *line, int *length);
MmResult pmeditor_init_syntax_state(PmEditor *self);
MmResult pmeditor_insert_char(PmEditor *self, char ch);
MmResult pmeditor_overwrite_char(PmEditor *self, char ch);
MmResult pmeditor_position_cursor(PmEditor *self, char *curp);
MmResult pmeditor_print_selection(PmEditor *self, PmEditor *old);
char *pmeditor_find_line_n(PmEditor *self, int line);
void pmeditor_restore_fn_pointers();
MmResult pmeditor_update_display(PmEditor *self, PmEditor *old);

/**
 * Creates a shallow copy of the editor state.
 *
 * Copies all editor fields including pointers (buf, txtp, mark, fname).
 * The copy shares the same buffer memory as the source - modifications
 * to the buffer through either instance will affect both.
 *
 * @param  src  Source editor to copy.
 * @return      Shallow copy of the editor.
 */
static inline PmEditor pmeditor_shallow_copy(PmEditor *src) {
    PmEditor dst = *src;
    return dst;
}

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

/**
 * Calculates the 0-based line and column number for a buffer position.
 *
 * Scans from buffer start, counting newlines for line number and characters
 * since last newline for column number.
 *
 * @param       self    Pointer to the PmEditor instance.
 * @param       pbuf    Buffer position to query.
 * @param[out]  line    Resulting 0-based line number.
 * @param[out]  column  Resulting 0-based column number.
 * @return              kOk on success, or an error code on failure.
 *
 * @note Performance is O(n) where n is the distance from buffer start to pbuf.
 */
static inline MmResult pmeditor_get_line_and_column(PmEditor *self, char *pbuf, int *line,
                                                    int *column) {
    if (!self || !pbuf || !line || !column) {
        return mmresult_ex(kInternalFault,
                           "invalid parameter: self=%p, pbuf=%p, line=%p, column=%p",
                           self, pbuf, line, column);
    }

    if (pbuf < self->buf || pbuf >= self->buf + self->buf_sz) {
        return mmresult_ex(kInternalFault,
                           "pbuf out of bounds: pbuf=%p, buf=%p, buf_sz=%d",
                           pbuf, self->buf, self->buf_sz);
    }

    *column = 0;
    *line = 0;
    for (char *p = self->buf; p != pbuf && p < self->buf + self->buf_sz; p++) {
        switch (*p) {
            case '\0':
                return mmresult_ex(kInternalFault, "pbuf beyond text: pbuf=%p", pbuf);
            case '\n':
                (*line)++;
                *column = 0;
                break;
            default:
                (*column)++;
                break;
        }
    }
    return kOk;
}

#endif // #if !defined(MMB4L_PMEDITOR_PRIVATE)
