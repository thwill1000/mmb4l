/***********************************************************************************************************************
MMBasic

Editor.c

Implements the full screen editor.

Copyright 2011 - 2020 Geoff Graham.  All Rights Reserved.

This file and modified versions of this file are supplied to specific individuals or organisations under the following
provisions:

- This file, or any files that comprise the MMBasic source (modified or not), may not be distributed or copied to any other
  person or organisation without written permission.

- Object files (.o and .hex files) generated using this file (modified or not) may not be distributed or copied to any other
  person or organisation without written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

************************************************************************************************************************/

#include <stdio.h>
#include <stdio.h>
#include <conio.h>
#include <direct.h>
#include <signal.h>
#include <time.h>
#include <windows.h>
#include <wincon.h>
#include <process.h>

#include "..\..\Version.h"


#define CTRLKEY(a) (a & 0x1f)


#define DISPLAY_CLS             1
#define REVERSE_VIDEO           3
#define CLEAR_TO_EOL            4
#define SCROLL_DOWN             6
#define DRAW_LINE               7

#define BLACK       0
#define GRAY        8
#define BLUE        1
#define BBLUE       9
#define GREEN       2
#define BGREEN      10
#define CYAN        3
#define BCYAN       11
#define RED         4
#define BRED        12
#define PURPLE      5
#define BPURPLE     13
#define YELLOW      6
#define BYELLOW     14
#define WHITE       7
#define BWHITE      15

#define LINE_CHAR   196

// these are the only global variables, the default place for the cursor when the editor opens and colours
char *StartEditPoint = NULL;
int StartEditChar = 0;


/********************************************************************************************************************************************
 THE EDIT COMMAND
********************************************************************************************************************************************/

char *EdBuff;                     // the buffer used for editing the text
int nbrlines;                     // size of the text held in the buffer (in lines)
int VWidth, VHeight;              // editing screen width and height in characters
int edx, edy;                     // column and row at the top left hand corner of the editing screen (in characters)
int curx, cury;                   // column and row of the current cursor (in characters) relative to the top left hand corner of the editing screen
char *txtp;                       // position of the current cursor in the text being edited
int drawstatusline;               // true if the status line needs to be redrawn on the next keystroke
int insert;                       // true if the editor is in insert text mode
int tempx;                        // used to track the preferred x position when up/down arrowing
int TextChanged;                  // true if the program has been modified and therefore a save might be required
DWORD ConsoleMode;                // the DOS console output mode when the editor was started
char EditRAM[PROG_FLASH_SIZE];

#define EDIT  1                   // used to select the status line string
#define MARK  2

int C_Normal = BWHITE;
int C_Backgnd = BLACK;
int C_Comment = BYELLOW;
int C_Keyword = BCYAN;
int C_Quote = BGREEN;
int C_Number = BWHITE;
int C_Status = PURPLE;
int StartFColour, StartBColour;

void FullScreenEditor(void);
char *findLine(int ln);
void printLine(int ln);
void printScreen(void);
void SCursor(int x, int y);
int editInsertChar(char c);
void PrintFunctKeys(int);
void PrintStatus(void);
void SaveToProgMemory(void);
void editDisplayMsg(char *msg);
void GetInputString(char *prompt);
void Scroll(void);
void ScrollDown(void);
void MarkMode(char *cb, char *buf);
void PositionCursor(char *curp);
int DOSgetch(void);



void DOSPutS(char *s, int fc, int bc) {
    DOSColour(fc, bc);
    MMPrintString(s);
}



void DOSDisplay(int fn) {
    int i, t;
    switch(fn) {
        case DISPLAY_CLS:       system("CLS");
                                break;
        case REVERSE_VIDEO:     DOSColour(C_Backgnd, C_Normal);
                                break;
        case CLEAR_TO_EOL:        while(MMCharPos <= VWidth) MMputchar(' ');
                              break;
        case SCROLL_DOWN:
                                break;
        case DRAW_LINE:         DOSColour(C_Status, C_Backgnd);
                              DOSCursor(0, Option.Height - 2);
                              for(t = 0; t < VWidth; t++) MMputchar(LINE_CHAR);
                                DOSCursor(0, Option.Height - 2);
                                break;
    }
}



