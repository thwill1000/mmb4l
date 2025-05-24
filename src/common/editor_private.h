/*-*****************************************************************************

MMBasic for Linux (MMB4L)

editor_private.h

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

#if !defined(MMB4L_EDITOR_PRIVATE)
#define MMB4L_EDITOR_PRIVATE

#include <stdbool.h>
#include <stdint.h>

#include "mmresult.h"
#include "program.h" // for EDIT_BUFFER_SIZE

#define MAXCLIP                  1024
#define LAST_LINE                INT32_MAX
#define NO_CHANGE                -1
#define MAX_LINE_LENGTH          MAXSTRLEN
#define EMSG_CLIPBOARD_EMPTY     "CLIPBOARD IS EMPTY"
#define EMSG_CLIPBOARD_OVERFLOW  "MARKED TEXT EXCEEDS CLIPBOARD BUFFER SIZE"
#define EMSG_EDIT_BUFFER_FULL    "EDIT BUFFER FULL"
#define EMSG_LINE_TOO_LONG       "LINE IS TOO LONG"
#define EMSG_NOT_FOUND           "NOT FOUND"
#define SOFT_MARGIN              5

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
    kHighlightMark = 0xFE,  // bit 7 set for inverse
    kHighlightRight = 0xFF, // bit 7 set for inverse
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
} EditorMode;

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
    size_t buf_sz;           ///< Edit buffer size (EDIT_BUFFER_SIZE)
    int num_lines;           ///< Line count in buffer

    // Display
    int width;               ///< Editor width in characters
    int height;              ///< Editor height in characters (excludes status line)

    // Cursor and viewport
    int px;                  ///< TODO
    int py;                  ///< Top line displayed (scroll position)
    int cx;                  ///< Cursor column (viewport-relative, 0-based)
    int cy;                  ///< Cursor row (viewport-relative, 0-based)
    char *txtp;              ///< Cursor position in buffer
    int preferred_x;         ///< Preferred column for vertical navigation

    // Editing state
    bool insert;             ///< True for INSERT mode, false for OVERWRITE
    bool text_changed;       ///< True if buffer modified
    EditorMode mode;       ///< Edit, Mark, or Exit mode
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
} Editor;

// By changing these function pointers unit-tests can override "display"
// behaviour.
extern MmResult (*editor_print_msg)(Editor *, const char *);
extern MmResult (*editor_highlight)(Editor *, HighlightType);
extern MmResult (*editor_print_func_keys)(Editor *);
extern MmResult (*editor_print_line_fast)(Editor *, char *);
extern MmResult (*editor_print_lines)(Editor *, int, int);
extern MmResult (*editor_print_status)(Editor *);

MmResult editor_construct(Editor *self, const char *filename, int width, int height);
MmResult editor_destruct(Editor *self);
MmResult editor_adjust_viewport(Editor *self);
MmResult editor_cmd_backspace(Editor *self);
MmResult editor_cmd_char(Editor *self);
MmResult editor_cmd_copy(Editor *self);
MmResult editor_cmd_cut(Editor *self);
MmResult editor_cmd_delete(Editor *self);
MmResult editor_cmd_delete_selection(Editor *self);
MmResult editor_cmd_down(Editor *self);
MmResult editor_cmd_end(Editor *self);
MmResult editor_cmd_home(Editor *self);
MmResult editor_cmd_left(Editor *self);
MmResult editor_cmd_newline(Editor *self);
MmResult editor_cmd_page_up(Editor *self);
MmResult editor_cmd_page_down(Editor *self);
MmResult editor_cmd_paste(Editor *self);
MmResult editor_cmd_right(Editor *self);
MmResult editor_cmd_search_again(Editor *self);
MmResult editor_cmd_up(Editor *self);
char *editor_back_in_line(Editor *self, char *start, size_t num_chars);
char *editor_end_of_line_n(Editor *self, int line);
char *editor_find_in_line(Editor *self, const char *needle, char *start, size_t max_len);
char *editor_find_line_ex(Editor *self, int line, int *comment_level);
MmResult editor_get_highlight(Editor *self, SyntaxState *syntax, char *p, HighlightType *highlight);
size_t editor_get_selection(Editor *self, char **start, char **end);
MmResult editor_find_longest_line(Editor *self, int *line, int *length);
MmResult editor_init_syntax_state(Editor *self);
MmResult editor_print_selection(Editor *self, Editor *old);
void editor_restore_fn_pointers();
MmResult editor_set_changed_lines(Editor *self, int start, int end);
char *editor_start_of_line_n(Editor *self, int line);
MmResult editor_sync_cursor_to_buffer(Editor *self, char *curp);
MmResult editor_update_display(Editor *self, Editor *old);

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
static inline Editor editor_shallow_copy(Editor *src) {
    Editor dst = *src;
    return dst;
}

/**
 * Finds the end of the line containing the given position.
 *
 * @param  self  Pointer to the Editor instance (unused).
 * @param  p     Pointer to any position within the current line.
 * @return       Pointer to the newline character at the end of the line,
 *               or to '\0' if at the last line.
 */
