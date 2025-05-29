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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "error.h"
#include "file.h"
#include "fonttbl.h"
#include "graphics.h"
#include "memory.h"
#include "mmb4l.h"
#include "mmtime.h"
#include "program.h"
#include "../core/commandtbl.h"
#include "../core/MMBasic.h"
#include "../core/tokentbl.h"

#define DisplayPutC  console_putc
#define gui_bcolour  graphics_bcolour
#define gui_fcolour  graphics_fcolour
#define CurrentX     graphics_current->cursor_x
#define CurrentY     graphics_current->cursor_y
#define HRes         graphics_current->width
#define VRes         graphics_current->height
#define OPTION_CONTINUATION     false
#define OPTION_COLOUR_CODE      true
#define OPTION_DISPLAY_CONSOLE  true
#define gui_font_height  font_height(graphics_font)
#define gui_font_width   font_width(graphics_font)
#define SCREENMODE1   1
#define OPTION_DISPLAY_TYPE  SCREENMODE1
#define SCREENHEIGHT  40
#define SCREENWIDTH   80
#define FF_MAX_LFN    63
#define MMInkey  console_getc

const char *ANSI_UNDERLINE     = "\033[4m";
const char *ANSI_RESET         = "\033[0m";
const char *ANSI_CLEAR_TO_EOL  = "\033[K";
const char *ANSI_REVERSE_VIDEO = "\033[7m";
const char *ANSI_FG_BLACK      = "\033[30m";
const char *ANSI_FG_RED        = "\033[31m";
const char *ANSI_FG_GREEN      = "\033[32m";
const char *ANSI_FG_YELLOW     = "\033[33m";
const char *ANSI_FG_BLUE       = "\033[34m";
const char *ANSI_FG_MAGENTA    = "\033[35m";
const char *ANSI_FG_CYAN       = "\033[36m";
const char *ANSI_FG_WHITE      = "\033[37m";

const char *llist(char *b, const char *p);
int MMgetchar(void);

//#include "Hardware_Includes.h"
//#include "MMBasic_Includes.h"

#define CTRLKEY(a) (a & 0x1f)

#define DISPLAY_CLS 1
#define REVERSE_VIDEO 3
#define CLEAR_TO_EOL 4
#define CLEAR_TO_EOS 5
#define SCROLL_DOWN 6
#define DRAW_LINE 7

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

#define GUI_C_NORMAL WHITE
#define GUI_C_BCOLOUR BLACK
#define GUI_C_COMMENT YELLOW
#define GUI_C_KEYWORD CYAN
#define GUI_C_QUOTE MAGENTA
#define GUI_C_NUMBER GREEN
#define GUI_C_LINE MAGENTA
#define GUI_C_STATUS WHITE

// these two are the only global variables, the default place for the cursor when the editor opens
char *StartEditPoint = NULL;
int StartEditChar = 0;
static bool markmode = false;
void routinechecks(void) {}
int8_t optioncolourcodesave;

void ShowCursor(bool show) {}

//#if !defined(LITE)
#ifdef PICOMITEVGA
int editactive = 0;
static int r_on = 0;
void DisplayPutClever(char c) {
    if ((DISPLAY_TYPE == SCREENMODE1 && markmode && OPTION_COLOUR_CODE &&
         ytileheight == gui_font_height && gui_font_width % 8 == 0)) {
        if (c >= FontTable[gui_font >> 4][2] &&
            c < FontTable[gui_font >> 4][2] + FontTable[gui_font >> 4][3]) {
            if (CurrentX + gui_font_width > HRes) {
                DisplayPutClever('\r');
                DisplayPutClever('\n');
            }
        }

        // handle the standard control chars
        switch (c) {
            case '\b':
                CurrentX -= gui_font_width;
                if (CurrentX < 0) CurrentX = 0;
                return;
            case '\r':
                CurrentX = 0;
                return;
            case '\n':
                CurrentY += gui_font_height;
                if (CurrentY + gui_font_height >= VRes) {
                    ScrollLCD(CurrentY + gui_font_height - VRes);
                    CurrentY -= (CurrentY + gui_font_height - VRes);
                }
                return;
            case '\t':
                do {
                    DisplayPutClever(' ');
                } while ((CurrentX / gui_font_width) % mmb_options.tab);
                return;
        }
#ifdef HDMI
        if (r_on) {
            if (FullColour) {
                if (r_on)
                    for (int i = 0; i < gui_font_width / 8; i++)
                        tilebcols[CurrentY / gui_font_height * X_TILE + CurrentX / 8 + i] =
                            RGB555(BLUE);
            } else {
                if (r_on)
                    for (int i = 0; i < gui_font_width / 8; i++)
                        tilebcols_w[CurrentY / gui_font_height * X_TILE + CurrentX / 8 + i] =
                            RGB332(BLUE);
            }
        }
#else
        if (r_on)
            for (int i = 0; i < gui_font_width / 8; i++)
                tilebcols[CurrentY / gui_font_height * X_TILE + CurrentX / 8 + i] = 0x1111;
#endif
#ifdef HDMI
        else {
            if (FullColour) {
                for (int i = 0; i < gui_font_width / 8; i++)
                    tilebcols[CurrentY / gui_font_height * X_TILE + CurrentX / 8 + i] =
                        RGB555(Option.DefaultBC);
            } else {
                for (int i = 0; i < gui_font_width / 8; i++)
                    tilebcols_w[CurrentY / gui_font_height * X_TILE + CurrentX / 8 + i] =
                        RGB332(Option.DefaultBC);
            }
        }
#else
        else
            for (int i = 0; i < gui_font_width / 8; i++)
                tilebcols[CurrentY / gui_font_height * X_TILE + CurrentX / 8 + i] =
                    RGB121pack(Option.DefaultBC);
#endif
        CurrentX += gui_font_width;
    } else
        DisplayPutC(c);
}
#endif

void DisplayPutS(const char *s) {
    while (*s) DisplayPutC(*s++);
}

static inline void printnothing(char *dummy) {}
static inline char nothingchar(char dummy, int flush) { return 0; }
int OriginalFC, OriginalBC;  // the original fore/background colours used by MMBasic

#define  PrintString  pmeditor_puts

/** Prints a string of text. */
static inline MmResult pmeditor_puts(const char *s) {
    console_puts(s);
    return kOk;
}

static void SSputchar(const char ch, int flush) {
    console_putc(ch);
    fflush(stdout);
} // = SerialConsolePutC;
//    #define PrintString       SSPrintString
#ifdef PICOMITEVGA
#define MX470PutC(c) DisplayPutClever(c)
#else
#define MX470PutC(c) DisplayPutC(c)
#endif
// Only SSD1963 displays support H/W scrolling so for other displays it is much quicker to redraw
// the screen than scroll it However, we don't want to redraw the serial console so we dummy out the
// serial writes whiole re-drawing the physical screen
// #ifdef PICOMITEVGA
// #define MX470Scroll(n) ScrollLCD(n);
// #else
#if 0
#define MX470Scroll(n)                                                       \
    if (OPTION_DISPLAY_CONSOLE) {                                            \
        if (!((SPIREAD && ScrollLCD != ScrollLCDSPISCR) || Option.NoScroll)) \
            ScrollLCD(n);                                                    \
        else {                                                               \
            PrintString = printnothing;                                      \
            SSputchar = nothingchar;                                         \
            printScreen();                                                   \
            PrintString = SSPrintString;                                     \
            SSputchar = SerialConsolePutC;                                   \
        }                                                                    \
    }
#endif
// #endif
#define MX470Scroll(n)

//    #define dx(...) {char s[140];sprintf(s,  __VA_ARGS__); SerUSBPutS(s); SerUSBPutS("\r\n");}

static inline void MX470PutS(const char *s, int fc, int bc) {
    if (!OPTION_DISPLAY_CONSOLE) return;
    int tfc, tbc;
    tfc = gui_fcolour;
    tbc = gui_bcolour;
    gui_fcolour = fc;
    gui_bcolour = bc;
    DisplayPutS(s);
    gui_fcolour = tfc;
    gui_bcolour = tbc;
}

static MmResult pmeditor_cls() {
    if (!graphics_current) {
        console_clear();
        return kOk;
    }
    // ON_FAILURE_ERROR(graphics_cls(graphics_current, gui_bcolour));
    // ClearScreen(gui_bcolour);
    return kUnimplemented;
}

/**
 * Sets the graphics cursor position.
 *
 * @param  x  x-coordinate, in pixels, starting at 0 (left).
 * @param  y  y-coordinate, in pixels, starting at 0 (top).
 */
static inline MmResult pmeditor_graphics_set_cursor(int x, int y) {
    if (!OPTION_DISPLAY_CONSOLE || !graphics_current) return kOk;
    graphics_current->cursor_x = x;
    graphics_current->cursor_y = y;
    return kOk;
}

#define  MX470Cursor  pmeditor_graphics_set_cursor

MmResult pmeditor_draw_box(int x1, int y1, int x2, int y2, MmGraphicsColour fcolour,
                           MmGraphicsColour bcolour) {
    if (!graphics_current) return kOk;
    return graphics_draw_box(graphics_current, x1, y1, x2, y2, 0, fcolour, bcolour);
}

MmResult pmeditor_draw_line(int x1, int y1, int x2, int y2) {
    if (!graphics_current) return kOk;
    return graphics_draw_line(
            graphics_current, x1, y1, x2, y2, 1, GUI_C_LINE);
}