// edit command:
//  EDIT              Will run the full screen editor on the current program memory, if run after an error will place the cursor on the error line
void cmd_edit(void) {
  char *fromp, *p;
  int y, x;
  CONSOLE_SCREEN_BUFFER_INFO consoleinfo;

  GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &ConsoleMode);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ConsoleMode & ~ENABLE_WRAP_AT_EOL_OUTPUT);

  EdBuff = EditRAM;

   GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleinfo);
   StartFColour = consoleinfo.wAttributes & 0x0f;
   StartBColour = consoleinfo.wAttributes >> 4;

    // get the colours from the environment variable
    p = getenv("MMCOLOURS");
    if(p != NULL) {
      getargs(&p, 13, ",");                           // getargs macro must be the first executable stmt in a block
        if(argc == 13) {
            C_Normal = strtol(argv[0], &p, 10);
            C_Backgnd = strtol(argv[2], &p, 10);
            C_Comment = strtol(argv[4], &p, 10);
            C_Keyword = strtol(argv[6], &p, 10);
            C_Quote = strtol(argv[8], &p, 10);
            C_Number = strtol(argv[10], &p, 10);
            C_Status = strtol(argv[12], &p, 10);
        }
    }

  if(CurrentLinePtr) error("Invalid in a program");
  GetConsoleSize();
  if(Option.Height < 12 || Option.Width < 75) error("Console window is too small");

  ClearRuntime();
    *EdBuff = 0;

  VHeight = Option.Height - 2;
  VWidth = Option.Width;
  edx = edy = curx = cury = y = x = tempx = 0;
  txtp = EdBuff;
  *tknbuf = 0;

    fromp  = ProgMemory;
    p = EdBuff;
    nbrlines = 0;
    while(*fromp != 0xff) {
        if(*fromp == T_LINENBR || *fromp == T_NEWLINE) {
            if(StartEditPoint >= ProgMemory) {
                if(StartEditPoint == fromp) {
                    y = nbrlines;                                   // we will start editing at this line
                    tempx = x = StartEditChar;
                    txtp = p + StartEditChar;
                }
            } else {
                if(StartEditPoint == (char *)nbrlines) {
                    y = nbrlines;
                    tempx = x = StartEditChar;
                    txtp = p + StartEditChar;
                }
            }
            nbrlines++;
            fromp = llist(p, fromp);                                // otherwise expand the line
            p += strlen(p);
            *p++ = '\n'; *p = 0;
        }
        // finally, is it the end of the program?
        if(fromp[0] == 0 || fromp[0] == 0xff) break;
    }
    if(nbrlines == 0) nbrlines++;
    if(p > EdBuff) --p;
    *p = 0;                                                         // erase the last line terminator

  DOSColour(C_Normal, C_Backgnd);
    DOSDisplay(DISPLAY_CLS);                                        // clear screen
  SCursor(0, 0);
  PrintFunctKeys(EDIT);

  if(nbrlines > VHeight) {
      edy = y - VHeight/2;                                          // edy is the line displayed at the top
      if(edy < 0) edy = 0;                                          // compensate if we are near the start
      y = y - edy;                                                  // y is the line on the screen
  }
  printScreen();                                                    // draw the screen
  SCursor(x, y);
  drawstatusline = true;
  FullScreenEditor();
  memset(tknbuf, 0, STRINGSIZE);                                    // zero this so that nextstmt is pointing to the end of program
}



