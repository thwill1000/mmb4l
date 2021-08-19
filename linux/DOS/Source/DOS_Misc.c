/***********************************************************************************************************************
MMBasic

Misc.c

Handles all the miscelaneous commands and functions in DOS MMBasic.  These are commands and functions that do not
comfortably fit anywhere else.

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


#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <wincon.h>
#include <process.h>
#include <tchar.h>
#include <conio.h>
#include <strsafe.h>

#include "..\..\Version.h"

extern char CurrentFile[STRINGSIZE];

void fun_timer(void) {
    iret = (long long int)(clock() - mSecTimer);
    targ = T_INT;
}

void fun_hres(void) {
    GetConsoleSize();
    iret = Option.Width;
    targ = T_INT;
}

void fun_vres(void) {
    GetConsoleSize();
    iret = Option.Height;
    targ = T_INT;
}

// this is invoked as a command (ie, TIMER = 0)
// search through the line looking for the equals sign and step over it,
// evaluate the rest of the command and save in the timer
void cmd_timer(void) {
    while (*cmdline && *cmdline != tokenEQUAL) cmdline++;
    if (!*cmdline) error("Invalid syntax");
    mSecTimer = clock() - getinteger(++cmdline);
}

void cmd_pause(void) {
    int i;
    i = clock() + getinteger(cmdline);
    while (!MMAbort && clock() < i)
        ;
}

void fun_date(void) {
    time_t time_of_day;
    struct tm *tmbuf;

    time_of_day = time(NULL);
    tmbuf = localtime(&time_of_day);
    sret = GetTempStrMemory();  // this will last for the life of the command
    sprintf(sret, "%02d-%02d-%04d", tmbuf->tm_mday, tmbuf->tm_mon + 1,
            tmbuf->tm_year + 1900);
    CtoM(sret);
    targ = T_STR;
}

void fun_time(void) {
    time_t time_of_day;
    struct tm *tmbuf;

    time_of_day = time(NULL);
    tmbuf = localtime(&time_of_day);
    sret = GetTempStrMemory();  // this will last for the life of the command
    sprintf(sret, "%02d:%02d:%02d", tmbuf->tm_hour, tmbuf->tm_min,
            tmbuf->tm_sec);
    CtoM(sret);
    targ = T_STR;
}

// function (which looks like a pre defined variable) to return the type of
// platform
void fun_type(void) {
    sret = GetTempStrMemory();  // this will last for the life of the command
    strcpy(sret, "\3DOS");
    targ = T_STR;
}

void fun_cmdline(void) {
    sret = GetTempStrMemory();  // this will last for the life of the command
    _bgetcmd(sret, MAXSTRLEN);
    CtoM(sret);
    targ = T_STR;
}

void cmd_exitmmb(void) {
    checkend(cmdline);
    ExitMMBasicFlag = true;  // signal that we want out of here
    longjmp(mark, 1);        // jump back to the input prompt
}

void cmd_system(void) {
    int rc;

    rc = system(getCstring(cmdline));
    if (rc != 0) {
        error("Command could not be run");
    }
}

void cmd_cls(void) {
    int rc;

    checkend(cmdline);
    rc = system("CLS");
    if (rc != 0) {
        error("Command could not be run");
    }
}

void cmd_cursor(void) {
    getargs(&cmdline, 3,
            ",");  // getargs macro must be the first executable stmt in a block
    if (argc != 3) error("Syntax");
    DOSCursor(getinteger(argv[0]), getinteger(argv[2]));
}

void cmd_colour(void) {
    int fc, bc;
    getargs(&cmdline, 3,
            ",");  // getargs macro must be the first executable stmt in a block
    if (argc != 3) error("Syntax");
    fc = getint(argv[0], 0, 0x0f);
    bc = getint(argv[2], 0, 0x0f);
    DOSColour(fc, bc);
}

void cmd_settitle(void) { SetConsoleTitle(getCstring(cmdline)); }

void cmd_option(void) {
    char *tp;

    tp = checkstring(cmdline, "BASE");
    if (tp) {
        if (DimUsed) error("Must be before DIM or LOCAL");
        OptionBase = getint(tp, 0, 1);
        return;
    }

    tp = checkstring(cmdline, "EXPLICIT");
    if (tp) {
        OptionExplicit = true;
        return;
    }

    tp = checkstring(cmdline, "DEFAULT");
    if (tp) {
        if (checkstring(tp, "INTEGER")) {
            DefaultType = T_INT;
            return;
        }
        if (checkstring(tp, "FLOAT")) {
            DefaultType = T_NBR;
            return;
        }
        if (checkstring(tp, "STRING")) {
            DefaultType = T_STR;
            return;
        }
        if (checkstring(tp, "NONE")) {
            DefaultType = T_NOTYPE;
            return;
        }
    }

    tp = checkstring(cmdline, "CASE");
    if (tp) {
        if (checkstring(tp, "LOWER")) {
            Option.Listcase = CONFIG_LOWER;
            SaveOptions();
            return;
        }
        if (checkstring(tp, "UPPER")) {
            Option.Listcase = CONFIG_UPPER;
            SaveOptions();
            return;
        }
        if (checkstring(tp, "TITLE")) {
            Option.Listcase = CONFIG_TITLE;
            SaveOptions();
            return;
        }
    }

    tp = checkstring(cmdline, "TAB");
    if (tp) {
        if (checkstring(tp, "2")) {
            Option.Tab = 2;
            SaveOptions();
            return;
        }
        if (checkstring(tp, "4")) {
            Option.Tab = 4;
            SaveOptions();
            return;
        }
        if (checkstring(tp, "8")) {
            Option.Tab = 8;
            SaveOptions();
            return;
        }
    }

    error("Option");
}

void cmd_wedit(void) {
    int rc, del = false;
    char b[STRINGSIZE];
    char fname[STRINGSIZE];
    char *p;
    FILE *f;

    if (CurrentLinePtr) error("Invalid in a program");
    if (*CurrentFile > 1) {
        strcpy(fname, CurrentFile);
    } else {
        strcpy(fname, getenv("TEMP"));
        strcat(fname, "\\MMBasic.tmp");
        f = fopen(fname, "wb");
        if (errno) error("Cannot write to $", fname);

        p = ProgMemory;
        while (!(*p == 0 || *p == 0xff)) {  // this is a safety precaution
            if (*p == T_LINENBR || *p == T_NEWLINE) {
                p = llist(b, p);  // otherwise expand the line
                if (!(p[0] == 0 && p[1] == 0)) strcat(b, "\r\n");
                fwrite(b, strlen(b), 1, f);
                if (errno) error("Cannot write to $", fname);
                if (p[0] == 0 && p[1] == 0) break;  // end of the program ?
            }
        }
        fclose(f);
        del = true;
    }

    strcpy(b, "\"");
    if (getenv("MMEDITOR") != NULL)
        strcat(b, getenv("MMEDITOR"));
    else
        strcat(b, "Notepad");
    strcat(b, " \"");
    strcat(b, fname);
    strcat(b, "\"\"");  //"
    rc = system(b);
    if (rc != 0) {
        error("Editor could not be run");
    }

    if (!FileLoadProgram(fname)) error("Could not read from $", fname);
    if (del) {
        SetConsoleTitle("MMBasic - Untitled");
        remove(fname);
    }
}