void MX470Display(int fn) {
    if (!OPTION_DISPLAY_CONSOLE) return;
    int t;
    switch (fn) {
        case DISPLAY_CLS:
            ON_FAILURE_ERROR(pmeditor_cls());
            break;
#ifdef PICOMITEVGA
        case CLEAR_TO_EOS:
            MX470Display(CLEAR_TO_EOL);
            DrawRectangle(0, CurrentY + gui_font_height, HRes - 1, VRes - 1,
                          DISPLAY_TYPE == SCREENMODE1 ? 0 : gui_bcolour);
#ifdef HDMI
            if (FullColour) {
                if (DISPLAY_TYPE == SCREENMODE1 && OPTION_COLOUR_CODE &&
                    ytileheight == gui_font_height) {
                    for (int y = (CurrentY + gui_font_height) / ytileheight; y < Y_TILE; y++)
                        for (int x = 0; x < X_TILE; x++) {
                            tilefcols[y * X_TILE + x] = RGB555(gui_fcolour);
                            tilebcols[y * X_TILE + x] = RGB555(gui_bcolour);
                        }
                }
            } else {
                if (DISPLAY_TYPE == SCREENMODE1 && OPTION_COLOUR_CODE &&
                    ytileheight == gui_font_height) {
                    for (int y = (CurrentY + gui_font_height) / ytileheight; y < Y_TILE; y++)
                        for (int x = 0; x < X_TILE; x++) {
                            tilefcols_w[CurrentY / ytileheight * X_TILE + x] = RGB332(gui_fcolour);
                            tilebcols_w[CurrentY / ytileheight * X_TILE + x] = RGB332(gui_bcolour);
                        }
                }
            }
#else
            if (DISPLAY_TYPE == SCREENMODE1 && OPTION_COLOUR_CODE &&
                ytileheight == gui_font_height) {
                for (int y = (CurrentY + gui_font_height) / ytileheight; y < Y_TILE; y++)
                    for (int x = 0; x < X_TILE; x++) {
                        tilefcols[CurrentY / ytileheight * X_TILE + x] = RGB121pack(gui_fcolour);
                        tilebcols[CurrentY / ytileheight * X_TILE + x] = RGB121pack(gui_bcolour);
                    }
            }

#endif
            break;
        case REVERSE_VIDEO:
            if ((DISPLAY_TYPE == SCREENMODE1 && OPTION_COLOUR_CODE &&
                 ytileheight == gui_font_height)) {
                r_on ^= 1;
            } else {
                t = gui_fcolour;
                gui_fcolour = gui_bcolour;
                gui_bcolour = t;
            }
            break;
        case CLEAR_TO_EOL:
            DrawBox(CurrentX, CurrentY, HRes - 1, CurrentY + gui_font_height - 1, 0, 0,
                    DISPLAY_TYPE == SCREENMODE1 ? 0 : gui_bcolour);
#ifdef HDMI
            if (FullColour) {
                if (DISPLAY_TYPE == SCREENMODE1 && OPTION_COLOUR_CODE &&
                    ytileheight == gui_font_height) {
                    for (int x = CurrentX / 8; x < X_TILE; x++) {
                        tilefcols[CurrentY / ytileheight * X_TILE + x] = RGB555(gui_fcolour);
                        tilebcols[CurrentY / ytileheight * X_TILE + x] = RGB555(gui_bcolour);
                    }
                }
            } else {
                if (DISPLAY_TYPE == SCREENMODE1 && OPTION_COLOUR_CODE &&
                    ytileheight == gui_font_height) {
                    for (int x = CurrentX / 8; x < X_TILE; x++) {
                        tilefcols_w[CurrentY / ytileheight * X_TILE + x] = RGB332(gui_fcolour);
                        tilebcols_w[CurrentY / ytileheight * X_TILE + x] = RGB332(gui_bcolour);
                    }
                }
            }
#else
            if (DISPLAY_TYPE == SCREENMODE1 && OPTION_COLOUR_CODE &&
                ytileheight == gui_font_height) {
                for (int x = CurrentX / 8; x < X_TILE; x++) {
                    tilefcols[CurrentY / ytileheight * X_TILE + x] = RGB121pack(gui_fcolour);
                    tilebcols[CurrentY / ytileheight * X_TILE + x] = RGB121pack(gui_bcolour);
                }
            }

#endif

            break;
#else
        case REVERSE_VIDEO:
            t = gui_fcolour;
            gui_fcolour = gui_bcolour;
            gui_bcolour = t;
            break;
        case CLEAR_TO_EOL:
            if (graphics_current) {
                ON_FAILURE_ERROR(pmeditor_draw_box(
                        graphics_current->cursor_x, graphics_current->cursor_y,
                        HRes - 1, graphics_current->cursor_y + gui_font_height - 1,
                    0, gui_bcolour));
                // DrawBox(CurrentX, CurrentY, HRes - 1, CurrentY + gui_font_height - 1, 0, 0,
                //         gui_bcolour);
            }
            break;
        case CLEAR_TO_EOS:
            if (graphics_current) {
                ON_FAILURE_ERROR(pmeditor_draw_box(
                        graphics_current->cursor_x, graphics_current->cursor_y,
                        HRes - 1, graphics_current->cursor_y + gui_font_height - 1,
                        0, gui_bcolour));
                // DrawBox(CurrentX, CurrentY, HRes - 1, CurrentY + gui_font_height - 1, 0, 0,
                //         gui_bcolour);
                ON_FAILURE_ERROR(
                    graphics_draw_rectangle(
                        graphics_current,
                        0, graphics_current->cursor_x + gui_font_height,
                        HRes - 1, VRes - 1,
                        gui_bcolour));
                // DrawRectangle(0, CurrentY + gui_font_height, HRes - 1, VRes - 1, gui_bcolour);
            }
            break;
#endif
        case SCROLL_DOWN:
            break;
        case DRAW_LINE:
            if (graphics_current) {
                ON_FAILURE_ERROR(pmeditor_draw_box(
                        0, gui_font_height * (mmb_options.height - 2),
                        HRes - 1, VRes - 1,
                        0, (OPTION_DISPLAY_TYPE == SCREENMODE1 ? 0 : gui_bcolour)));
                // DrawBox(0, gui_font_height * (mmb_options.height - 2), HRes - 1, VRes - 1, 0, 0,
                //         (DISPLAY_TYPE == SCREENMODE1 ? 0 : gui_bcolour));
                ON_FAILURE_ERROR(
                    graphics_draw_line(
                        graphics_current,
                        0, (VRes / gui_font_height) * gui_font_height - gui_font_height - 6,
                        HRes - 1, (VRes / gui_font_height) * gui_font_height - gui_font_height - 6,
                        1, GUI_C_LINE));
                // DrawLine(0, (VRes / gui_font_height) * gui_font_height - gui_font_height - 6, HRes - 1,
                //          (VRes / gui_font_height) * gui_font_height - gui_font_height - 6, 1,
                //          GUI_C_LINE);
            }
#ifdef PICOMITEVGA
#ifdef HDMI
            if (FullColour) {
                if (DISPLAY_TYPE == SCREENMODE1 && OPTION_COLOUR_CODE &&
                    ytileheight == gui_font_height)
                    for (int i = 0; i < HRes / 8; i++)
                        tilefcols[(Mmb_options.height - 2) * X_TILE + i] = RGB555(MAGENTA);
            } else {
                if (DISPLAY_TYPE == SCREENMODE1 && OPTION_COLOUR_CODE &&
                    ytileheight == gui_font_height)
                    for (int i = 0; i < HRes / 8; i++)
                        tilefcols_w[(Mmb_options.height - 2) * X_TILE + i] = RGB332(MAGENTA);
            }
#else
            if (DISPLAY_TYPE == SCREENMODE1 && OPTION_COLOUR_CODE && ytileheight == gui_font_height)
                for (int i = 0; i < HRes / 8; i++)
                    tilefcols[(Mmb_options.height - 2) * X_TILE + i] = 0x9999;
#endif
#endif
            if (graphics_current) {
                CurrentX = 0;
                CurrentY = (VRes / gui_font_height) * gui_font_height - gui_font_height;
            }
            break;
    }
}

/********************************************************************************************************************************************
 THE EDIT COMMAND
********************************************************************************************************************************************/

#define VHeight   pmeditor_height
#define VWidth    pmeditor_width
#define curx      pmeditor_cursor_x
#define cury      pmeditor_cursor_y
#define EdBuff    pmeditor_buf
#define nbrlines  pmeditor_num_lines
#define edx       pmeditor_page_x
#define edy       pmeditor_page_y

char *pmeditor_buf = NULL;  // Buffer used for editing the text.
int pmeditor_num_lines;     // Number of lines of text held in the buffer.
int pmeditor_width;         // Width of editor screen in characters.
int pmeditor_height;        // Height of editor screen in characters.
int pmeditor_page_x;        // Column at top left hand corner of editor.
int pmeditor_page_y;        // Row at top left hand corner of editor.
int pmeditor_cursor_x;      // Current cursor column (from 0).
int pmeditor_cursor_y;      // Current cursor row (from 0).) 
char *txtp;           // position of the current cursor in the text being edited
int drawstatusline;   // true if the status line needs to be redrawn on the next keystroke
int insert;           // true if the editor is in insert text mode
int tempx;            // used to track the prefered x position when up/down arrowing
int TextChanged;      // true if the program has been modified and therefor a save might be required

#define EDIT 1  // used to select the status line string
#define MARK 2

/**
 * Sets cursor position.
 *
 * @param  x  x-coordinate, in characters, starting at 0 (left).
 * @param  y  y-coordinate, in characters, starting at 0 (top).
 */ 
MmResult pmeditor_set_cursor(int x, int y) {
    ON_FAILURE_RETURN(
            pmeditor_graphics_set_cursor(x * gui_font_width, y * gui_font_height));
    console_set_cursor_char_pos(x, y);
    pmeditor_cursor_x = x;
    pmeditor_cursor_y = y;
    return kOk;
}

static inline MmResult pmeditor_underline() {
    return pmeditor_puts(ANSI_UNDERLINE);
}

static inline MmResult pmeditor_reset() {
    return pmeditor_puts(ANSI_RESET);
}

static inline MmResult pmeditor_clear_to_eol() {
    return pmeditor_puts(ANSI_CLEAR_TO_EOL);
}

static inline MmResult pmeditor_reverse_video() {
    return pmeditor_puts(ANSI_REVERSE_VIDEO);
}

static MmResult pmeditor_highlight(HighlightType highlight) {
    const char *ansi_fg = NULL;

    switch (highlight) {
        case kHighlightNormal:
            ansi_fg = ANSI_FG_WHITE;
            break;
        case kHighlightComment:
            ansi_fg = ANSI_FG_YELLOW;
            break;
        case kHighlightKeyword:
            ansi_fg = ANSI_FG_CYAN;
            break;
        case kHighlightQuote:
            ansi_fg = ANSI_FG_MAGENTA;
            break;
        case kHighlightNumber:
            ansi_fg = ANSI_FG_GREEN;
            break;
        case kHighlightLine:
            ansi_fg = ANSI_FG_MAGENTA;
            break;
        case kHighlightStatus:
            ansi_fg = ANSI_FG_WHITE;
            break;
        case kHighlightError:
            ansi_fg = ANSI_FG_WHITE;
            break;
        default:
            return kInternalFault;
    }

    return pmeditor_puts(ansi_fg);
}