void FullScreenEditor(void) {
  int c, i;
  char buf[STRINGSIZE + 2], clipboard[STRINGSIZE];
  char *p, *tp;
  char lastkey = 0;
  int x, y;
    DWORD mode;

  clipboard[0] = 0;
  insert = true;
  TextChanged = false;
    while(1) {
      PrintStatus();
        GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode & ~ENABLE_PROCESSED_INPUT);
        c = DOSgetch();
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);
      if(drawstatusline) PrintFunctKeys(EDIT);
      drawstatusline = false;
      if(c == TAB) {
            strcpy(buf, "        ");
            buf[Option.Tab - ((edx + curx) % Option.Tab)] = 0;
      } else {
          buf[0] = c;
          buf[1] = 0;
      }
      do {
            if(buf[0] == 3) buf[0] = ESC;                           // if the user tried to break turn it into an escape
          switch(buf[0]) {

              case '\r':
              case '\n':  // first count the spaces at the beginning of the line
                          if(txtp != EdBuff && (*txtp == '\n' || *txtp == 0)) {   // we only do this if we are at the end of the line
                              for(tp = txtp - 1, i = 0; *tp != '\n' && tp >= EdBuff; tp--)
                                  if(*tp != ' ')
                                      i = 0;                                      // not a space
                                  else
                                      i++;                                        // potential space at the start
                              if(tp == EdBuff && *tp == ' ') i++;                 // correct for a counting error at the start of the buffer
                              if(buf[1] != 0)
                                  i = 0;                                          // do not insert spaces if buffer too small or has something in it
                              else
                                  buf[i + 1] = 0;                                 // make sure that the end of the buffer is zeroed
                              while(i) buf[i--] = ' ';                            // now, place our spaces in the typeahead buffer
                          }
                          if(!editInsertChar('\n')) break;                        // insert the newline
                          TextChanged = true;
                          nbrlines++;
                          y = cury;
                          if(cury < VHeight - 1)                                  // if we are NOT at the bittom
                              y++;                                                // just increment the cursor
                          else
                              edy++;                                              // otherwise scroll
                          printScreen();                                          // redraw everything
                          PositionCursor(txtp);
                          break;

                case CTRLKEY('E'):
              case UP:    if(cury == 0 && edy == 0) break;
                          if(*txtp == '\n') txtp--;                               // step back over the terminator if we are right at the end of the line
                          while(txtp != EdBuff && *txtp != '\n') txtp--;          // move to the beginning of the line
                          if(txtp != EdBuff) {
                              txtp--;                                             // step over the terminator to the end of the previous line
                              while(txtp != EdBuff && *txtp != '\n') txtp--;      // move to the beginning of that line
                              if(*txtp == '\n') txtp++;                           // and position at the start
                          }
                          for(i = 0; i < edx + tempx && *txtp != 0 && *txtp != '\n'; i++, txtp++);  // move the cursor to the column

                          if(cury > 2 || edy == 0) {                              // if we are more that two lines from the top
                              if(cury > 0) SCursor(i, cury - 1);                  // just move the cursor up
                          }
                          else if(edy > 0) {                                      // if we are two lines or less from the top
                              curx = i;
                              ScrollDown();
                          }
                          PositionCursor(txtp);
                          break;

                case CTRLKEY('X'):
              case DOWN:  p = txtp;
                          while(*p != 0 && *p != '\n') p++;                       // move to the end of this line
                          if(*p == 0) break;                                      // skip if it is at the end of the file
                          p++;                                                    // step over the line terminator to the start of the next line
                          for(i = 0; i < edx + tempx && *p != 0 && *p != '\n'; i++, p++);  // move the cursor to the column
                          txtp = p;

                          if(cury < VHeight - 3 || edy + VHeight == nbrlines) {
                              if(cury < VHeight - 1) SCursor(i, cury + 1);
                          }
                          else if(edy + VHeight < nbrlines) {
                              curx = i;
                              Scroll();
                          }
                          PositionCursor(txtp);
                          break;

                case CTRLKEY('S'):
              case LEFT:  if(txtp == EdBuff) break;
                          if(*(txtp - 1) == '\n') {                                // if at the beginning of the line wrap around
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
                case RIGHT: if(*txtp == '\n') {                                   // if at the end of the line wrap around
                              buf[1] = HOME;
                              buf[2] = DOWN;
                              buf[3] = 0;
                              break;
                          }
                          if(curx >= VWidth) {
                              editDisplayMsg(" LINE IS TOO LONG ");
                              break;
                          }

                   //     if(*txtp == 0) break;                                   // end of buffer
                          if(*txtp != 0) txtp++;                                  // now we can move the cursor
                          PositionCursor(txtp);
                          break;

              // backspace
              case BKSP:  if(txtp == EdBuff) break;
                          if(*(txtp - 1) == '\n') {                               // if at the beginning of the line wrap around
                              buf[1] = UP;
                              buf[2] = END;
                              buf[3] = DEL;
                              buf[4] = 0;
                              break;
                          }
                          // find how many spaces are between the cursor and the start of the line
                          for(p = txtp - 1; *p == ' ' && p != EdBuff; p--);
                          if((p == EdBuff || *p == '\n') && txtp - p > 1) {
                              i = txtp - p - 1;
                              // we have have the number of continuous spaces between the cursor and the start of the line
                              // now figure out the number of backspaces to the nearest tab stop

                                i = (i % Option.Tab); if(i == 0) i = Option.Tab;
                                // load the corresponding number of deletes in the type ahead buffer
                                buf[i + 1] = 0;
                                while(i--) {
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

                case CTRLKEY(']'):
              case DEL: if(*txtp == 0) break;
                          p = txtp;
                          c = *p;
                          while(*p) {
                              p[0] = p[1];
                              p++;
                          }
                          x = curx; y = cury;
                          if(c == '\n') {
                              printScreen();
                              nbrlines--;
                          }
                          else
                              printLine(edy + cury);
                          TextChanged = true;
                          PositionCursor(txtp);
                          break;

                case CTRLKEY('N'):
              case INSERT:insert = !insert;
                          break;

                case CTRLKEY('U'):
              case HOME:  if(txtp == EdBuff) break;
                          if(lastkey == HOME || lastkey == CTRLKEY('U')) {
                              edx = edy = curx = cury = 0;
                              txtp = EdBuff;
                                DOSDisplay(DISPLAY_CLS);                          // clear screen
                              printScreen();
                              PrintFunctKeys(EDIT);
                              PositionCursor(txtp);
                              break;
                          }
                          if(*txtp == '\n') txtp--;                               // step back over the terminator if we are right at the end of the line
                          while(txtp != EdBuff && *txtp != '\n') txtp--;          // move to the beginning of the line
                          if(*txtp == '\n') txtp++;                               // skip if no more lines above this one
                          PositionCursor(txtp);
                          break;

                case CTRLKEY('K'):
              case END: if(*txtp == 0) break;                                     // already at the end
                          if(lastkey == END || lastkey == CTRLKEY('K')) {         // jump to the end of the file
                              i = 0; p = txtp = EdBuff;
                              while(*txtp != 0) {
                                  if(*txtp == '\n') { p = txtp + 1; i++; }
                                  txtp++;
                              }

                              if(i >= VHeight) {
                                  edy = i - VHeight + 1;
                                  printScreen();
                                  cury = VHeight - 1;
                              } else {
                                  cury = i;
                              }
                              txtp = p;
                              curx = 0;
                          }

                          while(curx < VWidth && *txtp != 0 && *txtp != '\n') {
                              txtp++;
                              PositionCursor(txtp);
                              //SCursor(curx + 1, cury);
                          }
                          if(curx > VWidth) editDisplayMsg(" LINE IS TOO LONG ");
                          break;

                case CTRLKEY('P'):
              case PUP: if(edy == 0) {                                            // if we are already showing the top of the text
                              buf[1] = HOME;                                      // force the editing point to the start of the text
                              buf[2] = HOME;
                              buf[3] = 0;
                              break;
                          } else if(edy >= VHeight - 1) {                         // if we can scroll a full screenfull
                              i = VHeight + 1;
                              edy -= VHeight;
                          } else {                                                // if it is less than a full screenfull
                              i = edy + 1;
                              edy = 0;
                          }
                          while(i--) {
                              if(*txtp == '\n') txtp--;                           // step back over the terminator if we are right at the end of the line
                              while(txtp != EdBuff && *txtp != '\n') txtp--;      // move to the beginning of the line
                              if(txtp == EdBuff) break;                           // skip if no more lines above this one
                          }
                          if(txtp != EdBuff) txtp++;                              // and position at the start of the line
                          for(i = 0; i < edx + curx && *txtp != 0 && *txtp != '\n'; i++, txtp++);  // move the cursor to the column
                          y = cury;
                          /* if(cury != 0) */ printScreen();
                          PositionCursor(txtp);
                          // SCursor(i, y);
                          break;

                case CTRLKEY('L'):
              case PDOWN:   if(nbrlines <= edy + VHeight + 1) {                   // if we are already showing the end of the text
                              buf[1] = END;                                       // force the editing point to the end of the text
                              buf[2] = END;
                              buf[3] = 0;
                              break;                                              // cursor to the top line
                          } else if(nbrlines - edy - VHeight >= VHeight) {        // if we can scroll a full screenfull
                              edy += VHeight;
                              i = VHeight;
                          } else {                                                // if it is less than a full screenfull
                              i = nbrlines - VHeight - edy;
                              edy = nbrlines - VHeight;
                          }
                          if(*txtp == '\n') i--;                                  // compensate if we are right at the end of a line
                          while(i--) {
                              if(*txtp == '\n') txtp++;                           // step over the terminator if we are right at the start of the line
                              while(*txtp != 0 && *txtp != '\n') txtp++;          // move to the end of the line
                              if(*txtp == 0) break;                               // skip if no more lines after this one
                          }
                          if(txtp != EdBuff) txtp++;                              // and position at the start of the line
                          for(i = 0; i < edx + curx && *txtp != 0 && *txtp != '\n'; i++, txtp++);  // move the cursor to the column
                          y = cury;
                          printScreen();
                          PositionCursor(txtp);
                          // SCursor(i, y);
                          break;

              // Abort without saving
              case 3:     // CTRL-C
              case ESC: if(TextChanged) {
                              GetInputString("Exit and discard all changes (Y/N): ");
                              if(toupper(*inpbuf) != 'Y') break;
                          }
                          // fall through to the normal exit

                case CTRLKEY('Q'):   // Save and exit
              case F1:             // Save and exit
                case CTRLKEY('W'):     // Save, exit and run
              case F2:             // Save, exit and run
                            if(buf[0] != ESC && TextChanged) SaveToProgMemory();
                          DOSColour(StartFColour, StartBColour);
                          SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ConsoleMode);
                          DOSDisplay(DISPLAY_CLS);                          // clear screen
                          if(buf[0] == ESC || buf[0] == CTRLKEY('Q') || buf[0] == F1) return;
                            // this must be save, exit and run.  We have done the first two, now do the run part.
                          ClearRuntime();
                            WatchdogSet = false;
                            PrepareProgram(true);
                          nextstmt = ProgMemory;
                          return;

              // Search
                case CTRLKEY('R'):
              case F3:    GetInputString("Find (Use F6 to repeat): ");
                          if(*inpbuf == 0 || *inpbuf == ESC) break;
                          if(!(*inpbuf == 0xb3 || *inpbuf == F3)) strcpy(tknbuf, inpbuf);
                          // fall through

                case CTRLKEY('G'):
              case F6:
                          p = txtp;
                          if(*p == 0) p = EdBuff - 1;
                          i = strlen(tknbuf);
                          while(1) {
                              p++;
                              if(p == txtp) break;
                              if(*p == 0) p = EdBuff;
                              if(p == txtp) break;
                              if(mem_equal(p, tknbuf, i)) break;
                          }
                          if(p == txtp) {
                              editDisplayMsg(" NOT FOUND ");
                              break;
                          }
                          for(y = x = 0, txtp = EdBuff; txtp != p; txtp++) {      // find the line and column of the string
                              x++;
                              if(*txtp == '\n') {
                                  y++;                                            // y is the line
                                  x = 0;                                          // x is the column
                              }
                          }
                          edy = y - VHeight/2;                                    // edy is the line displayed at the top
                          if(edy < 0) edy = 0;                                    // compensate if we are near the start
                          y = y - edy;                                            // y is the line on the screen
                          printScreen();
                          PositionCursor(txtp);
                          // SCursor(x, y);
                          break;

              // Mark
                case CTRLKEY('T'):
              case F4:    MarkMode(clipboard, &buf[1]);
                          printScreen();
                          PrintFunctKeys(EDIT);
                          PositionCursor(txtp);
                          break;

                case CTRLKEY('Y'):
              case F5:    if(*clipboard == 0) {
                              editDisplayMsg(" CLIPBOARD IS EMPTY ");
                              break;
                          }
                          for(i = 0; clipboard[i]; i++) buf[i + 1] = clipboard[i];
                          buf[i + 1] = 0;
                          break;

              // F6 to F12 - Normal function keys
              case F7:
              case F8:
              case F9:
              case F10:
              case F11:
              case F12:
                          break;

              // a normal character
              default:    c = buf[0];
                          if(c < ' ' || c > '~') break;                           // make sure that this is valid
                          if(curx >= VWidth) {
                              editDisplayMsg(" LINE IS TOO LONG ");
                              break;
                          }
                          TextChanged = true;
                          if(insert || *txtp == '\n' || *txtp == 0) {
                              if(!editInsertChar(c)) break;                       // insert it
                          } else
                              *txtp++ = c;                                        // or just overtype
//                            if(Option.ColourCode) {
                                printLine(edy + cury);                            // redraw the whole line so that colour coding will occur
//                            } else {
//                                MMputchar(c);                                   // and echo
//                                x = ++curx;
//                                if(insert &&  *txtp != '\n' && *txtp != 0) printLine(edy + cury);
//                            }
                          PositionCursor(txtp);
                          // SCursor(x, cury);
                          tempx = cury;                                           // used to track the preferred cursor position
                          break;

          }
          lastkey = buf[0];
          if(buf[0] != UP && buf[0] != DOWN && buf[0] != CTRLKEY('E') && buf[0] != CTRLKEY('X')) tempx = curx;
          buf[STRINGSIZE + 1] = 0;
          for(i = 0; i < STRINGSIZE + 1; i++) buf[i] = buf[i + 1];                // suffle down the buffer to get the next char
      } while(*buf);
  }
}


