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
//#include <windows.h>
//#include <wincon.h>
//#include <process.h>
//#include <tchar.h>
//#include <conio.h>
//#include <strsafe.h>

#include "../../Version.h"
#include "common/console.h"
//#include "option.h"
#include "common/utility.h"

#define IsDigitinline(a)	( a >= '0' && a <= '9' )

extern char CurrentFile[STRINGSIZE];

void fun_format(void) {
    char *p, *fmt;
    int inspec;
    getargs(&ep, 3, ",");
    if (argc % 2 == 0) error("Invalid syntax");
    if (argc == 3)
        fmt = getCstring(argv[2]);
    else
        fmt = "%g";

    // check the format string for errors that might crash the CPU
    for (inspec = 0, p = fmt; *p; p++) {
        if (*p == '%') {
            inspec++;
            if (inspec > 1) error("Only one format specifier (%) allowed");
            continue;
        }

        if (inspec == 1 && (*p == 'g' || *p == 'G' || *p == 'f' || *p == 'e' ||
                            *p == 'E' || *p == 'l'))
            inspec++;

        if (inspec == 1 && !(IsDigitinline(*p) || *p == '+' || *p == '-' ||
                             *p == '.' || *p == ' '))
            error("Illegal character in format specification");
    }
    if (inspec != 2) error("Format specification not found");
    sret = GetTempStrMemory();  // this will last for the life of the command
    sprintf(sret, fmt, getnumber(argv[0]));
    CtoM(sret);
    targ = T_STR;
}

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
    ERROR_UNIMPLEMENTED("file_io_stubs#fun_cmdline");
#if 0
    sret = GetTempStrMemory();  // this will last for the life of the command
    _bgetcmd(sret, MAXSTRLEN);
    CtoM(sret);
    targ = T_STR;
#endif
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
    // int rc;

    checkend(cmdline);
    clear_console();
    // rc = system("CLS");
    // if (rc != 0) {
    //     error("Command could not be run");
    // }
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

void cmd_settitle(void) {
    set_console_title(getCstring(cmdline));
}

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

