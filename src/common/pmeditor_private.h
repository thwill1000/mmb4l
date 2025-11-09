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
    kHighlightNormal,
    kHighlightComment,
    kHighlightKeyword,
    kHighlightQuote,
    kHighlightNumber,
    kHighlightLine,
    kHighlightStatus,
    kHighlightError,
    kHighlightTrailingWhitespace,
} HighlightType;

typedef struct SyntaxState {
    bool incomment;
    bool inquote;
    bool inkeyword;
    bool innumber;
    bool intext;
    bool just_exited_comment;
    char *twokeyword;
} SyntaxState;

typedef struct s_PmEditor {
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
    bool redraw_status_line;  // True if the status line needs redrawing on next keystroke
    bool insert;            // True if the editor is in INSERT mode
    int preferred_x;        // User to track preferred x-position when up/down arrowing
    bool text_changed;      // True if the etxt has been editor and thus may need saving
    int comment_level;      // Tracks current multiline comment depth in pmeditor_print_line()
    bool mark_mode;         // True if we are in mark mode
    char last_key;          // Last key pressed
    char clipboard[MAXCLIP + 2];  // Clipboard contents
    char keys[MAXCLIP + 2]; // Buffer of incoming keystrokes
    bool exit_flag;         // True if the editor should exit
    char saved_break_key;   // Original value of mmb_options.break_key when editor entered
    char *mark;             // Current position of the mark in mark mode
    SyntaxState syntax;     // Current syntax highlighting state
} PmEditor;

// By changing these function pointers unit-tests can override "display"
// behaviour.
extern MmResult (*pmeditor_display_msg)(PmEditor *, const char *);
extern MmResult (*pmeditor_highlight)(PmEditor *, HighlightType);
extern MmResult (*pmeditor_print_screen)(PmEditor *);

MmResult pmeditor_cmd_backspace(PmEditor *self);
MmResult pmeditor_cmd_char(PmEditor *self);
MmResult pmeditor_delete_char(PmEditor *self, int *redraw);
char *pmeditor_back_in_line(PmEditor *self, char *start, size_t num_chars);
char *pmeditor_find_in_line(PmEditor *self, const char *needle, char *start, size_t max_len);
char *pmeditor_find_line(PmEditor *self, int line); // , int *comment_level);
MmResult pmeditor_find_longest_line(PmEditor *self, int *line, int *length);
MmResult pmeditor_init(PmEditor *self, const char *filename, int width, int height);
MmResult pmeditor_init_syntax_state(PmEditor *self);
MmResult pmeditor_insert_char(PmEditor *self, char ch, int *redraw);
void pmeditor_restore_fn_pointers();
MmResult pmeditor_set_colour(PmEditor *self, char *p);

#endif // #if !defined(MMB4L_PMEDITOR_PRIVATE)