/*******************************************************************************************************************
  UTILITY FUNCTIONS USED BY THE FULL SCREEN EDITOR
*******************************************************************************************************************/


void PositionCursor(char *curp) {
  int ln, col;
  char *p;

  for(p = EdBuff, ln = col = 0; p < curp; p++) {
      if(*p == '\n') {
          ln++;
          col = 0;
      } else
          col++;
  }
  if(ln < edy || ln >= edy + VHeight) return;
  SCursor(col, ln - edy);
}



// mark mode
// implement the mark mode (when the user presses F4)
void MarkMode(char *cb, char *buf) {
  char *p, *mark, *oldmark;
  int c, x, y, i, oldx, oldy, txtpx, txtpy, errmsg = false;

  PrintFunctKeys(MARK);
  oldmark = mark = txtp;
  txtpx = oldx = curx; txtpy = oldy = cury;
  while(1) {
      c = MMInkey();
      if(c != -1 && errmsg) {
            PrintFunctKeys(MARK);
            errmsg = false;
        }
      switch(c) {
          case ESC: curx = txtpx; cury = txtpy;
                      return;

            case CTRLKEY('E'):
          case UP:    if(cury <= 0) continue;
                      p = mark;
                      if(*p == '\n') p--;                                     // step back over the terminator if we are right at the end of the line
                      while(p != EdBuff && *p != '\n') p--;                   // move to the beginning of the line
                      if(p != EdBuff) {
                          p--;                                                // step over the terminator to the end of the previous line
                          for(i = 0; p != EdBuff && *p != '\n'; p--, i++);    // move to the beginning of that line
                          if(*p == '\n') p++;                                 // and position at the start
                          if(i >= VWidth) {
                              editDisplayMsg(" LINE IS TOO LONG ");
                              errmsg = true;
                              continue;
                          }
                      }
                      mark = p;
                      for(i = 0; i < edx + curx && *mark != 0 && *mark != '\n'; i++, mark++);  // move the cursor to the column
                      curx = i; cury--;
                      break;

            case CTRLKEY('X'):
          case DOWN:  if(cury == VHeight -1) continue;
                      for(p = mark, i = curx; *p != 0 && *p != '\n'; p++, i++);// move to the end of this line
                      if(*p == 0) continue;                                    // skip if it is at the end of the file
                      if(i >= VWidth) {
                          editDisplayMsg(" LINE IS TOO LONG ");
                          errmsg = true;
                          continue;
                      }
                      mark = p + 1;                                       // step over the line terminator to the start of the next line
                      for(i = 0; i < edx + curx && *mark != 0 && *mark != '\n'; i++, mark++);  // move the cursor to the column
                      curx = i; cury++;
                      break;

            case CTRLKEY('S'):
          case LEFT:  if(curx == edx) continue;
                      mark--;
                      curx--;
                      break;

            case CTRLKEY('D'):
          case RIGHT: if(curx >= VWidth || *mark == 0 || *mark == '\n') continue;
                      mark++;
                      curx++;
                      break;

            case CTRLKEY('U'):
          case HOME:  if(mark == EdBuff) break;
                      if(*mark == '\n') mark--;                           // step back over the terminator if we are right at the end of the line
                      while(mark != EdBuff && *mark != '\n') mark--;      // move to the beginning of the line
                      if(*mark == '\n') mark++;                           // skip if no more lines above this one
                      break;

            case CTRLKEY('K'):
          case END: if(*mark == 0) break;
                      for(p = mark, i = curx; *p != 0 && *p != '\n'; p++, i++);// move to the end of this line
                      if(i >= VWidth) {
                          editDisplayMsg(" LINE IS TOO LONG ");
                          errmsg = true;
                          continue;
                      }
                      mark = p;
                      break;

            case CTRLKEY('Y'):
            case CTRLKEY('T'):
          case F5:
          case F4:    if(txtp - mark > MAXSTRLEN || mark - txtp > MAXSTRLEN) {
                          editDisplayMsg(" MARKED TEXT EXCEEDS 255 CHARACTERS ");
                          errmsg = true;
                          break;
                      }
                      if(mark <= txtp) {
                          p = mark;
                          while(p < txtp) *cb++ = *p++;
                      } else {
                          p = txtp;
                          while(p <= mark - 1) *cb++ = *p++;
                      }
                      *cb = 0;
                      if(c == F5 || c == CTRLKEY('Y')) {
                          PositionCursor(txtp);
                          return;
                      }
                      // fall through

            case CTRLKEY(']'):
          case DEL: if(mark < txtp) {
                          p = txtp;  txtp = mark; mark = p;               // swap txtp and mark
                      }
                      for(p = txtp; p < mark; p++) if(*p == '\n') nbrlines--;
                      for(p = txtp; *mark; ) *p++ = *mark++;
                      *p++ = 0; *p++ = 0;
                      TextChanged = true;
                      PositionCursor(txtp);
                      return;

          default:    continue;
      }

      x = curx; y = cury;

      // first unmark the area not marked as a result of the keystroke
      if(oldmark < mark) {
          PositionCursor(oldmark);
          p = oldmark;
          while(p < mark) {
              if(*p == '\n') MMputchar('\r');
              MMputchar(*p++);
          }
      } else if(oldmark > mark) {
          PositionCursor(mark);
          p = mark;
          while(oldmark > p) {
              if(*p == '\n') MMputchar('\r');
              MMputchar(*p++);
          }
      }

      oldmark = mark; oldx = x; oldy = y;

      // now draw the marked area
      if(mark < txtp) {
          PositionCursor(mark);
            DOSDisplay(REVERSE_VIDEO);                              // reverse video
          p = mark;
          while(p < txtp) {
              if(*p == '\n') MMputchar('\r');
              MMputchar(*p++);
          }
            DOSDisplay(REVERSE_VIDEO);                              // reverse video back to normal on the MX470 display only
      } else if(mark > txtp) {
          PositionCursor(txtp);
            DOSDisplay(REVERSE_VIDEO);                              // reverse video on the MX470 display only
          p = txtp;
          while(p < mark) {
              if(*p == '\n') MMputchar('\r');
              MMputchar(*p++);
          }
            DOSDisplay(REVERSE_VIDEO);                              // reverse video back to normal on the MX470 display only
      }

      oldx = x; oldy = y; oldmark = mark;
      PositionCursor(mark);
  }
}




