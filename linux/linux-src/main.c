/***********************************************************************************************************************
MMBasic

Main.c

The startup code and essential functions for the DOS version of MMBasic.

Copyright 2011 - 2020 Geoff Graham.  All Rights Reserved.

This file and modified versions of this file are supplied to specific
individuals or organisations under the following provisions:

- This file, or any files that comprise the MMBasic source (modified or not),
may not be distributed or copied to any other person or organisation without
written permission.

- Object files (.o and .hex files) generated using this file (modified or not)
may not be distributed or copied to any other person or organisation without
written permission.

- This file is provided in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.

************************************************************************************************************************/

#include <signal.h>
#include <stdio.h>
#include <time.h>

#include "common/console.h"
#include "common/version.h"

// the values returned by the standard control keys
#define TAB 0x9
#define BKSP 0x8
#define ENTER 0xd
#define ESC 0x1b

// the values returned by the function keys
#define F1 0x91
#define F2 0x92
#define F3 0x93
#define F4 0x94
#define F5 0x95
#define F6 0x96
#define F7 0x97
#define F8 0x98
#define F9 0x99
#define F10 0x9a
#define F11 0x9b
#define F12 0x9c

// the values returned by special control keys
#define UP 0x80
#define DOWN 0x81
#define LEFT 0x82
#define RIGHT 0x83
#define INSERT 0x84
#define DEL 0x7f
#define HOME 0x86
#define END 0x87
#define PUP 0x88
#define PDOWN 0x89
#define NUM_ENT ENTER
#define SLOCK 0x8c
#define ALT 0x8b

// global variables used in MMBasic but must be maintained outside of the
// interpreter
int ListCnt;
int MMCharPos;
long long int mSecTimer;
int ExitMMBasicFlag = false;
volatile int MMAbort = false;
char *InterruptReturn = NULL;
struct option_s Option;
int WatchdogSet, IgnorePIN, InterruptUsed;
char *OnKeyGOSUB;
char *CFunctionFlash, *CFunctionLibrary, **FontTable;

int ErrorInPrompt = false;

void IntHandler(int signo);
int LoadFile(char *prog);
void dump_token_table(const struct s_tokentbl* tbl);

int main(int argc, char *argv[]) {
    static int PromptError = false;
    int RunCommandLineProgram = false;
    char *p;

    // get things setup to act like the Micromite version
    vartbl = &DOS_vartbl;
    ProgMemory[0] = ProgMemory[1] = ProgMemory[2] = 0;
    Option.ProgFlashSize = PROG_FLASH_SIZE;
    Option.Tab = 4;
    cmdCFUN = cmdCSUB = cmdIRET = 0xff;

    InitHeap();  // init memory allocation

    console_get_size();
    console_set_title("MMBasic - Untitled");

    console_clear();
    MMPrintString(MES_SIGNON);  // print signon message
    MMPrintString(COPYRIGHT);   // print signon message
    MMPrintString("Linux port by Thomas Hugo Williams, 2021");
    MMPrintString("\r\n");
    OptionErrorSkip = 0;
    InitBasic();

    //printf("Commands\n--------\n");
    //dump_token_table(commandtbl);
    //printf("\n");
    //printf("Tokens\n--------\n");
    //dump_token_table(tokentbl);

# if 0
    signal(SIGBREAK, IntHandler);  // CTRL-C handler
    signal(SIGINT, IntHandler);
#endif

    mSecTimer = clock();  // used for TIMER
    srand(0);             // seed the random generator with zero

    // if there is something on the command line try to load it as a program, if
    // that fails try AUTORUN.BAS
    if (argc > 1) RunCommandLineProgram = LoadFile(argv[1]);
    if (!RunCommandLineProgram) RunCommandLineProgram = LoadFile("AUTORUN.BAS");
    if (!RunCommandLineProgram)
        RunCommandLineProgram = LoadFile("C:\\AUTORUN.BAS");

    if (getenv("MMDIR") != NULL) chdir(GetEnvPath("MMDIR"));

    if (setjmp(mark) != 0) {
        // we got here via a long jump which means an error or CTRL-C or the
        // program wants to exit to the command prompt
        if (ExitMMBasicFlag)
            return 0;  // program has executed an ExitMMBasic command
        ContinuePoint =
            nextstmt;  // in case the user wants to use the continue command
        *tknbuf = 0;   // we do not want to run whatever is in the token buffer
        RunCommandLineProgram = false;  // nor the program on the command line
    }

    if (RunCommandLineProgram) {
        RunCommandLineProgram = false;
        ClearRuntime();
        PrepareProgram(true);
        ExecuteProgram(ProgMemory);  // if AUTORUN.BAS or something is on the
                                     // command line, run it
    }

    while (1) {
        GetConsoleSize();
        MMAbort = false;
        LocalIndex = 0;     // this should not be needed but it ensures that all
                            // space will be cleared
        ClearTempMemory();  // clear temp string space (might have been used by
                            // the prompt)
        CurrentLinePtr = NULL;  // do not use the line number in error reporting
        if (MMCharPos > 1)
            MMPrintString("\r\n");  // prompt should be on a new line
        PrepareProgram(false);
        if (!ErrorInPrompt && FindSubFun("MM.PROMPT", 0) >= 0) {
            ErrorInPrompt = true;
            ExecuteProgram("MM.PROMPT\0");
        } else
            MMPrintString("> ");  // print the prompt
        ErrorInPrompt = false;
        MMgetline(0, inpbuf);    // get the input
        if (!*inpbuf) continue;  // ignore an empty line
        tokenise(true);          // turn into executable code
        if (*tknbuf ==
            T_LINENBR)  // don't let someone use line numbers at the prompt
            tknbuf[0] = tknbuf[1] = tknbuf[2] =
                ' ';            // convert the line number into spaces
        CurrentLinePtr = NULL;  // do not use the line number in error reporting
        //printf("tknbuf = %s\n", tknbuf);
        ExecuteProgram(tknbuf);  // execute the line straight away
    }
}