void integersort(int64_t *iarray, int n, long long *index, int flags,
                 int startpoint) {
    int i, j = n, s = 1;
    int64_t t;
    if ((flags & 1) == 0) {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (iarray[i] < iarray[i - 1]) {
                    t = iarray[i];
                    iarray[i] = iarray[i - 1];
                    iarray[i - 1] = t;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    } else {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (iarray[i] > iarray[i - 1]) {
                    t = iarray[i];
                    iarray[i] = iarray[i - 1];
                    iarray[i - 1] = t;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    }
}

void floatsort(MMFLOAT *farray, int n, long long *index, int flags,
               int startpoint) {
    int i, j = n, s = 1;
    int64_t t;
    MMFLOAT f;
    if ((flags & 1) == 0) {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (farray[i] < farray[i - 1]) {
                    f = farray[i];
                    farray[i] = farray[i - 1];
                    farray[i - 1] = f;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    } else {
        while (s) {
            s = 0;
            for (i = 1; i < j; i++) {
                if (farray[i] > farray[i - 1]) {
                    f = farray[i];
                    farray[i] = farray[i - 1];
                    farray[i - 1] = f;
                    s = 1;
                    if (index != NULL) {
                        t = index[i - 1 + startpoint];
                        index[i - 1 + startpoint] = index[i + startpoint];
                        index[i + startpoint] = t;
                    }
                }
            }
            j--;
        }
    }
}

void stringsort(unsigned char *sarray, int n, int offset, long long *index,
                int flags, int startpoint) {
    int ii, i, s = 1, isave;
    int k;
    unsigned char *s1, *s2, *p1, *p2;
    unsigned char temp;
    int reverse = 1 - ((flags & 1) << 1);
    while (s) {
        s = 0;
        for (i = 1; i < n; i++) {
            s2 = i * offset + sarray;
            s1 = (i - 1) * offset + sarray;
            ii = *s1 < *s2 ? *s1 : *s2;  // get the smaller  length
            p1 = s1 + 1;
            p2 = s2 + 1;
            k = 0;  // assume the strings match
            while ((ii--) && (k == 0)) {
                if (flags & 2) {
                    if (toupper(*p1) > toupper(*p2)) {
                        k = reverse;  // earlier in the array is bigger
                    }
                    if (toupper(*p1) < toupper(*p2)) {
                        k = -reverse;  // later in the array is bigger
                    }
                } else {
                    if (*p1 > *p2) {
                        k = reverse;  // earlier in the array is bigger
                    }
                    if (*p1 < *p2) {
                        k = -reverse;  // later in the array is bigger
                    }
                }
                p1++;
                p2++;
            }
            // if up to this point the strings match
            // make the decision based on which one is shorter
            if (k == 0) {
                if (*s1 > *s2) k = reverse;
                if (*s1 < *s2) k = -reverse;
            }
            if (k == 1) {  // if earlier is bigger swap them round
                ii = *s1 > *s2 ? *s1 : *s2;  // get the bigger length
                ii++;
                p1 = s1;
                p2 = s2;
                while (ii--) {
                    temp = *p1;
                    *p1 = *p2;
                    *p2 = temp;
                    p1++;
                    p2++;
                }
                s = 1;
                if (index != NULL) {
                    isave = index[i - 1 + startpoint];
                    index[i - 1 + startpoint] = index[i + startpoint];
                    index[i + startpoint] = isave;
                }
            }
            // routinechecks(1);
        }
    }
}

void cmd_sort(void) {
    void *ptr1 = NULL;
    void *ptr2 = NULL;
    MMFLOAT *a3float = NULL;
    int64_t *a3int = NULL, *a4int = NULL;
    unsigned char *a3str = NULL;
    int i, size, truesize, flags = 0, maxsize = 0, startpoint = 0;
    getargs(&cmdline, 9, ",");
    ptr1 = findvar(argv[0], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
    if (vartbl[VarIndex].type & T_NBR) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
            error("Argument 1 must be array");
        }
        a3float = (MMFLOAT *)ptr1;
    } else if (vartbl[VarIndex].type & T_INT) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
            error("Argument 1 must be array");
        }
        a3int = (int64_t *)ptr1;
    } else if (vartbl[VarIndex].type & T_STR) {
        if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
        if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
            error("Argument 1 must be array");
        }
        a3str = (unsigned char *)ptr1;
        maxsize = vartbl[VarIndex].size;
    } else
        error("Argument 1 must be array");
    //if ((uint32_t)ptr1 != (uint32_t)vartbl[VarIndex].val.s)
    if (ptr1 != vartbl[VarIndex].val.s)
        error("Argument 1 must be array");
    truesize = size = (vartbl[VarIndex].dims[0] - OptionBase);
    if (argc >= 3 && *argv[2]) {
        ptr2 = findvar(argv[2], V_FIND | V_EMPTY_OK | V_NOFIND_ERR);
        if (vartbl[VarIndex].type & T_INT) {
            if (vartbl[VarIndex].dims[1] != 0) error("Invalid variable");
            if (vartbl[VarIndex].dims[0] <= 0) {  // Not an array
                error("Argument 2 must be integer array");
            }
            a4int = (int64_t *)ptr2;
        } else
            error("Argument 2 must be integer array");
        if ((vartbl[VarIndex].dims[0] - OptionBase) != size)
            error("Arrays should be the same size");
        // if ((uint32_t)ptr2 != (uint32_t)vartbl[VarIndex].val.s)
        if (ptr2 != vartbl[VarIndex].val.s)
            error("Argument 2 must be array");
    }
    if (argc >= 5 && *argv[4]) flags = getint(argv[4], 0, 3);
    if (argc >= 7 && *argv[6])
        startpoint = getint(argv[6], OptionBase, size + OptionBase);
    size -= startpoint;
    if (argc == 9) size = getint(argv[8], 1, size + 1 + OptionBase) - 1;
    if (startpoint) startpoint -= OptionBase;
    if (a3float != NULL) {
        a3float += startpoint;
        if (a4int != NULL)
            for (i = 0; i < truesize + 1; i++) a4int[i] = i + OptionBase;
        floatsort(a3float, size + 1, a4int, flags, startpoint);
    } else if (a3int != NULL) {
        a3int += startpoint;
        if (a4int != NULL)
            for (i = 0; i < truesize + 1; i++) a4int[i] = i + OptionBase;
        integersort(a3int, size + 1, a4int, flags, startpoint);
    } else if (a3str != NULL) {
        a3str += ((startpoint) * (maxsize + 1));
        if (a4int != NULL)
            for (i = 0; i < truesize + 1; i++) a4int[i] = i + OptionBase;
        stringsort(a3str, size + 1, maxsize + 1, a4int, flags, startpoint);
    }
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

    // Launch an external editor.
    char *mmeditor = getenv("MMEDITOR");
    mmeditor = mmeditor == NULL ? "code -w" : mmeditor;
    snprintf(b, STRINGSIZE, "%s \"%s\"", mmeditor, fname);
    rc = system(b);
    if (rc != 0) {
        error("Editor could not be run");
    }

    // Reload the file.
    void *quoted_fname = GetTempStrMemory();
    snprintf(quoted_fname, STRINGSIZE, "\"%s\"", fname);
    if (!FileLoadProgram(quoted_fname)) error("Could not read from $", fname);

    if (del) {
        set_console_title("MMBasic - Untitled");
        remove(fname);
    }
}