// search through the text in the editing buffer looking for a specific line
// enters with ln = the line required
// exits pointing to the start of the line or pointing to a zero char if not that many lines in the buffer
char *findLine(int ln) {
  char *p;
  p = EdBuff;
  while(ln && *p) {
      if(*p == '\n') ln--;
      p++;
  }
  return p;
}


int EditCompStr(char *p, char *tkn) {
    while(*tkn && (toupper(*tkn) == toupper(*p))) {
        if(*tkn == '(' && *p == '(') return true;
        if(*tkn == '$' && *p == '$') return true;
        tkn++; p++;
    }
    if(*tkn == 0 && !isnamechar(*p)) return true;                   // return the string if successful

    return false;                                                   // or NULL if not
}


// this function does the syntax colour coding
// p = pointer to the current character to be printed
//     or NULL if the colour coding is to be reset to normal
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

    // this is a list of keywords that can come after the OPTION and GUI commands
    // the list must be terminated with a NULL
    char *twokeywordtbl[] = {
        "BASE", "EXPLICIT", "DEFAULT", "BREAK", "AUTORUN", "BAUDRATE", "DISPLAY",
        NULL
    };

    // this is a list of common keywords that should be highlighted as such
    // the list must be terminated with a NULL
    char *specialkeywords[] = {
        "SELECT", "INTEGER", "FLOAT", "STRING", "DISPLAY", "SDCARD", "OUTPUT", "APPEND", "WRITE", "SLAVE",
        NULL
    };


    // reset everything back to normal
    if(p == NULL) {
        innumber = inquote = inkeyword = incomment = intext = false;
        twokeyword = NULL;
        DOSColour(C_Normal, C_Backgnd);
        return;
    }

    // check for a comment char
    if(*p == '\'' && !inquote) {
        DOSColour(C_Comment, C_Backgnd);
        incomment = true;
        return;
    }

    // once in a comment all following chars must be comments also
    if(incomment) return;

    // check for a quoted string
    if(*p == '\"') {
        if(!inquote) {
            inquote = true;
          DOSColour(C_Quote, C_Backgnd);
            return;
        } else {
            inquote = false;
            return;
        }
    }

    if(inquote) return;

    // if we are displaying a keyword check that it is still actually in the keyword and reset if not
    if(inkeyword) {
        if(isnamechar(*p) || *p == '$') return;
      DOSColour(C_Normal, C_Backgnd);
        inkeyword = false;
        return;
    }

    // if we are displaying a number check that we are still actually in it and reset if not
    // this is complicated because numbers can be in hex or scientific notation
    if(innumber) {
        if(!isdigit(*p) && !(toupper(*p) >= 'A' && toupper(*p) <= 'F') && toupper(*p) != 'O' && toupper(*p) != 'H' && *p != '.') {
            DOSColour(C_Normal, C_Backgnd);
            innumber = false;
            return;
        } else {
            return;
        }
    // check if we are starting a number
    } else if(!intext){
        if(isdigit(*p) || *p == '&' || ((*p == '-' || *p == '+' || *p == '.') && isdigit(p[1]))) {
            DOSColour(C_Number, C_Backgnd);
            innumber = true;
            return;
        }
        // check if this is an 8 digit hex number as used in CFunctions
        for(i = 0; i < 8; i++) if(!isxdigit(p[i])) break;
        if(i == 8 && (p[8] == ' ' || p[8] == '\'' || p[8] == 0)) {
            DOSColour(C_Number, C_Backgnd);
            innumber = true;
            return;
        }
    }

    // check if this is the start of a keyword
    if(isnamestart(*p) && !intext) {
        for(i = 0; i < CommandTableSize - 1; i++) {                 // check the command table for a match
            if(EditCompStr(p, commandtbl[i].name) != NULL) {
                if(EditCompStr(p, "REM") != NULL) {                 // special case, REM is a comment
                    DOSColour(C_Comment, C_Backgnd);
                    incomment = true;
                } else {
                    DOSColour(C_Keyword, C_Backgnd);
                    inkeyword = true;
                    if(EditCompStr(p, "GUI") || EditCompStr(p, "OPTION")) {
                        twokeyword = p;
                        while(isalnum(*twokeyword)) twokeyword++;
                        while(*twokeyword == ' ') twokeyword++;
                    }
                return;
                }
            }
        }
        for(i = 0; i < TokenTableSize - 1; i++) {                   // check the token table for a match
            if(EditCompStr(p, tokentbl[i].name) != NULL) {
                DOSColour(C_Keyword, C_Backgnd);
                inkeyword = true;
                return;
            }
        }

        // check for the second keyword in two keyword commands
        if(p == twokeyword) {
            for(pp = twokeywordtbl; *pp; pp++)
                if(EditCompStr(p, *pp)) break;
            if(*pp) {
                DOSColour(C_Keyword, C_Backgnd);
                inkeyword = true;
                return;
            }
        }
        if(p >= twokeyword) twokeyword = NULL;

        // check for a range of common keywords
        for(pp = specialkeywords; *pp; pp++)
            if(EditCompStr(p, *pp)) break;
        if(*pp) {
            DOSColour(C_Keyword, C_Backgnd);
            inkeyword = true;
            return;
        }
    }

    // try to keep track of if we are in general text or not
    // this is to avoid recognising keywords or numbers inside variables
    if(isnamechar(*p)) {
        intext = true;
    } else {
        intext = false;
        DOSColour(C_Normal, C_Backgnd);
    }

}