void IntHandler(int signo) {
#if 0
    signal(SIGBREAK, IntHandler);
    signal(SIGINT, IntHandler);
#endif
    MMAbort = true;
}

int LoadFile(char *prog) {
#if 0
    FILE *f;
    char buf[STRINGSIZE];
    f = fopen(prog, "rb");
    if (f != NULL) {
        fclose(f);
        buf[0] = '"';
        strcpy(&buf[1], prog);
        strcat(buf, "\"");
        FileLoadProgram(buf);
        if (*ProgMemory == T_NEWLINE ||
            *ProgMemory == T_LINENBR) {  // is there a program to run?
            return true;
        }
    }
#endif
    return false;
}

void GetConsoleSize(void) {
#if 0
    CONSOLE_SCREEN_BUFFER_INFO consoleinfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleinfo);
    Option.Height = consoleinfo.srWindow.Bottom - consoleinfo.srWindow.Top;
    Option.Width = consoleinfo.srWindow.Right - consoleinfo.srWindow.Left;
#endif
    Option.Height = 40;
    Option.Width = 80;
}

char *GetEnvPath(char *env) {
    char *p;
    p = getenv(env);
    if (p == NULL) return NULL;
    if (*p == '\"') {
        p++;
        if (p[strlen(p) - 1] == '\"') p[strlen(p) - 1] = 0;
    }
    return p;
}

void DOSCursor(int x, int y) {
#if 0
    COORD coord;
    CONSOLE_SCREEN_BUFFER_INFO consoleinfo;
    if (x < 0 || y < 0) return;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &consoleinfo);
    Option.Height = consoleinfo.srWindow.Bottom - consoleinfo.srWindow.Top;
    Option.Width = consoleinfo.srWindow.Right - consoleinfo.srWindow.Left;
    coord.X = x + consoleinfo.srWindow.Left;
    coord.Y = y + consoleinfo.srWindow.Top;
    if (coord.X > consoleinfo.srWindow.Right ||
        coord.Y > consoleinfo.srWindow.Bottom)
        return;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    MMCharPos = x + 1;
#endif
}

void DOSColour(int fc, int bc) {
#if 0
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fc | bc << 4);
#endif
}

void FlashWriteInit(char *p, int nbr) {
    ProgMemory[0] = ProgMemory[1] = ProgMemory[2] = 0;
    console_set_title("MMBasic - Untitled");
    CurrentFile[0] = 0;
}

// get a char from the DOS console and convert function keys to MMBasic keycodes
int DOSgetch(void) {
#if 0
    int c;
    char s;
    c = getch();   // get the first character of a possible multibyte function
                   // key
    if (c == 0) {  // keypress is a special key
        s = getch();
        if (s == 28)
            c = '\r';  // numeric enter key
        else if (s >= 0x3B && s <= 0x44)
            c = F1 + (s - 0x3B);
        else if (s == 0x57)
            c = F11;
        else if (s == 0x58)
            c = F12;
        else if (s == 0x35)
            c = '/';
        else if (s == 0x52)
            c = INSERT;
        else if (s == 0x47)
            c = HOME;
        else if (s == 0x4C)
            c = 0x35;
        else if (s == 0x4F)
            c = END;
        else if (s == 0x49)
            c = PUP;
        else if (s == 0x51)
            c = PDOWN;
        else if (s == 0x53)
            c = DEL;
        else if (s == 0x48)
            c = UP;
        else if (s == 0x50)
            c = DOWN;
        else if (s == 0x4B)
            c = LEFT;
        else if (s == 0x4D)
            c = RIGHT;
        else
            c = -1;
    }
    return c;
#endif
    return 0;
}

// get a character from the console
// returns -1 if nothing there
int MMInkey(void) {
#if 0
    CheckAbort();
    if (kbhit())
        return DOSgetch();
    else
        return -1;
#endif
    return -1;
}

void CheckAbort(void) {
    if (MMAbort) longjmp(mark, 1);  // jump back to the input prompt
}