void FullScreenEditor(int x, int y, const char *fname, int edit_buff_size);
char *findLine(int ln, int *inmulti);
void printLine(int ln);
void printScreen(void);
int editInsertChar(char c, char *multi, int edit_buff_size);
void PrintFunctKeys(int);
void PrintStatus(void);
void SaveToProgMemory(void);
void editDisplayMsg(const char *msg);
void GetInputString(char *prompt);
void Scroll(void);
void ScrollDown(void);
void MarkMode(char *cb, char *buf);
void PositionCursor(char *curp);
void setterminal() {}
static int multilinecomment = false;
bool modmode = false;
int oldfont;
int oldmode;
#define MAXCLIP 1024
// edit command:
//  EDIT              Will run the full screen editor on the current program memory, if run after an
//  error will place the cursor on the error line
void edit(const char *cmdline, bool cmdfile) {
    const char *fromp = NULL;
    char *p = NULL;
    int y, x;
    // TODO optioncolourcodesave = OPTION_COLOUR_CODE;
    char name[STRINGSIZE], *filename = NULL;
    getargs(&cmdline, 1, DELIM_COMMA);
    if (argc) {
        // strcpy(name, (char *)getFstring(argv[0]));
        strcpy(name, (char *)getCstring(argv[0]));
        filename = name;
    }
    if (CurrentLinePtr && cmdfile) error_throw_legacy("Invalid in a program");
    if (argc == 0 && !cmdfile) error_throw_legacy("Syntax");
    if (!cmdfile) {
        // SaveContext();
        // ClearVars(0, false);
        ClearVars(0);
    }
#ifdef PICOMITEVGA
    modmode = false;
    editactive = 1;
    oldmode = DISPLAY_TYPE;
    oldfont = PromptFont;
    if (HRes < 512) {
        DISPLAY_TYPE = SCREENMODE1;
        modmode = true;
        ResetDisplay();
    }
    //
    memset((void *)WriteBuf, 0, ScreenSize);

#ifdef PICOMITEVGA
#ifdef rp2350
#ifdef HDMI
    if (DISPLAY_TYPE == SCREENMODE3)
        for (int i = 0; i < 16; i++) map16[i] = remap[i] = RGB555(MAP16DEF[i]);
#else
    if (DISPLAY_TYPE == SCREENMODE3)
        for (int i = 0; i < 16; i++) map16[i] = remap[i] = i;
#endif
#endif
#endif

#endif
// #ifndef USBKEYBOARD
//     if (mouse0 == false && Option.MOUSE_CLOCK)
//         initMouse0(0);  // see if there is a mouse to initialise
// #endif
    if (OPTION_COLOUR_CODE) {
        gui_fcolour = WHITE;
        gui_bcolour = BLACK;
    }
    if (OPTION_DISPLAY_CONSOLE == true && HRes / gui_font_width < 32) error_throw_legacy("Font is too large");
    // if (OPTION_DISPLAY_TYPE >= VIRTUAL && OPTION_DISPLAY_TYPE < NEXTGEN && WriteBuf)
    //     FreeMemorySafe((void **)&WriteBuf);
    if (cmdfile) {
        // ClearVars(0, true);
        ClearVars(0);
        // ClearRuntime(true);
        ClearRuntime();
    }
    if (HRes == 640 || HRes == 512 || HRes == 848 || HRes == 720) {
//        SetFont(1);
//        PromptFont = 1;
    }
    if (modmode) {
#ifdef HDMI
        if (FullColour || MediumRes) {
#endif
//            SetFont(1);
//            PromptFont = 1;
#ifdef HDMI
        } else {
            SetFont(2 << 4 | 1);
            PromptFont = 2 << 4 | 1;
        }
#endif
    }
#ifdef PICOMITEWEB
    cleanserver();
#endif
    multilinecomment = false;
    EdBuff = GetTempMemory(EDIT_BUFFER_SIZE);
    // char buff[STRINGSIZE * 2] = {0};
    *EdBuff = 0;

    VHeight = mmb_options.height - 2;
    VWidth = mmb_options.width;
    edx = edy = curx = cury = y = x = tempx = 0;
    txtp = EdBuff;
    *tknbuf = 0;
    if (filename == NULL) {
        fromp = ProgMemory;
        p = EdBuff;
        nbrlines = 0;
        while (*fromp != 0xff) {
            if (*fromp == T_NEWLINE) {
                if (StartEditPoint >= ProgMemory) {
                    if (StartEditPoint == fromp) {
                        y = nbrlines;  // we will start editing at this line
                        tempx = x = StartEditChar;
                        txtp = p + StartEditChar;
                    }
                } else {
                    // if (StartEditPoint == (char *) nbrlines) {
                    //     y = nbrlines;
                    //     tempx = x = StartEditChar;
                    //     txtp = p + StartEditChar;
                    // }
                }
                nbrlines++;
                // if (OPTION_CONTINUATION) {
                //     fromp = llist(buff, fromp);  // otherwise expand the line
                //     if (!(nbrlines == 1 && buff[0] == '\'' && buff[1] == '#')) {
                //         nbrlines += format_string(&buff[0], mmb_options.width);
                //         strcat((char *)p, buff);
                //         p += strlen((char *)buff);
                //         *p++ = '\n';
                //         *p = 0;
                //     } else
                //         nbrlines = 0;
                // } else {
                    fromp = llist(p, fromp);
                    if (!(nbrlines == 1 && p[0] == '\'' && p[1] == '#')) {
                        p += strlen((char *)p);
                        *p++ = '\n';
                        *p = 0;
                    } else
                        nbrlines = 0;
                // }
            }
            // finally, is it the end of the program?
            if (fromp[0] == 0 || fromp[0] == 0xff) break;
        }
    } else {
        //        char *fname = (char *)filename;
        char c;
        int fsize;
        //        strcpy(name,fname);
        if (!file_exists(filename)) {
            if (strchr(filename, '.') == NULL) strcat(filename, ".bas");
        }
        // if (!fstrstr(filename, ".bas")) OPTION_COLOUR_CODE = 0;
        if (file_exists(filename)) {
            int fnbr1;
            fnbr1 = file_find_free();
            ON_FAILURE_ERROR(file_open(filename, "rb", fnbr1));
            // BasicFileOpen(filename, fnbr1, FA_READ);
            fsize = file_size(fnbr1);
            // if (filesource[fnbr1] != FLASHFILE)
            //     fsize = f_size(FileTable[fnbr1].fptr);
            // else
            //     fsize = lfs_file_size(&lfs, FileTable[fnbr1].lfsptr);
            if (fsize > EDIT_BUFFER_SIZE - 10) error_throw_legacy("Out of memory");
            p = EdBuff;
            // char *q = (char *)EdBuff;
            do {  // while waiting for the end of file
                c = file_getc(fnbr1);
                if (c == '\n') {
                    nbrlines++;
                    // if (OPTION_CONTINUATION) {
                    //     strcpy(buff, q);
                    //     nbrlines += format_string(&buff[0], Option.Width);
                    //     strcpy(q, buff);
                    //     p = (char *)q + strlen(q);
                    //     q = (char *)p;
                    // }
                }
                if (c == '\r') continue;
                *p++ = c;
            } while (!file_eof(fnbr1));
            p++;
            ON_FAILURE_ERROR(file_close(fnbr1));
        }
        txtp = EdBuff;
        tempx = x = 0;
    }
    if (nbrlines == 0) nbrlines++;
    if (p > EdBuff) --p;
    *p = 0;  // erase the last line terminator
    // Only setterminal if editor requires is bigger than 80*24
    if (mmb_options.width > SCREENWIDTH || mmb_options.height > SCREENHEIGHT) {
        setterminal(
            (mmb_options.height > SCREENHEIGHT) ? mmb_options.height : SCREENHEIGHT,
            (mmb_options.width > SCREENWIDTH) ? mmb_options.width : SCREENWIDTH);  // or height is > 24
    }
    PrintString("\033[?1000h");         // Tera Term turn on mouse click report in VT200 mode
    PrintString("\0337\033[2J\033[H");  // vt100 clear screen and home cursor
    MX470Display(DISPLAY_CLS);          // clear screen on the MX470 display only
    pmeditor_set_cursor(0, 0);
    PrintFunctKeys(EDIT);

    if (nbrlines > VHeight) {
        edy = y - VHeight / 2;  // edy is the line displayed at the top
        if (edy < 0) edy = 0;   // compensate if we are near the start
        y = y - edy;            // y is the line on the screen
    }
    // if (cmdfile) m_alloc(M_VAR);  // clean up clipboard usage
    FullScreenEditor(x, y, filename, EDIT_BUFFER_SIZE);
    if (cmdfile)
        memset(tknbuf, 0,
               STRINGSIZE);  // zero this so that nextstmt is pointing to the end of program
    MMCharPos = 0;
}

/*  @endcond */
// void cmd_edit(void) { edit(cmdline, true); }
// void cmd_editfile(void) { edit(cmdline, FALSE); }
int find_longest_line_length(const char *text, int *linein) {
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
    int fnbr;

    if (file_exists(filename)) {
        char backup[FF_MAX_LFN];
        strcpy(backup, filename);
        strcat(backup, ".bak");
        fnbr = file_find_free();
        ON_FAILURE_RETURN(file_open(filename, "rb", fnbr));
        // BasicFileOpen(fname, fnbr1, FA_READ);
        const int fnbr_bak = file_find_free();
        ON_FAILURE_RETURN(file_open(backup, "wb", fnbr_bak));
        // BasicFileOpen(backup, fnbr_bak, FA_WRITE | FA_CREATE_ALWAYS);
        while (!file_eof(fnbr)) {  // while waiting for the end of file
            file_putc(fnbr_bak, file_getc(fnbr));
        }
        file_close(fnbr);
        file_close(fnbr_bak);
    }

    fnbr = file_find_free();
    ON_FAILURE_RETURN(file_open(filename, "wb", fnbr));
    // BasicFileOpen(fname, fnbr1, FA_WRITE | FA_CREATE_ALWAYS);
    char *p = EdBuff;
    // if (OPTION_CONTINUATION) {
    //     char *q = p;
    //     while (*p) {
    //         if (*p == OPTION_CONTINUATION && p[1] == '\n')
    //             p += 2;  // step over the continuation characters
    //         else
    //             *q++ = *p++;
    //     }
    //     *q = 0;
    //     p = EdBuff;
    // }
    do {
        const char ch = *p++;
        if (ch == '\n') file_putc(fnbr, '\r');
        file_putc(fnbr, ch);
    } while (*p);

    return file_close(fnbr);
}

/*
 * @cond
 * The following section will be excluded from the documentation.
 */