// print a line starting at the current column (edx) at the current cursor.
// if the line is beyond the end of the text then just clear to the end of line
// enters with the line number to be printed
void printLine(int ln) {
  char *p;
  int i;

    MMputchar('\r');
    p = findLine(ln);
    i = VWidth;
    while(i && *p && *p != '\n') {
        SetColour(p, false);
        MMputchar(*p++);
        i--;
    }
    DOSDisplay(CLEAR_TO_EOL);
    SetColour(NULL, false);
  curx = VWidth - 1;
}



// print a full screen starting with the top left corner specified by edx, edy
// this draws the full screen including blank areas so there is no need to clear the screen first
// it then returns the cursor to its original position
void printScreen(void) {
  int i;

  SCursor(0, 0);
  for(i = 0; i < VHeight; i++) {
      printLine(i + edy);
        MMPrintString("\r\n");
      curx = 0;
      cury = i + 1;
  }
  while(MMInkey() != -1);                                           // consume any keystrokes accumulated while redrawing the screen
}



// position the cursor on the screen
void SCursor(int x, int y) {
    DOSCursor(x, y);
  curx = x; cury = y;
}



// move the text down by one char starting at the current position in the text
// and insert a character
int editInsertChar(char c) {
  char *p;

  for(p = EdBuff; *p; p++);                                         // find the end of the text in memory
  if(p >= EdBuff + PROG_FLASH_SIZE - 10) {                          // and check that we have the space (allow 10 bytes for slack)
      editDisplayMsg(" OUT OF MEMORY ");
      return false;
  }
  for(; p >= txtp; p--) *(p + 1) = *p;                              // shift everything down
  *txtp++ = c;                                                      // and insert our char
  return true;
}