// get a keystroke.  Will wait forever for input
// if the char is a lf then replace it with a cr
// unless it was preceded by a cr and in that case throw away the char
// so console end of line is always cr
int MMgetchar(void) {
//     int c;
//     static char prevchar = 0;
//     DWORD mode;
//     GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);

// loopback:
//     SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
//                    mode & ~ENABLE_PROCESSED_INPUT);
//     c = DOSgetch();
//     SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);
//     if (c == 3) longjmp(mark, 1);  // jump back to the input prompt if CTRL-C
//     if (c == '\n' && prevchar == '\r') {
//         prevchar = 0;
//         goto loopback;
//     }
//     prevchar = c;
//     if (c == '\n') c = '\r';
//     return c;

    static char prevchar = 0;
    int c;
    for (;;) {
        c = getc(stdin);
        if (c == '\n' && prevchar == '\r') {
            prevchar = 0;
        } else {
            break;
        }
    }
    prevchar = c;
    return c == '\n' ? '\r' : c;
}

// put a character out to the operating system
char MMputchar(char c) {
    // putch(c);
    putc(c, stdout);
    if (isprint(c)) MMCharPos++;
    if (c == '\r' || c == '\n') {
        MMCharPos = 1;
        ListCnt++;
    }
    return c;
}

// get a line from the keyboard or a file handle
void MMgetline(int filenbr, char *p) {
    int c, nbrchars = 0;
    char *tp;

    while (1) {
        CheckAbort();                // jump right out if CTRL-C
        if (MMfeof(filenbr)) break;  // end of file - stop collecting
        c = MMfgetc(filenbr);

        // if this is the console, check for a programmed function key and
        // insert the text
        if (filenbr == 0) {
            tp = NULL;
            if (c == F2) tp = "RUN";
            if (c == F3) tp = "LIST";
            if (c == F4) tp = "EDIT";
            if (c == F5) tp = "WEDIT";
            if (tp) {
                strcpy(p, tp);
                MMPrintString(tp);
                MMPrintString("\r\n");
                return;
            }
        }

        if (c == '\t') {  // expand tabs to spaces
            do {
                if (++nbrchars > MAXSTRLEN) error("Line is too long");
                *p++ = ' ';
                if (filenbr == 0) MMputchar(' ');
            } while (nbrchars % Option.Tab);
            continue;
        }

        if (c == '\b') {  // handle the backspace
            if (nbrchars) {
                if (filenbr == 0) MMPrintString("\b \b");
                nbrchars--;
                p--;
            }
            continue;
        }

        if (c == '\n') {  // what to do with a newline
            break;        // a newline terminates a line (for a file)
        }

        if (c == '\r') {
            if (filenbr == 0) {
                MMPrintString("\r\n");
                break;  // on the console this meand the end of the line - stop
                        // collecting
            } else
                continue;  // for files loop around looking for the following
                           // newline
        }

        if (isprint(c)) {
            if (filenbr == 0)
                MMputchar(c);  // The console requires that chars be echoed
        }

        if (++nbrchars > MAXSTRLEN)
            error("Line is too long");  // stop collecting if maximum length
        *p++ = c;                       // save our char
    }
    *p = 0;

    //printf("%s", p);
}

// check if an interrupt has occured
// DOS does not use interrupts
int check_interrupt(void) { return 0; }

// dump a memory area to the console
// for debugging
void dump(char *p, int nbr) {
    char buf1[80], buf2[80], buf3[80], *b1, *b2, *pt;
    b1 = buf1;
    b2 = buf2;
    MMPrintString(
        "   addr    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    "
        "0123456789ABCDEF\r\n");
    b1 += sprintf(b1, "%8x: ", (unsigned int)p);
    for (pt = p; (unsigned int)pt % 16 != 0; pt--) {
        b1 += sprintf(b1, "   ");
        b2 += sprintf(b2, " ");
    }
    while (nbr > 0) {
        b1 += sprintf(b1, "%02x ", *p);
        b2 += sprintf(b2, "%c", (*p >= ' ' && *p < 0x7f) ? *p : '.');
        p++;
        nbr--;
        if ((unsigned int)p % 16 == 0) {
            MMPrintString(buf1);
            MMPrintString("   ");
            MMPrintString(buf2);
            b1 = buf1;
            b2 = buf2;
            b1 += sprintf(b1, "\r\n%8x: ", (unsigned int)p);
        }
    }
    if (b2 != buf2) {
        MMPrintString(buf1);
        MMPrintString("   ");
        for (pt = p; (unsigned int)pt % 16 != 0; pt++) {
            MMPrintString("   ");
        }
        MMPrintString(buf2);
    }
    MMPrintString("\r\n");
}

void dump_token_table(const struct s_tokentbl* tbl) {
    int i = 0;
    for (;;) {
        printf("%3d:  %-15s, %5d, %5d, 0x%8lX\n", i, tbl[i].name, tbl[i].type, tbl[i].precedence, (unsigned long int) tbl[i].fptr);
        if (*(tbl[i].name) == 0) break;
        i++;
    }
}