// #ifndef PICOMITE
// #ifndef PICOMITEWEB
// static short lastx1 = 9999, lasty1 = 9999;
// static uint16_t lastfc, lastbc;
// static bool leftpushed = false, rightpushed = false, middlepushed = false;
// #endif
// #endif
void FullScreenEditor(int xx, int yy, const char *fname, int edit_buff_size) {
    int c = -1, i;
    char buf[MAXCLIP + 2], clipboard[MAXCLIP + 2];
    char *p, *tp; //, BreakKeySave;
    char currdel = 0, nextdel = 0, lastdel = 0;
    char multi = false;
#ifdef PICOMITEVGA
    int fontinc = gui_font_width / 8;  // use to decide where to position mouse cursor
    int OptionY_TILESave;
    int ytileheightsave;
    ytileheightsave = ytileheight;
    OptionY_TILESave = Y_TILE;
    if (!OPTION_COLOUR_CODE)
        ytileheight = 16;
    else {
        ytileheight = gui_font_height;
        Y_TILE = VRes / ytileheight;
        if (VRes % ytileheight) Y_TILE++;
    }
#else
    // char RefreshSave = Option.Refresh;
    // Option.Refresh = 0;
#endif
    printScreen();  // draw the screen
    pmeditor_set_cursor(xx, yy);
    drawstatusline = true;
    char lastkey = 0;
    int y, statuscount;
    clipboard[0] = 0;
    buf[0] = 0;
    insert = true;
    TextChanged = false;
    // BreakKeySave = BreakKey;
    // BreakKey = 0;
    while (1) {
        statuscount = 0;
        do {
#ifndef PICOMITE
#ifndef PICOMITEWEB
            c = -1;
#if 0
#ifdef USBKEYBOARD
            if (HID[1].Device_type == 2 && DISPLAY_TYPE == SCREENMODE1) {
#else
            if (mouse0 && DISPLAY_TYPE == SCREENMODE1) {
#endif
                if (!nunstruct[2].L) leftpushed = false;
                if (!nunstruct[2].R) rightpushed = false;
                if (!nunstruct[2].C) middlepushed = false;
                if (nunstruct[2].y1 != lasty1 || nunstruct[2].x1 != lastx1) {
                    if (lastx1 != 9999) {
#ifdef HDMI
                        if (FullColour) {
#endif
                            for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                                tilefcols[lasty1 * X_TILE + i] = lastfc;
                            for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                                tilebcols[lasty1 * X_TILE + i] = lastbc;
#ifdef HDMI
                        } else {
                            for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                                tilefcols_w[lasty1 * X_TILE + i] = lastfc;
                            for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                                tilebcols_w[lasty1 * X_TILE + i] = lastbc;
                        }
#endif
                    }
                    lastx1 = nunstruct[2].x1;
                    lasty1 = nunstruct[2].y1;
                    if (lasty1 >= VHeight) lasty1 = VHeight - 1;
#ifdef HDMI
                    if (FullColour) {
#endif
                        lastfc = tilefcols[lasty1 * X_TILE + lastx1 * fontinc];
                        lastbc = tilebcols[lasty1 * X_TILE + lastx1 * fontinc];
#ifdef HDMI
                    } else {
                        lastfc = tilefcols_w[lasty1 * X_TILE + lastx1 * fontinc];
                        lastbc = tilebcols_w[lasty1 * X_TILE + lastx1 * fontinc];
                    }
                    if (FullColour) {
                        for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                            tilefcols[lasty1 * X_TILE + i] = RGB555(RED);
                        for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                            tilebcols[lasty1 * X_TILE + i] = RGB555(WHITE);
                    } else {
                        for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                            tilefcols_w[lasty1 * X_TILE + i] = RGB332(RED);
                        for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                            tilebcols_w[lasty1 * X_TILE + i] = RGB332(WHITE);
                    }
#else
                    for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                        tilefcols[lasty1 * X_TILE + i] = RGB121pack(RED);
                    for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                        tilebcols[lasty1 * X_TILE + i] = RGB121pack(WHITE);
#endif
                }
                if ((nunstruct[2].L && leftpushed == false && rightpushed == false &&
                     middlepushed == false) ||
                    (nunstruct[2].R && leftpushed == false && rightpushed == false &&
                     middlepushed == false) ||
                    (nunstruct[2].C && leftpushed == false && rightpushed == false &&
                     middlepushed == false)) {
                    if (nunstruct[2].L)
                        leftpushed = true;
                    else if (nunstruct[2].R)
                        rightpushed = true;
                    else
                        middlepushed = true;
                    if (lastx1 >= 0 && lastx1 < VWidth && lasty1 >= 0 &&
                        lasty1 < VHeight) {  // c == ' ' means mouse down and no shift, ctrl, etc
                        ShowCursor(false);
                        // first position on the y axis
                        while (*txtp != 0 &&
                               lasty1 > cury)  // assume we have to move down the screen
                            if (*txtp++ == '\n') cury++;
                        while (txtp != EdBuff &&
                               lasty1 < cury)  // assume we have to move up the screen
                            if (*--txtp == '\n') cury--;
                        while (txtp != EdBuff && *(txtp - 1) != '\n')
                            txtp--;  // move to the beginning of the line
                        for (curx = 0; curx < lastx1 && *txtp && *txtp != '\n'; curx++)
                            txtp++;  // now position on the x axis
                        PositionCursor(txtp);
                        PrintStatus();
                        ShowCursor(true);
#ifdef HDMI
                        if (FullColour) {
#endif
                            // for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                            //     tilefcols[lasty1 * X_TILE + i] = lastfc;
                            // for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                            //     tilebcols[lasty1 * X_TILE + i] = lastbc;
#ifdef HDMI
                        } else {
                            for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                                tilefcols_w[lasty1 * X_TILE + i] = lastfc;
                            for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
                                tilebcols_w[lasty1 * X_TILE + i] = lastbc;
                        }
#endif
                    }
                    if (rightpushed == true) {
                        c = F4;
                    } else if (middlepushed == true) {
                        c = F5;
                    }
                }
            }
            ShowCursor(true);
            if (!((rightpushed == true && c == F4) || (middlepushed == true && c == F5)))
#else
            ShowCursor(true);
#endif
#else
            ShowCursor(true);
#endif
#endif
                c = MMInkey();

            if (statuscount++ == 5000) PrintStatus();
        } while (c == -1);
        ShowCursor(false);

        if (drawstatusline) PrintFunctKeys(EDIT);
        drawstatusline = false;
        if (c == TAB) {
            strcpy((char *)buf, "        ");
            buf[mmb_options.tab - ((edx + curx) % mmb_options.tab)] = 0;
        } else {
            buf[0] = c;
            buf[1] = 0;
        }
        do {
            // if (buf[0] == BreakKeySave)
            //     buf[0] = ESC;  // if the user tried to break turn it into an escape
            switch (buf[0]) {
                case '\r':
                case '\n':  // first count the spaces at the beginning of the line
                    if (txtp != EdBuff &&
                        (*txtp == '\n' ||
                         *txtp == 0)) {  // we only do this if we are at the end of the line
                        for (tp = txtp - 1, i = 0; *tp != '\n' && tp >= EdBuff; tp--)
                            if (*tp != ' ')
                                i = 0;  // not a space
                            else
                                i++;  // potential space at the start
                        if (tp == EdBuff && *tp == ' ')
                            i++;  // correct for a counting error at the start of the buffer
                        if (buf[1] != 0)
                            i = 0;  // do not insert spaces if buffer too small or has something in
                                    // it
                        else
                            buf[i + 1] = 0;        // make sure that the end of the buffer is zeroed
                        while (i) buf[i--] = ' ';  // now, place our spaces in the typeahead buffer
                    }
                    if (!editInsertChar('\n', &multi, edit_buff_size)) break;  // insert the newline
                    TextChanged = true;
                    nbrlines++;
                    if (!(cury < VHeight - 1))  // if we are NOT at the bottom
                        edy++;                  // otherwise scroll
                    printScreen();              // redraw everything
                    PositionCursor(txtp);
                    break;

                case CTRLKEY('E'):
                case UP:
                    if (cury == 0 && edy == 0) break;
                    if (*txtp == '\n')
                        txtp--;  // step back over the terminator if we are right at the end of the
                                 // line
                    while (txtp != EdBuff && *txtp != '\n')
                        txtp--;  // move to the beginning of the line
                    if (txtp != EdBuff) {
                        txtp--;  // step over the terminator to the end of the previous line
                        while (txtp != EdBuff && *txtp != '\n')
                            txtp--;                 // move to the beginning of that line
                        if (*txtp == '\n') txtp++;  // and position at the start
                    }
                    for (i = 0; i < edx + tempx && *txtp != 0 && *txtp != '\n';
                         i++, txtp++);  // move the cursor to the column

                    if (cury > 2 || edy == 0) {  // if we are more that two lines from the top
                        if (cury > 0) pmeditor_set_cursor(i, cury - 1);  // just move the cursor up
                    } else if (edy > 0) {  // if we are two lines or less from the top
                        curx = i;
                        ScrollDown();
                    }
                    PositionCursor(txtp);
                    break;

                case CTRLKEY('X'):
                case DOWN:
                    p = txtp;
                    while (*p != 0 && *p != '\n') p++;  // move to the end of this line
                    if (*p == 0) break;                 // skip if it is at the end of the file
                    p++;  // step over the line terminator to the start of the next line
                    for (i = 0; i < edx + tempx && *p != 0 && *p != '\n';
                         i++, p++);  // move the cursor to the column
                    txtp = p;

                    if (cury < VHeight - 3 || edy + VHeight == nbrlines) {
                        if (cury < VHeight - 1) pmeditor_set_cursor(i, cury + 1);
                    } else if (edy + VHeight < nbrlines) {
                        curx = i;
                        Scroll();
                    }
                    PositionCursor(txtp);
                    break;

                case CTRLKEY('S'):
                case LEFT:
                    if (txtp == EdBuff) break;
                    if (*(txtp - 1) == '\n') {  // if at the beginning of the line wrap around
                        buf[1] = UP;
                        buf[2] = END;
                        buf[3] = 1;
                        buf[4] = 0;
                    } else {
                        txtp--;
                        PositionCursor(txtp);
                    }
                    break;

                case CTRLKEY('D'):
                case RIGHT:
                    if (*txtp == '\n') {  // if at the end of the line wrap around
                        buf[1] = HOME;
                        buf[2] = DOWN;
                        buf[3] = 0;
                        break;
                    }
                    if (curx >= VWidth) {
                        editDisplayMsg((char *)" LINE IS TOO LONG ");
                        break;
                    }

                    if (*txtp != 0) txtp++;  // now we can move the cursor
                    PositionCursor(txtp);
                    break;

                // backspace
                case BKSP:
                    if (txtp == EdBuff) break;
                    if (*(txtp - 1) == '\n') {  // if at the beginning of the line wrap around
                        buf[1] = UP;
                        buf[2] = END;
                        buf[3] = DEL;
                        buf[4] = 0;
                        break;
                    }
                    // find how many spaces are between the cursor and the start of the line
                    for (p = txtp - 1; *p == ' ' && p != EdBuff; p--);
                    if ((p == EdBuff || *p == '\n') && txtp - p > 1) {
                        i = txtp - p - 1;
                        // we have have the number of continuous spaces between the cursor and the
                        // start of the line now figure out the number of backspaces to the nearest
                        // tab stop

                        i = (i % mmb_options.tab);
                        if (i == 0) i = mmb_options.tab;
                        // load the corresponding number of deletes in the type ahead buffer
                        buf[i + 1] = 0;
                        while (i--) {
                            buf[i + 1] = DEL;
                            txtp--;
                        }
                        // and let the delete case take care of deleting the characters
                        PositionCursor(txtp);
                        break;
                    }
                    // this is just a normal backspace (not a tabbed backspace)
                    txtp--;
                    PositionCursor(txtp);
                    // fall through to delete the char
                    CASE_FALLTHROUGH;

                case CTRLKEY(']'):
                case DEL:
                    if (*txtp == 0) break;
                    p = txtp;
                    c = *p;
                    currdel = *p;
                    if (p != EdBuff + edit_buff_size - 1)
                        nextdel = p[1];
                    else
                        nextdel = 0;
                    if (p != EdBuff) {
                        lastdel = *(--p);
                        p++;
                    } else
                        lastdel = 0;
                    while (*p) {
                        p[0] = p[1];
                        p++;
                    }
                    if (c == '\n') {
                        printScreen();
                        nbrlines--;
                    } else
                        printLine(edy + cury);
                    TextChanged = true;
                    PositionCursor(txtp);
                    if (currdel == '/' && nextdel == '*' && OPTION_COLOUR_CODE) printScreen();
                    if (currdel == '*' && nextdel == '/' && OPTION_COLOUR_CODE) printScreen();
                    if (currdel == '/' && lastdel == '*' && OPTION_COLOUR_CODE) printScreen();
                    if (currdel == '*' && lastdel == '/' && OPTION_COLOUR_CODE) printScreen();
                    break;

                case CTRLKEY('N'):
                case INSERT:
                    insert = !insert;
                    break;

                case CTRLKEY('U'):
                case HOME:
                    if (txtp == EdBuff) break;
                    if (lastkey == HOME || lastkey == CTRLKEY('U')) {
                        edx = edy = curx = cury = 0;
                        txtp = EdBuff;
                        PrintString("\033[2J\033[H");  // vt100 clear screen and home cursor
                        MX470Display(DISPLAY_CLS);     // clear screen on the MX470 display only
                        printScreen();
                        PrintFunctKeys(EDIT);
                        PositionCursor(txtp);
                        break;
                    }
                    if (*txtp == '\n')
                        txtp--;  // step back over the terminator if we are right at the end of the
                                 // line
                    while (txtp != EdBuff && *txtp != '\n')
                        txtp--;                 // move to the beginning of the line
                    if (*txtp == '\n') txtp++;  // skip if no more lines above this one
                    PositionCursor(txtp);
                    break;

                case CTRLKEY('K'):
                case END:
                    if (*txtp == 0) break;                            // already at the end
                    if (lastkey == END || lastkey == CTRLKEY('K')) {  // jump to the end of the file
                        i = 0;
                        p = txtp = EdBuff;
                        while (*txtp != 0) {
                            if (*txtp == '\n') {
                                p = txtp + 1;
                                i++;
                            }
                            txtp++;
                        }

                        if (i >= VHeight) {
                            edy = i - VHeight + 1;
                            printScreen();
                            cury = VHeight - 1;
                        } else {
                            cury = i;
                        }
                        txtp = p;
                        curx = 0;
                    }

                    while (curx < VWidth && *txtp != 0 && *txtp != '\n') {
                        txtp++;
                        PositionCursor(txtp);
                    }
                    if (curx > VWidth) editDisplayMsg(" LINE IS TOO LONG ");
                    break;

                case CTRLKEY('P'):
                case PUP:
                    if (edy == 0) {     // if we are already showing the top of the text
                        buf[1] = HOME;  // force the editing point to the start of the text
                        buf[2] = HOME;
                        buf[3] = 0;
                        break;
                    } else if (edy >= VHeight - 1) {  // if we can scroll a full screenfull
                        i = VHeight + 1;
                        edy -= VHeight;
                    } else {  // if it is less than a full screenfull
                        i = edy + 1;
                        edy = 0;
                    }
                    while (i--) {
                        if (*txtp == '\n')
                            txtp--;  // step back over the terminator if we are right at the end of
                                     // the line
                        while (txtp != EdBuff && *txtp != '\n')
                            txtp--;                 // move to the beginning of the line
                        if (txtp == EdBuff) break;  // skip if no more lines above this one
                    }
                    if (txtp != EdBuff) txtp++;  // and position at the start of the line
                    for (i = 0; i < edx + curx && *txtp != 0 && *txtp != '\n';
                         i++, txtp++);  // move the cursor to the column
                    printScreen();
                    PositionCursor(txtp);
                    break;

                case CTRLKEY('L'):
                case PDOWN:
                    if (nbrlines <=
                        edy + VHeight + 1) {  // if we are already showing the end of the text
                        buf[1] = END;         // force the editing point to the end of the text
                        buf[2] = END;
                        buf[3] = 0;
                        break;  // cursor to the top line
                    } else if (nbrlines - edy - VHeight >=
                               VHeight) {  // if we can scroll a full screenfull
                        edy += VHeight;
                        i = VHeight;
                    } else {  // if it is less than a full screenfull
                        i = nbrlines - VHeight - edy;
                        edy = nbrlines - VHeight;
                    }
                    if (*txtp == '\n') i--;  // compensate if we are right at the end of a line
                    while (i--) {
                        if (*txtp == '\n')
                            txtp++;  // step over the terminator if we are right at the start of the
                                     // line
                        while (*txtp != 0 && *txtp != '\n') txtp++;  // move to the end of the line
                        if (*txtp == 0) break;  // skip if no more lines after this one
                    }
                    if (txtp != EdBuff) txtp++;  // and position at the start of the line
                    for (i = 0; i < edx + curx && *txtp != 0 && *txtp != '\n';
                         i++, txtp++);  // move the cursor to the column
                    // y = cury;
                    printScreen();
                    PositionCursor(txtp);
                    break;

                // Abort without saving
                case ESC:
                    // Wait 50ms to see if anything more is coming.
                    mmtime_sleep_ns(MILLISECONDS_TO_NANOSECONDS(50));
                    routinechecks();
                    if (MMInkey() == '[' && MMInkey() == 'M') {
                        // received escape code for Tera Term reporting a mouse click or scroll
                        // wheel movement
                        int c, x, y;
                        c = MMInkey();
                        x = MMInkey() - '!';
                        y = MMInkey() - '!';
                        if (c == 'e' || c == 'a') {  // Tera Term - SHIFT + mouse-wheel-rotate-down
                            buf[1] = UP;
                            buf[2] = 0;
                        } else if (c == 'd' ||
                                   c == '`') {  // Tera Term - SHIFT + mouse-wheel-rotate-up
                            buf[1] = DOWN;
                            buf[2] = 0;
                        } else if (c == ' ' && x >= 0 && x < VWidth && y >= 0 &&
                                   y < VHeight) {  // c == ' ' means mouse down and no shift, ctrl,
                                                   // etc
                            // first position on the y axis
                            while (*txtp != 0 &&
                                   y > cury)  // assume we have to move down the screen
                                if (*txtp++ == '\n') cury++;
                            while (txtp != EdBuff &&
                                   y < cury)  // assume we have to move up the screen
                                if (*--txtp == '\n') cury--;
                            while (txtp != EdBuff && *(txtp - 1) != '\n')
                                txtp--;  // move to the beginning of the line
                            for (curx = 0; curx < x && *txtp && *txtp != '\n'; curx++)
                                txtp++;  // now position on the x axis
                            PositionCursor(txtp);
                        }
                        break;
                    }
                    // this must be an ordinary escape (not part of an escape code)
                    if (TextChanged) {
                        GetInputString((char *)"Exit and discard all changes (Y/N): ");
                        if (toupper(*inpbuf) != 'Y') break;
                    }
                    // fall through to the normal exit
                    CASE_FALLTHROUGH;

                case CTRLKEY('Q'):  // Save and exit
                case F1:            // Save and exit
                case CTRLKEY('W'):  // Save, exit and run
                case F2:            // Save, exit and run
                                    //                            if(OPTION_CONTINUATION){
                    int line = 0;
                    int i = find_longest_line_length((char *)EdBuff, &line);
                    if (i > 255) {
                        char buff[20] = {};
                        sprintf(buff, " LINE %d TOO LONG", line);
                        editDisplayMsg(buff);
                        break;
                    }
                    //                            }
                    c = buf[0];
                    PrintString(
                        "\033[?1000l");  // Tera Term turn off mouse click report in vt200 mode
                    PrintString("\0338\033[2J\033[H");  // vt100 clear screen and home cursor
                    gui_fcolour = GUI_C_NORMAL;
                    pmeditor_highlight(kHighlightNormal);
                    // gui_fcolour = PromptFC;
                    // gui_bcolour = PromptBC;
                    MX470Cursor(0, 0);          // home the cursor
                    MX470Display(DISPLAY_CLS);  // clear screen on the MX470 display only
                    PrintString("\033[0m");
//                    BreakKey = BreakKeySave;
                    // OPTION_COLOUR_CODE = optioncolourcodesave;
#ifdef PICOMITEVGA
                    editactive = 0;
                    Y_TILE = OptionY_TILESave;
                    ytileheight = ytileheightsave;
                    if (modmode) {
                        DISPLAY_TYPE = oldmode;
                        ResetDisplay();
                        SetFont(oldfont);
                        PromptFont = oldfont;
                        MX470Display(DISPLAY_CLS);  // clear screen on the MX470 display only
                    }
#ifdef HDMI
                    while (v_scanline != 0) {
                    }
                    if (DISPLAY_TYPE == SCREENMODE5)
                        for (int i = 0; i < 256; i++) map256[i] = remap[i] = RGB555(MAP256DEF[i]);
                    else if (FullColour)
                        for (int i = 0; i < 16; i++) {
                            map16[i] = remap[i] = RGB555(MAP16DEF[i]);
                            map16pairs[i] = map16[i] | (map16[i] << 16);
                        }
                    else if (DISPLAY_TYPE == SCREENMODE3)
                        for (int i = 0; i < 16; i++) {
                            map16s[i] = RGB332(MAP16DEF[i]);
                            map16d[i] = remap[i] = RGB332(MAP16DEF[i]) | (RGB332(MAP16DEF[i]) << 8);
                        }
                    else if (DISPLAY_TYPE == SCREENMODE2)
                        for (int i = 0; i < 16; i++)
                            map16q[i] = remap[i] =
                                RGB332(MAP16DEF[i]) | (RGB332(MAP16DEF[i]) << 8) |
                                (RGB332(MAP16DEF[i]) << 16) | (RGB332(MAP16DEF[i]) << 24);
                    if (DISPLAY_TYPE == SCREENMODE1) {
                        if (FullColour) {
                            tilefcols = (uint16_t *)((uint32_t)FRAMEBUFFER + (MODE1SIZE * 3));
                            tilebcols = (uint16_t *)((uint32_t)FRAMEBUFFER + (MODE1SIZE * 3) +
                                                     (MODE1SIZE >> 1));
                            X_TILE = MODE_H_ACTIVE_PIXELS / 8;
                            Y_TILE = MODE_V_ACTIVE_LINES / 8;
                            for (int x = 0; x < X_TILE; x++) {
                                for (int y = 0; y < Y_TILE; y++) {
                                    tilefcols[y * X_TILE + x] = RGB555(Option.DefaultFC);
                                    tilebcols[y * X_TILE + x] = RGB555(Option.DefaultBC);
                                }
                            }
                        } else {
                            tilefcols_w = (uint8_t *)DisplayBuf + MODE1SIZE;
                            tilebcols_w = tilefcols_w +
                                          (MODE_H_ACTIVE_PIXELS / 8) *
                                              (MODE_V_ACTIVE_LINES / 8);  // minimum tilesize is 8x8
                            memset(tilefcols_w, RGB332(Option.DefaultFC),
                                   (MODE_H_ACTIVE_PIXELS / 8) * (MODE_V_ACTIVE_LINES / 8) *
                                       sizeof(uint8_t));
                            memset(tilebcols_w, RGB332(Option.DefaultBC),
                                   (MODE_H_ACTIVE_PIXELS / 8) * (MODE_V_ACTIVE_LINES / 8) *
                                       sizeof(uint8_t));
                            X_TILE = MODE_H_ACTIVE_PIXELS / 8;
                            Y_TILE = MODE_V_ACTIVE_LINES / ytileheight;
                        }
                    }
#endif
#else
//                    Option.Refresh = RefreshSave;
#endif
                    // if (c != ESC && TextChanged && fname == NULL) SaveToProgMemory();
                    if (c != ESC && TextChanged && fname) {
                        ON_FAILURE_ERROR(pmeditor_save_file(fname));
                    }
//                     if (cmdfile == false) {
// //                        RestoreContext(false);
//                         return;
//                     }
                    if (c == ESC || c == CTRLKEY('Q') || c == F1 || fname) {
                        cmdline = NULL;
                        // do_end(false);
                        longjmp(mark, JMP_END);  // jump back to the input prompt
                    }
                    // this must be save, exit and run.  We have done the first two, now do the run
                    // part.
                    ClearRuntime();
                    //ClearRuntime(true);
                    //                            WatchdogSet = false;
                    PrepareProgram(true);
                    // Create a global constant MM.CMDLINE$ containing the empty string.
                    //                            (void) findvar((char *)"MM.CMDLINE$",
                    //                            V_FIND | V_DIM_VAR | T_CONST);
                    // if (Option.LIBRARY_FLASH_SIZE == MAX_PROG_SIZE)
                    //     ExecuteProgram(LibMemory);  // run anything that might be in the library
                    if (*ProgMemory != T_NEWLINE) return;  // no program to run
// #ifdef PICOMITEWEB
// #ifndef PICOMITE
// // #ifndef PICOMITEWEB
// // #ifdef USBKEYBOARD
// //         if (HID[1].Device_type == 2 && DISPLAY_TYPE == SCREENMODE1) {
// // #else

//                     cleanserver();
// #endif
                    nextstmt = ProgMemory;
#ifdef USBKEYBOARD
                    clearrepeat();
#endif
                    return;

                // Search
                case CTRLKEY('R'):
                case F3:
                    GetInputString((char *)"Find (Use SHIFT-F3 to repeat): ");
                    if (*inpbuf == 0 || *inpbuf == ESC) break;
                    if (!(*inpbuf == 0xb3 || *inpbuf == F3)) strcpy((char *)tknbuf, (char *)inpbuf);
                    // fall through

                case CTRLKEY('G'):
                case 0xB3:  // SHIFT-F3
                    p = txtp;
                    if (*p == 0) p = EdBuff - 1;
                    i = strlen((char *)tknbuf);
                    while (1) {
                        p++;
                        if (p == txtp) break;
                        if (*p == 0) p = EdBuff;
                        if (p == txtp) break;
                        if (memcmp(p, tknbuf, i) == 0) break;
                    }
                    if (p == txtp) {
                        editDisplayMsg(" NOT FOUND ");
                        break;
                    }
                    for (y = 0, txtp = EdBuff; txtp != p;
                         txtp++) {  // find the line and column of the string
                        if (*txtp == '\n') {
                            y++;  // y is the line
                        }
                    }
                    edy = y - VHeight / 2;  // edy is the line displayed at the top
                    if (edy < 0) edy = 0;   // compensate if we are near the start
                    printScreen();
                    PositionCursor(txtp);
                    // pmeditor_set_cursor(x, y);
                    break;

                // Mark
                case CTRLKEY('T'):
                case F4:
                    MarkMode(clipboard, &buf[1]);
                    printScreen();
                    PrintFunctKeys(EDIT);
                    PositionCursor(txtp);
                    break;

                case CTRLKEY('Y'):
                case CTRLKEY('V'):
                case F5:
                    if (*clipboard == 0) {
                        editDisplayMsg(" CLIPBOARD IS EMPTY ");
                        break;
                    }
                    for (i = 0; clipboard[i]; i++) buf[i + 1] = clipboard[i];
                    buf[i + 1] = 0;
                    break;

                // F6 to F12 - Normal function keys
                case F6:
                case F7:
                case F8:
                case F9:
                case F10:
                case F11:
                case F12:
                    break;

                // a normal character
                default:
                    c = buf[0];
                    if (c < ' ' || c > '~') break;  // make sure that this is valid
                    if (curx >= VWidth) {
                        editDisplayMsg(" LINE IS TOO LONG ");
                        break;
                    }
                    TextChanged = true;
                    if (insert || *txtp == '\n' || *txtp == 0) {
                        if (!editInsertChar(c, &multi, edit_buff_size)) break;  // insert it
                    } else
                        *txtp++ = c;  // or just overtype
                    printLine(edy +
                              cury);  // redraw the whole line so that colour coding will occur
                    PositionCursor(txtp);
                    // pmeditor_set_cursor(x, cury);
                    tempx = cury;  // used to track the preferred cursor position
                    if (multi && OPTION_COLOUR_CODE) printScreen();
                    break;
            }
            lastkey = buf[0];
            if (buf[0] != UP && buf[0] != DOWN && buf[0] != CTRLKEY('E') && buf[0] != CTRLKEY('X'))
                tempx = curx;
            buf[MAXCLIP + 1] = 0;
            for (i = 0; i < MAXCLIP + 1; i++)
                buf[i] = buf[i + 1];  // suffle down the buffer to get the next char
        } while (*buf);
    }
}

/*******************************************************************************************************************
  UTILITY FUNCTIONS USED BY THE FULL SCREEN EDITOR
*******************************************************************************************************************/

void PositionCursor(char *curp) {
    int ln, col;
    char *p;

    for (p = EdBuff, ln = col = 0; p < curp; p++) {
        if (*p == '\n') {
            ln++;
            col = 0;
        } else
            col++;
    }
    if (ln < edy || ln >= edy + VHeight) return;
    pmeditor_set_cursor(col, ln - edy);
}

// mark mode
// implement the mark mode (when the user presses F4)
void MarkMode(char *cb, char *buf) {
    char *p, *mark, *oldmark;
    int c = -1, x, y, i, oldx, oldy, txtpx, txtpy, errmsg = false;
#ifdef PICOMITEVGA
    int fontinc = gui_font_width / 8;
#endif
    PrintFunctKeys(MARK);
    oldmark = mark = txtp;
    txtpx = oldx = curx;
    txtpy = oldy = cury;
    while (1) {
        c = -1;
// #ifndef PICOMITE
// #ifndef PICOMITEWEB
// #ifdef USBKEYBOARD
//         if (HID[1].Device_type == 2 && DISPLAY_TYPE == SCREENMODE1) {
// #else
//         if (mouse0 && DISPLAY_TYPE == SCREENMODE1) {
// #endif
//             if (!nunstruct[2].L) leftpushed = false;
//             if (!nunstruct[2].R) rightpushed = false;
//             if (!nunstruct[2].C) middlepushed = false;
//             if (nunstruct[2].y1 != lasty1 || nunstruct[2].x1 != lastx1) {
//                 if (lastx1 != 9999) {
// #ifdef HDMI
//                     if (FullColour) {
// #endif
//                         for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
//                             tilefcols[lasty1 * X_TILE + i] = lastfc;
//                         for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
//                             tilebcols[lasty1 * X_TILE + i] = lastbc;
// #ifdef HDMI
//                     } else {
//                         for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
//                             tilefcols_w[lasty1 * X_TILE + i] = lastfc;
//                         for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
//                             tilebcols_w[lasty1 * X_TILE + i] = lastbc;
//                     }
// #endif
//                 }
//                 lastx1 = nunstruct[2].x1;
//                 lasty1 = nunstruct[2].y1;
//                 if (lasty1 >= VHeight) lasty1 = VHeight - 1;
// #ifdef HDMI
//                 if (FullColour) {
// #endif
//                     lastfc = tilefcols[lasty1 * X_TILE + lastx1 * fontinc];
//                     lastbc = tilebcols[lasty1 * X_TILE + lastx1 * fontinc];
// #ifdef HDMI
//                 } else {
//                     lastfc = tilefcols_w[lasty1 * X_TILE + lastx1 * fontinc];
//                     lastbc = tilebcols_w[lasty1 * X_TILE + lastx1 * fontinc];
//                 }
//                 if (FullColour) {
//                     for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
//                         tilefcols[lasty1 * X_TILE + i] = RGB555(RED);
//                     for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
//                         tilebcols[lasty1 * X_TILE + i] = RGB555(WHITE);
//                 } else {
//                     for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
//                         tilefcols_w[lasty1 * X_TILE + i] = RGB332(RED);
//                     for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
//                         tilebcols_w[lasty1 * X_TILE + i] = RGB332(WHITE);
//                 }
// #else
//                 for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
//                     tilefcols[lasty1 * X_TILE + i] = RGB121pack(RED);
//                 for (int i = lastx1 * fontinc; i < (lastx1 + 1) * fontinc; i++)
//                     tilebcols[lasty1 * X_TILE + i] = RGB121pack(WHITE);
// #endif
//             }
//             if ((nunstruct[2].L && leftpushed == false && rightpushed == false &&
//                  middlepushed == false) ||
//                 (nunstruct[2].R && leftpushed == false && rightpushed == false &&
//                  middlepushed == false) ||
//                 (nunstruct[2].C && leftpushed == false && rightpushed == false &&
//                  middlepushed == false)) {
//                 if (nunstruct[2].L)
//                     leftpushed = true;
//                 else if (nunstruct[2].R)
//                     rightpushed = true;
//                 else
//                     middlepushed = true;
//                 if (lastx1 >= 0 && lastx1 < VWidth && lasty1 >= 0 &&
//                     lasty1 < VHeight) {  // c == ' ' means mouse down and no shift, ctrl, etc
//                                          //        char * mtxtp=txtp;
//                     p = txtp;
//                     // first position on the y axis
//                     while (*p != 0 && lasty1 > cury)  // assume we have to move down the screen
//                         if (*p++ == '\n') cury++;
//                     while (p != EdBuff && lasty1 < cury)  // assume we have to move up the screen
//                         if (*--p == '\n') cury--;
//                     while (p != EdBuff && *(p - 1) != '\n')
//                         p--;  // move to the beginning of the line
//                     for (curx = 0; curx < lastx1 && *p && *p != '\n'; curx++)
//                         p++;  // now position on the x axis
//                     PositionCursor(p);
//                     mark = p;
//                 }
//                 if (rightpushed == true) {
//                     c = F4;
//                 }
//                 if (middlepushed == true) {
//                     c = F5;
//                 }
//                 if (leftpushed == true) {
//                     c = 9999;
//                 }
//             }
//         }
//         if (!((rightpushed == true && c == F4) || (middlepushed == true && c == F5) ||
//               (leftpushed == true && c == 9999)))
// #endif
// #endif
            c = MMInkey();
        if (c != -1 && errmsg) {
            PrintFunctKeys(MARK);
            errmsg = false;
        }
        switch (c) {
            case ESC:
                // Wait 50ms to see if anything more is coming.
                mmtime_sleep_ns(MILLISECONDS_TO_NANOSECONDS(50));
                if (MMInkey() == '[' && MMInkey() == 'M') {
                    // received escape code for Tera Term reporting a mouse click.  in mark mode we
                    // ignore it
                    MMInkey();
                    MMInkey();
                    MMInkey();
                    break;
                }
                curx = txtpx;
                cury = txtpy;  // just an escape key
                return;

            case CTRLKEY('E'):
            case UP:
                if (cury <= 0) continue;
                p = mark;
                if (*p == '\n')
                    p--;  // step back over the terminator if we are right at the end of the line
                while (p != EdBuff && *p != '\n') p--;  // move to the beginning of the line
                if (p != EdBuff) {
                    p--;  // step over the terminator to the end of the previous line
                    for (i = 0; p != EdBuff && *p != '\n';
                         p--, i++);       // move to the beginning of that line
                    if (*p == '\n') p++;  // and position at the start
                    // if(i >= VWidth) {
                    if (i > VWidth) {
                        editDisplayMsg(" LINE IS TOO LONG ");
                        errmsg = true;
                        continue;
                    }
                }
                mark = p;
                for (i = 0; i < edx + curx && *mark != 0 && *mark != '\n';
                     i++, mark++);  // move the cursor to the column
                curx = i;
                cury--;
                break;

            case CTRLKEY('X'):
            case DOWN:
                if (cury == VHeight - 1) continue;
                for (p = mark, i = curx; *p != 0 && *p != '\n';
                     p++, i++);         // move to the end of this line
                if (*p == 0) continue;  // skip if it is at the end of the file
                                        // if(i >= VWidth) {
                if (i > VWidth) {
                    editDisplayMsg(" LINE IS TOO LONG ");
                    errmsg = true;
                    continue;
                }
                mark = p + 1;  // step over the line terminator to the start of the next line
                for (i = 0; i < edx + curx && *mark != 0 && *mark != '\n';
                     i++, mark++);  // move the cursor to the column
                curx = i;
                cury++;
                break;

            case CTRLKEY('S'):
            case LEFT:
                if (curx == edx) continue;
                mark--;
                curx--;
                break;

            case CTRLKEY('D'):
            case RIGHT:
                if (curx >= VWidth || *mark == 0 || *mark == '\n') continue;
                mark++;
                curx++;
                break;

            case CTRLKEY('U'):
            case HOME:
                if (mark == EdBuff) break;
                if (*mark == '\n')
                    mark--;  // step back over the terminator if we are right at the end of the line
                while (mark != EdBuff && *mark != '\n')
                    mark--;                 // move to the beginning of the line
                if (*mark == '\n') mark++;  // skip if no more lines above this one
                break;

            case CTRLKEY('K'):
            case END:
                if (*mark == 0) break;
                for (p = mark, i = curx; *p != 0 && *p != '\n';
                     p++, i++);  // move to the end of this line
                // if(i >= VWidth) {
                if (i > VWidth) {
                    editDisplayMsg(" LINE IS TOO LONG ");
                    errmsg = true;
                    continue;
                }
                mark = p;
                break;

            case CTRLKEY('Y'):
            case CTRLKEY('T'):
            case F5:
            case F4:
                if (txtp - mark > MAXCLIP || mark - txtp > MAXCLIP) {
                    editDisplayMsg(" MARKED TEXT EXCEEDS CLIPBOARD BUFFER SIZE");
                    errmsg = true;
                    break;
                }
                if (mark <= txtp) {
                    p = mark;
                    while (p < txtp) *cb++ = *p++;
                } else {
                    p = txtp;
                    while (p <= mark - 1) *cb++ = *p++;
                }
                *cb = 0;
                if (c == F5 || c == CTRLKEY('Y')) {
                    PositionCursor(txtp);
#ifndef PICOMITE
#ifndef PICOMITEWEB
#ifdef USBKEYBOARD
                    if (HID[1].Device_type == 2 && DISPLAY_TYPE == SCREENMODE1) {
#else
//                     if (mouse0 && DISPLAY_TYPE == SCREENMODE1) {
#endif
//                         nunstruct[2].ax = curx * FontTable[gui_font >> 4][0] * (gui_font & 0b1111);
//                         nunstruct[2].ay = cury * FontTable[gui_font >> 4][1] * (gui_font & 0b1111);
//                         lastx1 = 9999;
//                         lasty1 = 9999;
//                     }
#endif
#endif
                    return;
                }
                // fall through

            case CTRLKEY(']'):
            case DEL:
                if (mark < txtp) {
                    p = txtp;
                    txtp = mark;
                    mark = p;  // swap txtp and mark
                }
                for (p = txtp; p < mark; p++)
                    if (*p == '\n') nbrlines--;
                for (p = txtp; *mark;) *p++ = *mark++;
                *p++ = 0;
                *p++ = 0;
                TextChanged = true;
                PositionCursor(txtp);
#ifndef PICOMITE
#ifndef PICOMITEWEB
#ifdef USBKEYBOARD
                if (HID[1].Device_type == 2 && DISPLAY_TYPE == SCREENMODE1) {
#else
//                 if (mouse0 && DISPLAY_TYPE == SCREENMODE1) {
// #endif
//                     nunstruct[2].ax = curx * FontTable[gui_font >> 4][0] * (gui_font & 0b1111);
//                     nunstruct[2].ay = cury * FontTable[gui_font >> 4][1] * (gui_font & 0b1111);
//                     lastx1 = 9999;
//                     lasty1 = 9999;
//                 }
#endif
#endif
                return;
            case 9999:
                break;
            default:
                continue;
        }

        x = curx;
        y = cury;
        markmode = true;
        // first unmark the area not marked as a result of the keystroke
        if (oldmark < mark) {
            PositionCursor(oldmark);
            p = oldmark;
            while (p < mark) {
                if (*p == '\n') {
                    SSputchar('\r', 0);
                    //MX470PutC('\r');
                }               // also print on the MX470 display
                //MX470PutC(*p);  // print on the MX470 display
                SSputchar(*p++, 0);
            }
        } else if (oldmark > mark) {
            PositionCursor(mark);
            p = mark;
            while (oldmark > p) {
                if (*p == '\n') {
                    SSputchar('\r', 0);
                    //MX470PutC('\r');
                }               // also print on the MX470 display
                //MX470PutC(*p);  // print on the MX470 display
                SSputchar(*p++, 0);
            }
        }
        fflush(stdout);
        oldmark = mark;
        oldx = x;
        oldy = y;

        // now draw the marked area
        if (mark < txtp) {
            PositionCursor(mark);
            pmeditor_reverse_video();
            // MX470Display(REVERSE_VIDEO);  // reverse video on the MX470 display only
            p = mark;
            while (p < txtp) {
                if (*p == '\n') {
                    SSputchar('\r', 0);
                    //MX470PutC('\r');  // also print on the MX470 display
                }
                //MX470PutC(*p);  // print on the MX470 display
                SSputchar(*p++, 0);
            }
            // MX470Display(REVERSE_VIDEO);  // reverse video back to normal on the MX470 display only
        } else if (mark > txtp) {
            PositionCursor(txtp);
            pmeditor_reverse_video();
            // MX470Display(REVERSE_VIDEO);  // reverse video on the MX470 display only
            p = txtp;
            while (p < mark) {
                if (*p == '\n') {
                    SSputchar('\r', 0);
                    //MX470PutC('\r');  // also print on the MX470 display
                }
                //MX470PutC(*p);  // print on the MX470 display
                SSputchar(*p++, 0);
            }
            MX470Display(REVERSE_VIDEO);  // reverse video back to normal on the MX470 display only
        }
        markmode = false;
        PrintString("\033[0m");  // normal video

        oldx = x;
        oldy = y;
        oldmark = mark;
        PositionCursor(mark);
    }
}

// search through the text in the editing buffer looking for a specific line
// enters with ln = the line required
// exits pointing to the start of the line or pointing to a zero char if not that many lines in the
// buffer
char *findLine(int ln, int *inmulti) {
    char *p, *q;
    *inmulti = false;
    p = q = EdBuff;
    skipspace(q);
    if (q[0] == '/' && q[1] == '*') *inmulti = true;
    if (q[0] == '*' && q[1] == '/') *inmulti = false;
    while (ln && *p) {
        if (*p == '\n') {
            if (*inmulti == 2) *inmulti = false;
            ln--;
            q = &p[1];
            skipspace(q);
            if (q[0] == '/' && q[1] == '*') *inmulti = true;
            if (q[0] == '*' && q[1] == '/') *inmulti = 2;
        }
        p++;
    }
    return (char *)p;
}

int EditCompStr(char *p, const char *tkn) {
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
void SetColour(char *p, int DoVT100) {
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
        if (!multilinecomment) {
            gui_fcolour = GUI_C_NORMAL;
            if (DoVT100) pmeditor_highlight(kHighlightNormal);
        }
        return;
    }

    if (*p == '*' && p[1] == '/' && !inquote) {
        multilinecomment = 2;
        return;
    }

    if (*p == '/' && !inquote && multilinecomment == 2) {
        multilinecomment = false;
        return;
    }

    // check for a comment char
    if (*p == '\'' && !inquote) {
        gui_fcolour = GUI_C_COMMENT;
        if (DoVT100) pmeditor_highlight(kHighlightComment);
        incomment = true;
        return;
    }
    if (*p == '/' && p[1] == '*' && !inquote) {
        char *q = p;
        if (*(--q) == (char)'\n') {
            gui_fcolour = GUI_C_COMMENT;
            if (DoVT100) pmeditor_highlight(kHighlightComment);
            multilinecomment = true;
        }
        return;
    }

    // once in a comment all following chars must be comments also
    if (incomment || multilinecomment) return;

    // check for a quoted string
    if (*p == '\"') {
        if (!inquote) {
            inquote = true;
            gui_fcolour = GUI_C_QUOTE;
            if (DoVT100) pmeditor_highlight(kHighlightQuote);
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
        gui_fcolour = GUI_C_NORMAL;
        if (DoVT100) pmeditor_highlight(kHighlightNormal);
        inkeyword = false;
        return;
    }

    // if we are displaying a number check that we are still actually in it and cmdfile if not
    // this is complicated because numbers can be in hex or scientific notation
    if (innumber) {
        if (!isdigit(*p) && !(toupper(*p) >= 'A' && toupper(*p) <= 'F') && toupper(*p) != 'O' &&
            toupper(*p) != 'H' && *p != '.') {
            gui_fcolour = GUI_C_NORMAL;
            if (DoVT100) pmeditor_highlight(kHighlightNormal);
            innumber = false;
            return;
        } else {
            return;
        }
        // check if we are starting a number
    } else if (!intext) {
        if (isdigit(*p) || *p == '&' || ((*p == '-' || *p == '+' || *p == '.') && isdigit(p[1]))) {
            gui_fcolour = GUI_C_NUMBER;
            if (DoVT100) pmeditor_highlight(kHighlightNumber);
            innumber = true;
            return;
        }
        // check if this is an 8 digit hex number as used in CFunctions
        for (i = 0; i < 8; i++)
            if (!isxdigit(p[i])) break;
        if (i == 8 && (p[8] == ' ' || p[8] == '\'' || p[8] == 0)) {
            gui_fcolour = GUI_C_NUMBER;
            if (DoVT100) pmeditor_highlight(kHighlightNumber);
            innumber = true;
            return;
        }
    }

    // check if this is the start of a keyword
    if (isnamechar(*p) && !intext) {
        for (i = 0; i < commandtbl_size - 1; i++) {  // check the command table for a match
            if (EditCompStr((char *)p, (char *)commandtbl[i].name) != 0 ||
                ((EditCompStr((char *)&p[1], (char *)&commandtbl[i].name[1]) != 0) && *p == '.' &&
                 *commandtbl[i].name == '_')) {
                if (EditCompStr((char *)p, "REM") != 0) {  // special case, REM is a comment
                    gui_fcolour = GUI_C_COMMENT;
                    if (DoVT100) pmeditor_highlight(kHighlightComment);
                    incomment = true;
                } else {
                    gui_fcolour = GUI_C_KEYWORD;
                    if (DoVT100) pmeditor_highlight(kHighlightKeyword);
                    inkeyword = true;
                    if (EditCompStr((char *)p, "GUI") || EditCompStr((char *)p, "OPTION")) {
                        twokeyword = p;
                        while (isalnum(*twokeyword)) twokeyword++;
                        while (*twokeyword == ' ') twokeyword++;
                    }
                    return;
                }
            }
        }
        for (i = 0; i < tokentbl_size - 1; i++) {  // check the token table for a match
            if (EditCompStr((char *)p, (char *)tokentbl[i].name) != 0) {
                gui_fcolour = GUI_C_KEYWORD;
                if (DoVT100) pmeditor_highlight(kHighlightKeyword);
                inkeyword = true;
                return;
            }
        }

        // check for the second keyword in two keyword commands
        if (p == twokeyword) {
            for (pp = (char **)twokeywordtbl; *pp; pp++)
                if (EditCompStr((char *)p, (char *)*pp)) break;
            if (*pp) {
                gui_fcolour = GUI_C_KEYWORD;
                if (DoVT100) pmeditor_highlight(kHighlightKeyword);
                inkeyword = true;
                return;
            }
        }
        if (p >= twokeyword) twokeyword = NULL;

        // check for a range of common keywords
        for (pp = (char **)specialkeywords; *pp; pp++)
            if (EditCompStr((char *)p, (char *)*pp)) break;
        if (*pp) {
            gui_fcolour = GUI_C_KEYWORD;
            if (DoVT100) pmeditor_highlight(kHighlightKeyword);
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
        gui_fcolour = GUI_C_NORMAL;
        if (DoVT100) pmeditor_highlight(kHighlightNormal);
    }
}

// print a line starting at the current column (edx) at the current cursor.
// if the line is beyond the end of the text then just clear to the end of line
// enters with the line number to be printed
void printLine(int ln) {
    char *p;
    int i;
    int inmulti = false;
    // we always colour code the output to the LCD panel on the MX470 (when used as the console)
    if (OPTION_DISPLAY_CONSOLE) {
        MX470PutC('\r');  // print on the MX470 display
        p = findLine(ln, &inmulti);
        // i = VWidth - 1;   // I think this is wrong. Does not show last character in line G.A.
        i = VWidth;
        while (i && *p && *p != '\n') {
            if (!inmulti)
                SetColour((char *)p, false);  // set the colour for the LCD display only
            else
                gui_fcolour = GUI_C_COMMENT;
            MX470PutC(*p++);  // print on the MX470 display
            i--;
        }
        MX470Display(CLEAR_TO_EOL);  // clear to the end of line on the MX470 display only
    }
    SetColour(NULL, false);

    p = findLine(ln, &inmulti);
    if (OPTION_COLOUR_CODE) {
        // if we are colour coding we need to redraw the whole line
        SSputchar('\r', 0);  // display the chars after the editing point
        // i = VWidth - 1;         // I think this is wrong. Does not show last character in line
        // G.A.
        i = VWidth;
    } else {
        // if we are NOT colour coding we can start drawing at the current cursor position
        i = curx;
        while (i-- && *p && *p != '\n') p++;  // find the editing point in the buffer
        i = VWidth - curx;
    }

    while (i && *p && *p != '\n') {
        if (OPTION_COLOUR_CODE) {
            if (!inmulti)
                SetColour((char *)p,
                          true);  // if colour coding is used set the colour for the VT100 emulator
            else {
                gui_fcolour = GUI_C_COMMENT;
                pmeditor_highlight(kHighlightComment);
            }
        }
        SSputchar(*p++, 0);  // display the chars after the editing point
        i--;
    }

    PrintString("\033[K");  // all done, clear to the end of the line on a vt100 emulator
    if (OPTION_COLOUR_CODE) SetColour(NULL, true);
    curx = VWidth - 1;
}

// print a full screen starting with the top left corner specified by edx, edy
// this draws the full screen including blank areas so there is no need to clear the screen first
// it then returns the cursor to its original position
void printScreen(void) {
    int i;

    pmeditor_set_cursor(0, 0);
    for (i = 0; i < VHeight; i++) {
        printLine(i + edy);
        PrintString("\r\n");
        // MX470PutS("\r\n", gui_fcolour, gui_bcolour);
        curx = 0;
        cury = i + 1;
    }
    while (console_getc() != -1);  // consume any keystrokes accumulated while redrawing the screen
}

// position the cursor on the screen
// void SCursor(int x, int y) {
//     char s[12];

//     PrintString("\033[");
//     IntToStr(s, y + 1, 10);
//     PrintString(s);
//     PrintString(";");
//     IntToStr(s, x + 1, 10);
//     PrintString(s);
//     PrintString("H");
//     pmeditor_set_cursor(x, y);
//     curx = x;
//     cury = y;
// }

// move the text down by one char starting at the current position in the text
// and insert a character
int editInsertChar(char c, char *multi, int edit_buff_size) {
    char *p;

    for (p = EdBuff; *p; p++);  // find the end of the text in memory
    if (p >= EdBuff + edit_buff_size -
                 10) {  // and check that we have the space (allow 10 bytes for slack)
        editDisplayMsg(" OUT OF MEMORY ");
        return false;
    }
    for (; p >= txtp; p--) *(p + 1) = *p;  // shift everything down
    *multi = 0;
    p = txtp - 1;
    if ((c == '/' && *p == '*') || (c == '*' && *p == '/')) *multi = 1;
    p += 2;
    if ((c == '/' && *p == '*') || (c == '*' && *p == '/')) *multi = 1;
    *txtp++ = c;  // and insert our char
    return true;
}

// print the function keys at the bottom of the screen
void PrintFunctKeys(int typ) {
    int i;
    const char *p;

    if (typ == EDIT) {
        if (VWidth >= 78)
            p = "ESC:Exit  F1:Save  F2:Run  F3:Find  F4:Mark  F5:Paste";
        else if (VWidth >= 62)
            p = "F1:Save F2:Run F3:Find F4:Mark F5:Paste";
        else
            p = "EDIT MODE";
    } else {
        if (VWidth >= 49)
            p = "MARK MODE   ESC=Exit  DEL:Delete  F4:Cut  F5:Copy";
        else
            p = "MARK MODE";
    }

    // MX470Display(DRAW_LINE);  // on the MX470 display draw the line
    // MX470PutS(p, GUI_C_STATUS,
    //           gui_bcolour);      // display the string on the display attached to the MX470
    // MX470Display(CLEAR_TO_EOL);  // clear to the end of line on the MX470 display only

    const int x = curx;
    const int y = cury;
    pmeditor_set_cursor(0, VHeight);
    if (OPTION_COLOUR_CODE) pmeditor_highlight(kHighlightLine);
    pmeditor_underline();
    for (i = 0; i < VWidth; i++) SSputchar(' ', 0);
    pmeditor_reset();
    pmeditor_puts("\r\n");
    if (OPTION_COLOUR_CODE) pmeditor_highlight(kHighlightStatus);
    pmeditor_puts(p);
    if (OPTION_COLOUR_CODE) pmeditor_highlight(kHighlightNormal);
    pmeditor_clear_to_eol();
    pmeditor_set_cursor(x, y);
}

// print the current status
void PrintStatus(void) {
    char s[MAXSTRLEN];

    const int tx = edx + curx + 1;
    strcpy(s, "Ln: ");
    IntToStr(s + strlen(s), edy + cury + 1, 10);
    strcat(s + strlen(s), "  Col: ");
    IntToStr(s + strlen(s), tx, 10);
    strcat(s, "       ");
    strcpy(s + 19, insert ? "INS" : "OVR");

    // MX470Cursor((VWidth - strlen(s)) * gui_font_width,
    //             (VRes / gui_font_height) * gui_font_height - gui_font_height);
    // MX470PutS(s, GUI_C_STATUS,
    //           gui_bcolour);  // display the string on the display attached to the MX470

    pmeditor_set_cursor(VWidth - 25, VHeight + 1);
    if (OPTION_COLOUR_CODE) pmeditor_highlight(kHighlightStatus);
    PrintString(s);
    if (OPTION_COLOUR_CODE) pmeditor_highlight(kHighlightNormal);

    PositionCursor(txtp);
}

// display a message in the status line
void editDisplayMsg(const char *msg) {
    pmeditor_set_cursor(0, VHeight + 1);
    if (OPTION_COLOUR_CODE) pmeditor_highlight(kHighlightError);
    pmeditor_reverse_video();
    // MX470Cursor(0, (VRes / gui_font_height) * gui_font_height - gui_font_height);
    PrintString(msg);
    // MX470PutS((char *)msg, BLACK, RED);
    if (OPTION_COLOUR_CODE) pmeditor_highlight(kHighlightNormal);
    pmeditor_reset();
    pmeditor_clear_to_eol();       // clear to the end of the line on a vt100 emulator
    // MX470Display(CLEAR_TO_EOL);  // clear to the end of line on the MX470 display only
    PositionCursor(txtp);
    drawstatusline = true;
}

// get an input string from the user and save into inpbuf
void GetInputString(char *prompt) {
    int i;
    char *p;

    pmeditor_set_cursor(0, VHeight + 1);
    PrintString((char *)prompt);
//    MX470Cursor(0, (VRes / gui_font_height) * gui_font_height - gui_font_height);
//    MX470PutS((char *)prompt, gui_fcolour, gui_bcolour);
    for (i = 0; i < VWidth - (int) strlen((char *)prompt); i++) {
        SSputchar(' ', 1);
//        MX470PutC(' ');
    }
    pmeditor_set_cursor(strlen((char *)prompt), VHeight + 1);
//    MX470Cursor(strlen((char *)prompt) * gui_font_width,
//                (VRes / gui_font_height) * gui_font_height - gui_font_height);
    for (p = inpbuf; (*p = MMgetchar()) != '\r'; p++) {  // get the input
        if (*p == 0xb3 || *p == F3 || *p == ESC) {
            p++;
            break;
        }  // return if it is SHIFT-F3, F3 or ESC
        if (isprint(*p)) {
            SSputchar(*p, 1);  // echo the char
//            MX470PutC(*p);     // echo the char on the MX470 display
        }
        if (*p == '\b') {
            p--;  // backspace over a backspace
            if (p >= inpbuf) {
                p--;                                           // and the char before
                PrintString("\b \b");                          // erase on the screen
//                MX470PutS("\b \b", gui_fcolour, gui_bcolour);  // erase on the MX470 display
            }
        }
    }
    *p = 0;  // terminate the input string
    PrintFunctKeys(EDIT);
    PositionCursor(txtp);
}

// scroll up the video screen
void Scroll(void) {
    edy++;
    pmeditor_set_cursor(0, VHeight);
    PrintString("\033[J\033[99B\n");  // clear to end of screen, move to the end of the screen and
                                      // force a scroll of one line
    MX470Cursor(0, VHeight * gui_font_height);
    MX470Scroll(gui_font_height);
    pmeditor_set_cursor(0, VHeight);
    curx = 0;
    cury = VHeight - 1;
    PrintFunctKeys(EDIT);
    printLine(VHeight - 1 + edy);
    PositionCursor(txtp);
    while (console_getc() != -1);  // consume any keystrokes accumulated while redrawing the screen
}

// scroll down the video screen
void ScrollDown(void) {
    pmeditor_set_cursor(0, VHeight);    // go to the end of the editing area
    PrintString("\033[J");  // clear to end of screen
    edy--;
    pmeditor_set_cursor(0, 0);
    PrintString("\033M");  // scroll window down one line
    MX470Scroll(-gui_font_height);
    printLine(edy);
    PrintFunctKeys(EDIT);
    PositionCursor(txtp);

    while (console_getc() != -1) routinechecks();
    ;  // consume any keystrokes accumulated while redrawing the screen
}

#endif

/** Allocates the editor buffer. */
static MmResult pmeditor_alloc_buf() {
    if (pmeditor_buf) ClearSpecificTempMemory(pmeditor_buf);
    pmeditor_buf = GetTempMemory(EDIT_BUFFER_SIZE);
    return kOk;
}

/** Fills the editor buffer from a file. */
static MmResult pmeditor_load_file(const char *filename) {
    pmeditor_num_lines = 0;
    char *p = pmeditor_buf;
    const int fnbr = file_find_free();
    ON_FAILURE_RETURN(file_open(filename, "rb", fnbr));
    if (!file_eof(fnbr)) {
        for (;;) {
            int ch = file_getc(fnbr);
            if (ch == -1) break;
            // TODO: Handle overrun.
            *p++ = ch;
            if (ch == '\n') pmeditor_num_lines++;
        }
    }
    return file_close(fnbr);
}

/**
 * Shows the "PicoMite" editor.
 *
 * @param  filename  File to edit.
 * @param  line      Line to place the cursor on.
 */
MmResult pmeditor(const char *filename, int line) {
    ON_FAILURE_RETURN(console_get_size(&mmb_options.width, &mmb_options.height, 0));

    pmeditor_height = mmb_options.height - 2;
    pmeditor_width = mmb_options.width;
    pmeditor_cursor_x = 0;
    pmeditor_cursor_y = 0;
    PrintString("\033[?1000h");         // Tera Term turn on mouse click report in VT200 mode
    PrintString("\0337\033[2J\033[H");  // vt100 clear screen and home cursor
    MX470Display(DISPLAY_CLS);          // clear screen on the MX470 display only
    pmeditor_set_cursor(0, 0);
    PrintFunctKeys(EDIT);
    ON_FAILURE_RETURN(pmeditor_alloc_buf());
    ON_FAILURE_RETURN(pmeditor_load_file(filename));

    int x = 0;
    int y = 0;
    if (pmeditor_num_lines > pmeditor_height) {
        pmeditor_page_y = y - pmeditor_height / 2;  // edy is the line displayed at the top
        if (pmeditor_page_y < 0) pmeditor_page_y = 0;   // compensate if we are near the start
        y -= pmeditor_page_y;            // y is the line on the screen
    }

    txtp = pmeditor_buf;
    // if (cmdfile) m_alloc(M_VAR);  // clean up clipboard usage
    FullScreenEditor(x, y, filename, EDIT_BUFFER_SIZE);
    // if (cmdfile)
    //     memset(tknbuf, 0,
    //            STRINGSIZE);  // zero this so that nextstmt is pointing to the end of program
//    MMCharPos = 0;
    return kOk;
}