// print the function keys at the bottom of the screen
void PrintFunctKeys(int typ) {
  int i, x, y;
  char *p;

  if(typ == EDIT) {
          p = "ESC:Exit  F1:Save  F2:Run  F3:Find  F4:Mark  F5:Paste";
  } else {
          p = "MARK MODE   ESC=Exit  DEL:Delete  F4:Cut  F5:Copy";
  }
  DOSCursor(0, Option.Height - 2);
    DOSDisplay(DRAW_LINE);                                          // draw the line
  DOSCursor(0, Option.Height - 1);
    DOSPutS(p, C_Status, C_Backgnd);
  DOSColour(C_Normal, C_Backgnd);
    DOSDisplay(CLEAR_TO_EOL);                                       // clear to the end of line on the MX470 display only
  SCursor(curx, cury);
}



// print the current status
void PrintStatus(void) {
    int tx;
  char s[MAXSTRLEN];

  DOSColour(C_Normal, C_Backgnd);
    DOSCursor(53, Option.Height - 1);
    DOSDisplay(CLEAR_TO_EOL);                                       // clear to the end of line on the MX470 display only
    tx = edx + curx + 1;
    strcpy(s, "Ln: ");
    IntToStr(s + strlen(s), edy + cury + 1, 10);
    strcat(s + strlen(s), "  Col: ");
    IntToStr(s + strlen(s), tx, 10);
    strcat(s, insert?"   INS":"   OVR");

    DOSCursor((VWidth - strlen(s)), Option.Height - 1);
    DOSPutS(s, C_Status, C_Backgnd);                                // display the string on the display attached to the MX470

  DOSColour(C_Normal, C_Backgnd);
  PositionCursor(txtp);
}