static inline char *editor_end_of_line(Editor *self, char *p) {
    if (!p) return NULL;
    while (*p != '\0' && *p != '\n') p++;
    return p;
}

/**
 * Finds the start of the next line in the buffer.
 *
 * @param  self  Pointer to the Editor instance (unused).
 * @param  p     Pointer to any position within the current line.
 * @return       Pointer to the first character of the next line, or NULL
 *               if at the last line.
 */
static inline char *editor_next_line(Editor *self, char *p) {
    if (!p) return NULL;
    while (*p != '\n' && *p != '\0') p++;
    if (*p == '\0') return NULL;
    p++; // Skip newline
    return p;
}

/**
 * Finds the start of the line containing the given position.
 *
 * @param  self  Pointer to the Editor instance.
 * @param  p     Pointer to any position within the current line.
 * @return       Pointer to the first character of the line.
 */
static inline char *editor_start_of_line(Editor *self, char *p) {
    if (!p) return NULL;
    while (p != self->buf && *(p - 1) != '\n') p--;
    return p;
}

/**
 * Calculates the length of the line containing the given position.
 *
 * @param  self  Pointer to the Editor instance.
 * @param  p     Pointer to any position within the line.
 * @return       Total number of characters in the line, excluding the
 *               newline character.
 */
static inline int editor_line_length(Editor *self, char *p) {
    if (!p) return -1;
    char *start = editor_start_of_line(self, p);
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
 * @param  self  Pointer to the Editor instance.
 * @param  p     Pointer to any position within the current line.
 * @return       Pointer to the first character of the previous line, or NULL
 *               if at the first line.
 */
static inline char *editor_previous_line(Editor *self, char *p) {
    // Move to the start of this line
    p = editor_start_of_line(self, p);

    // Return NULL if we were already on the first line
    if (p == self->buf) return NULL;

    // Skip over the '\n' character to move to the end of the previous line
    p--;

    // Move to start of the previous line
    return editor_start_of_line(self, p);
}

/**
 * Calculates the 0-based line and column number for a buffer position.
 *
 * Scans from buffer start, counting newlines for line number and characters
 * since last newline for column number.
 *
 * @param       self    Pointer to the Editor instance.
 * @param       pbuf    Buffer position to query.
 * @param[out]  line    Resulting 0-based line number.
 * @param[out]  column  Resulting 0-based column number.
 * @return              kOk on success, or an error code on failure.
 *
 * @note Performance is O(n) where n is the distance from buffer start to pbuf.
 */
static inline MmResult editor_get_line_and_column(Editor *self, char *pbuf, int *line,
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

#endif // #if !defined(MMB4L_EDITOR_PRIVATE)