// display a message in the status line
void editDisplayMsg(char *msg) {
  SCursor(0, Option.Height - 1);
  DOSPutS(msg, C_Backgnd, BRED);
  DOSColour(C_Normal, C_Backgnd);
    DOSDisplay(CLEAR_TO_EOL);                                       // clear to the end of line on the MX470 display only
  PositionCursor(txtp);
  drawstatusline = true;
}



void SaveToRAM(char *EdBuff) {
  char *p = ProgMemory, *pp;
    while(*EdBuff) {
      for(pp = inpbuf; !(*EdBuff == 0 || *EdBuff == '\n'); EdBuff++, pp++) *pp = *EdBuff;
      *pp = 0;
      tokenise(false);
      for(pp = tknbuf; !(pp[0] == 0 && pp[1] == 0); p++, pp++) {
            if(p > ProgMemory + PROG_FLASH_SIZE - 3) {
                DOSColour(C_Normal, C_Backgnd);
                system("CLS");
              error("Not enough memory");
            }
          *p = *pp;
        }
      *p++ = 0;                                                     // write the terminating zero char for the line
      if(*EdBuff == '\n') EdBuff++;
    }
    *p++ = 0; *p = 0;                                               // two zeros terminate the program but as an extra just in case
}



void SaveToFile(void) {
    char b[STRINGSIZE];
    char *p;
    FILE *f;

  if(*CurrentFile == 0) {
        GetInputString("Filename to save to (ENTER = no file): ");
      if(*inpbuf == 0)
          *CurrentFile = 1;
      else {
          strcpy(CurrentFile, inpbuf);
          if(strchr(CurrentFile, '.') == NULL) strcat(CurrentFile, ".bas");
          strcpy(inpbuf, "MMBasic - "); strcat(inpbuf, CurrentFile);
          SetConsoleTitle(inpbuf);
          *inpbuf = 0;
      }
  }
  if(*CurrentFile == 1) return;                                     // user does not want to save to a file

    errno = 0;
    f = fopen(CurrentFile, "wb");
    if(errno) error("Cannot write to $", CurrentFile);

    p = ProgMemory;
    while(!(*p == 0 || *p == 0xff)) {                               // normally a LIST ends at the break so this is a safety precaution
      if(*p == T_LINENBR || *p == T_NEWLINE) {
          p = llist(b, p);                                          // otherwise expand the line
          if(!(p[0] == 0 && p[1] == 0)) strcat(b, "\r\n");
            fwrite(b, strlen(b), 1, f);
              if(errno) error("Cannot write to $", CurrentFile);
          if(p[0] == 0 && p[1] == 0) break;                         // end of the program ?
      }
    }
    fclose(f);
}



// save the program in the editing buffer into the program memory
void SaveToProgMemory(void) {
    ClearProgram();
  StartEditPoint = (char *)(edy + cury);                            // record our position in case the editor is invoked again
    StartEditChar = edx + curx;
    // bugfix for when the edit point is a space
    while(StartEditChar > 0 && txtp > EdBuff && *(--txtp) == ' ') {
        StartEditChar--;
    }
    SaveToRAM(EdBuff);
    SaveToFile();
}



// get an input string from the user and save into inpbuf
void GetInputString(char *prompt) {
  int i;
  char *p;

  SCursor(0, Option.Height - 1);
  DOSPutS(prompt, C_Normal, C_Backgnd);
  DOSDisplay(CLEAR_TO_EOL);                                         // clear to the end of line on the MX470 display only
  SCursor(strlen(prompt), Option.Height - 1);
  for(p = inpbuf; (*p = MMgetchar()) != '\r'; p++) {                // get the input
      if(*p == 0xb3 || *p == F3 || *p == ESC) { p++; break; }       // return if it is SHIFT-F3, F3 or ESC
      if(isprint(*p)) {
            MMputchar(*p);                                          // echo the char
        }
      if(*p == '\b') {
          p--;                                                      // backspace over a backspace
          if(p >= inpbuf){
              p--;                                                  // and the char before
              DOSPutS("\b \b", C_Normal, C_Backgnd);                // erase on the MX470 display
          }
      }
  }
  *p = 0;                                                           // terminate the input string
  PrintFunctKeys(EDIT);
  PositionCursor(txtp);
}

// scroll up the video screen
void Scroll(void) {
  edy++;
  printScreen();
  //DOSCursor(VWidth - 1, Option.Height - 1);
  //MMputchar('\n');
 //   SCursor(0, VHeight);
  //curx = 0;
  //cury = VHeight - 1;
  //PrintFunctKeys(EDIT);
  //printLine(VHeight - 1 + edy);
  //PositionCursor(txtp);
  //while(MMInkey() != -1);                                         // consume any keystrokes accumulated while redrawing the screen
}


// scroll down the video screen
void ScrollDown(void) {
  edy--;
  printScreen();
  //printLine(edy);
  //PrintFunctKeys(EDIT);
  //PositionCursor(txtp);
}

