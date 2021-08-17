/***********************************************************************************************************************
MMBasic

DOS_IO.c

Handles all the file input/output in DOS MMBasic.

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

#include <stdlib.h>                               // standard library functions
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <windows.h>
#include <tchar.h>
#include <conio.h>
#include <strsafe.h>

#include "..\..\Version.h"

FILE *MMFilePtr[MAXOPENFILES];
HANDLE *MMComPtr[MAXOPENFILES];
int OptionFileErrorAbort = true;
char CurrentFile[STRINGSIZE];

unsigned short int *ConvertToUTF16(char *p);
char *ChangeToDir(char *p);
char *MMgetcwd(void);

void SerialOpen(char *arg1, char *arg2);
void SerialClose(HANDLE fd);
int Serialgetc(HANDLE fd);
int SerialEOF(HANDLE fd);
int Serialputc(int c, HANDLE fd);
int SerialRxQueueSize(HANDLE fd);

//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// ERROR HANDLING ////////////////////////////////////////////



/******************************************************************************************
Text for the file related error messages reported by MMBasic
******************************************************************************************/

int ErrorThrow(int e) {
    MMerrno = e;
    if(e > 0 && e < sys_nerr) {
        error((char *)sys_errlist[e]);
    }
    errno = 0;
    return e;
}


int ErrorCheck(void) {
    int e;
    e = errno;
    errno = 0;
    if(e < 1 || e > sys_nerr) return e;
    return ErrorThrow(e);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************************
File related commands in MMBasic
================================
These are the functions responsible for executing the file related commands in MMBasic
They are supported by utility functions that are grouped at the end of this file

Each function is responsible for decoding a command
all function names are in the form cmd_xxxx() (for a basic command) or fun_xxxx() (for a
basic function) so, if you want to search for the function responsible for the LOCATE command
look for cmd_name

There are 4 items of information that are setup before the command is run.
All these are globals.

int cmdtoken  This is the token number of the command (some commands can handle multiple
          statement types and this helps them differentiate)

char *cmdline   This is the command line terminated with a zero char and trimmed of leading
          spaces.  It may exist anywhere in memory (or even ROM).

char *nextstmt    This is a pointer to the next statement to be executed.  The only thing a
          command can do with it is save it or change it to some other location.

char *CurrentLinePtr  This is read only and is set to NULL if the command is in immediate mode.

The only actions a command can do to change the program flow is to change nextstmt or
execute longjmp(mark, 1) if it wants to abort the program.

********************************************************************************************/



// load a file into program memory
int FileLoadProgram(char *arg) {
    int fnbr;
    char *p, *pp, *fname, tmp[STRINGSIZE];

    memcpy(tmp, tknbuf, STRINGSIZE);                                // save the token buffer incase we are at the command prompt
    fname = getCstring(arg);                                        // get the filename
    if(strchr(fname, '.') == NULL) strcat(fname, ".bas");

    // first try opening the file to make sure that it is there
    fnbr = FindFreeFileNbr();
    MMfopen(fname, "rb", fnbr);
    if(errno) return false;
    MMfclose(fnbr);

    ClearProgram();                                                 // clear any leftovers from the previous program

    // reopen after clearing the program settings
    fnbr = FindFreeFileNbr();
    MMfopen(fname, "rb", fnbr);
    if(errno) return false;

    strcpy(CurrentFile, fname);
    p = ProgMemory;
    while(!MMfeof(fnbr)) {
        MMgetline(fnbr, inpbuf);
        if(errno) {
            memcpy(tknbuf, tmp, STRINGSIZE);
            return false;
        }
        tokenise(false);
        for(pp = tknbuf; !(pp[0] == 0 && pp[1] == 0); p++, pp++) {
            if(p > ProgMemory + PROG_FLASH_SIZE - 3) error("Not enough memory");
            *p = *pp;
        }
        *p++ = 0;                                                   // write the terminating zero char
    }
    *p++ = 0; *p = 0;                                               // two zeros terminate the program but as an extra just in case
    memcpy(tknbuf, tmp, STRINGSIZE);
    MMfclose(fnbr);
    if(errno) return false;
    strcpy(tmp, "MMBasic - "); strcat(tmp, CurrentFile);
    SetConsoleTitle(tmp);
    return true;
}


// the LOAD command
void cmd_load(void) {
    int autorun = false;
    getargs(&cmdline, 3, ",");
    if(!(argc & 1) || argc == 0) error("Syntax");
    if(argc == 3) {
        if(toupper(*argv[2]) == 'R')
            autorun = true;
        else
            error("Syntax");
    } else if(CurrentLinePtr != NULL)
          error("Invalid in a program");

    if(!FileLoadProgram(argv[0])) return;
    if(ErrorCheck()) return;

    if(autorun && (*ProgMemory == T_NEWLINE || *ProgMemory == T_LINENBR)) {
        ClearRuntime();
        PrepareProgram(true);
        nextstmt = ProgMemory;
    }
}





void cmd_save(void) {
    char b[STRINGSIZE];
    char *fname;
    char *p;
    FILE *f;

    fname = getCstring(cmdline);
    if(*fname == 0) error("Invalid file name");
    if(strchr(fname, '.') == NULL) strcat(fname, ".bas");
    errno = 0;
    f = fopen(fname, "wb");
    if(errno) error("Cannot write to $", fname);

    p = ProgMemory;
    while(!(*p == 0 || *p == 0xff)) {                               // normally a LIST ends at the break so this is a safety precaution
        if(*p == T_LINENBR || *p == T_NEWLINE) {
            p = llist(b, p);                                        // otherwise expand the line
            strcat(b, "\r\n");
            fwrite(b, strlen(b), 1, f);
            if(errno) error("Cannot write to $", fname);
            if(p[0] == 0 && p[1] == 0) break;                       // end of the program ?
        }
    }
    fclose(f);
    strcpy(CurrentFile, fname);
    strcpy(b, "MMBasic - "); strcat(b, CurrentFile);
    SetConsoleTitle(b);
}



void cmd_files(void) {
    char b[STRINGSIZE] = "DIR";

    skipspace(cmdline);
    if(!(*cmdline == 0 || *cmdline == '\'')) {
        strcat(b, " \"");
        strcat(b, getCstring(cmdline));
        strcat(b, "\"");
    }

    system(b);
}



void cmd_open(void) {
    int fnbr;
    char *mode, *fname;
    char ss[3];                                                     // this will be used to split up the argument line

    ss[0] = GetTokenValue("FOR");
    ss[1] = tokenAS;
    ss[2] = 0;
    {                                                               // start a new block
        getargs(&cmdline, 5, ss);                                   // getargs macro must be the first executable stmt in a block
        fname = getCstring(argv[0]);
        if(argc == 3 && *argv[1] == tokenAS && toupper(fname[0]) == 'C' && toupper(fname[1]) == 'O' && toupper(fname[2]) == 'M') {
            SerialOpen(fname, argv[2]);
            return;
        }

        if(argc != 5) error("Invalid Syntax");
        if(str_equal(argv[2], "OUTPUT"))
            mode = "wb";                                            // binary mode so that we do not have lf to cr/lf translation
        else if(str_equal(argv[2], "APPEND"))
            mode = "ab";                                            // binary mode is used in MMfopen()
        else if(str_equal(argv[2], "INPUT"))
            mode = "rb";                                            // note binary mode
        else if(str_equal(argv[2], "RANDOM"))
            mode = "x";                                             // a special mode for MMfopen()
        else
            error("Invalid file access mode");
        if(*argv[4] == '#') argv[4]++;
        fnbr = getinteger(argv[4]);
        MMfopen(fname, mode, fnbr);
    }
}



void cmd_close(void) {
    int i;
    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");                // getargs macro must be the first executable stmt in a block
    if((argc & 0x01) == 0) error("Invalid syntax");

    for(i = 0; i < argc; i += 2) {
        if(*argv[i] == '#') argv[i]++;
        MMfclose(getinteger(argv[i]));
    }
}



void cmd_seek(void) {
    int fnbr, idx;
    getargs(&cmdline, 5, ",");
    if(argc != 3) error("Invalid syntax");
    if(*argv[0] == '#') argv[0]++;
    fnbr = getinteger(argv[0]) - 1;
    if(fnbr < 0 || fnbr >= 10) error("Invalid file number");
    if(MMFilePtr[fnbr] == NULL) error("File number is not open");
    idx = getinteger(argv[2]) - 1;
    if(idx < 0) error("Invalid seek position");
    fflush(MMFilePtr[fnbr]);
    fsync(fileno(MMFilePtr[fnbr]));
    fseek(MMFilePtr[fnbr], idx, SEEK_SET);
    ErrorCheck();
}


void fun_loc(void) {
    int fnbr;
    targ = T_INT;
    skipspace(ep);
    if(*ep == '#') ep++;
    fnbr = getinteger(ep) - 1;
    if(fnbr < 0 || fnbr >= 10) error("Invalid file number");
    if(MMComPtr[fnbr] != NULL) {
        iret = SerialRxQueueSize(MMComPtr[fnbr]);                   // it is a serial I/O port
    } else {
        if(MMFilePtr[fnbr] == NULL) error("File number is not open");
        iret = ftell(MMFilePtr[fnbr]) + 1;                          // it is a file
        if(ErrorCheck()) return;
    }
}



void fun_lof(void) {
    int fnbr, pos;
    struct stat buf;
    targ = T_INT;
    skipspace(ep);
    if(*ep == '#') ep++;
    fnbr = getinteger(ep) - 1;
    if(fnbr < 0 || fnbr >= 10) error("Invalid file number");
    if(MMComPtr[fnbr] != NULL) {
        iret = 0;                                                   // it is a serial I/O port and they are unbuffered
    } else {
        if(MMFilePtr[fnbr] == NULL) error("File number is not open");
        pos = ftell(MMFilePtr[fnbr]);
        if(ErrorCheck()) return;
        fseek(MMFilePtr[fnbr], 0L, SEEK_END);
        if(ErrorCheck()) return;
        iret = ftell(MMFilePtr[fnbr]);
        if(ErrorCheck()) return;
        fseek(MMFilePtr[fnbr], pos, SEEK_SET);
        if(ErrorCheck()) return;
    }
}



void fun_inputstr(void) {
    int nbr, fnbr;
    char *p;
    getargs(&ep, 3, ",");
    if(argc != 3) error("Invalid syntax");
    nbr = getinteger(argv[0]);
    if(nbr < 1 || nbr > MAXSTRLEN) error("Number out of bounds");
    if(*argv[2] == '#') argv[2]++;
    fnbr = getinteger(argv[2]);
    sret = GetTempStrMemory();                                      // this will last for the life of the command
    p = sret + 1;                                                   // point to the start of the char array
    *sret = nbr;                                                    // set the length of the returned string
    while(nbr) {
        if(MMfeof(fnbr)) break;
        *p++ = MMfgetc(fnbr);                                       // get the char and save in our returned string
        nbr--;
    }
    *sret -= nbr;                                                   // correct if we get less than nbr chars
    targ = T_STR;
}



void cmd_mkdir(void) {
    char *p;

    p = getCstring(cmdline);                                        // get the directory name and convert to a standard C string
    errno = 0;
    mkdir(p);
    ErrorCheck();
}




void cmd_rmdir(void){
    char *p;

    p = getCstring(cmdline);                                        // get the directory name and convert to a standard C string
    errno = 0;
    rmdir(p);
    ErrorCheck();
}




void cmd_chdir(void){
    char *p;

    p = getCstring(cmdline);                                        // get the directory name and convert to a standard C string
    errno = 0;
    chdir(p);
    ErrorCheck();
}



void fun_eof(void) {
    getargs(&ep, 1, ",");
    targ = T_INT;
    if(argc == 0) error("Invalid syntax");
    if(*argv[0] == '#') argv[0]++;
    iret = MMfeof(getinteger(argv[0]));
}



void fun_cwd(void) {
    sret = CtoM(MMgetcwd());
    targ = T_STR;
}


void cmd_kill(void){
    char *p;
    int err;

    p = getCstring(cmdline);                                        // get the file name and convert to a standard C string

    errno = 0;
    remove(p);
    if((err = errno) != 0) {
        ErrorThrow(err);
        return;
    }

    ErrorCheck();
}




void cmd_name(void) {
    char *oldf, *newf, ss[2];
    ss[0] = tokenAS;                                                // this will be used to split up the argument line
    ss[1] = 0;
    {                                                               // start a new block
        getargs(&cmdline, 3, ss);                                   // getargs macro must be the first executable stmt in a block
        if(argc != 3) error("Invalid syntax");
        oldf = getCstring(argv[0]);                                 // get the old file name and convert to a standard C string
        newf = getCstring(argv[2]);                                 // get the new file name and convert to a standard C string
        errno = 0;
        rename(oldf, newf);
        if(ErrorCheck()) return;
    }
}




void cmd_copy(void) { // thanks to Bryan Rentoul for the contribution
    char *oldf, *newf, ss[2];
    char c;
    int of, nf;

    ss[0] = tokenTO;                                                // this will be used to split up the argument line
    ss[1] = 0;
    {
        getargs(&cmdline, 3, ss);                                   // getargs macro must be the first executable stmt in a block
        if(argc != 3) error("Invalid syntax");
        oldf = getCstring(argv[0]);                                 // get the old file name and convert to a standard C string
        newf = getCstring(argv[2]);                                 // get the new file name and convert to a standard C string

        of = FindFreeFileNbr();
        MMfopen(oldf, "r", of);

        nf = FindFreeFileNbr();
        MMfopen(newf, "w", nf);                                     // We'll just overwrite any existing file
    }
    while(1) {
        if(MMfeof(of)) break;
        c = MMfgetc(of);
        MMfputc(c, nf);
    }
    MMfclose(of);
    MMfclose(nf);
}


/*******************************************************************************************
I/O related functions called from within MMBasic
********************************************************************************************/



// fname must be a standard C style string (not the MMBasic style)
void MMfopen(char *fname, char *mode, int fnbr) {
    int err;
    if(fnbr < 1 || fnbr > 10) error("Invalid file number");
    fnbr--;
    if(MMFilePtr[fnbr] != NULL || MMComPtr[fnbr] != NULL) error("File number is already open");
    errno = 0;

    // random writing is not allowed when a file is opened for append so open it first for read+update
    // and if that does not work open it for writing+update.  This has the same effect as opening for
    // append+update but will allow writing
    if(*mode == 'x') {
        MMFilePtr[fnbr] = fopen(fname, "rb+");
        if(MMFilePtr[fnbr] == 0) {
            MMFilePtr[fnbr] = fopen(fname, "wb+");
            if(ErrorCheck()) return;
        }
        fseek(MMFilePtr[fnbr], 0, SEEK_END);
        if(ErrorCheck()) return;
    }
    else {
        MMFilePtr[fnbr] = fopen(fname, mode);
        if(ErrorCheck()) return;
        }

    if(MMFilePtr[fnbr] == NULL) ErrorThrow(9);
}




void MMfclose(int fnbr) {
    if(fnbr < 1 || fnbr > 10) error("Invalid file number");
    fnbr--;
    if(MMFilePtr[fnbr] == NULL && MMComPtr[fnbr] == NULL) error("File number is not open");
    if(MMFilePtr[fnbr] != NULL) {
        errno = 0;
        fclose(MMFilePtr[fnbr]);
        MMFilePtr[fnbr] = NULL;
        ErrorCheck();
    } else {
        SerialClose(MMComPtr[fnbr]);
        MMComPtr[fnbr] = NULL;
    }
}



void CloseAllFiles(void) {
    int i;
    for(i = 0; i < MAXOPENFILES; i++) {
        if(MMFilePtr[i] != NULL) fclose(MMFilePtr[i]);              // close the file
        if(MMComPtr[i] != NULL) SerialClose(MMComPtr[i]);
        MMComPtr[i] = NULL; MMFilePtr[i] = NULL;                    // make sure that the entry is removed
    }
}



int MMfgetc(int fnbr) {
    unsigned char ch;
    if(fnbr < 0 || fnbr > 10) error("Invalid file number");
    if(fnbr == 0) return MMgetchar();
    fnbr--;
    if(MMComPtr[fnbr] != NULL) return Serialgetc(MMComPtr[fnbr]);
    if(MMFilePtr[fnbr] == NULL) error("File number is not open");
    errno = 0;
    if(fread(&ch, 1, 1, MMFilePtr[fnbr]) == 0) ch = -1;
    ErrorCheck();
    return ch;
}



char MMfputc(char c, int fnbr) {

    if(fnbr < 0 || fnbr > 10) error("Invalid file number");
    if(fnbr == 0) return MMputchar(c);
    fnbr--;
    if(MMComPtr[fnbr] != NULL) return Serialputc(c, MMComPtr[fnbr]);
    if(MMFilePtr[fnbr] == NULL) error("File number is not open");
    errno = 0;
    if(fwrite(&c, 1, 1, MMFilePtr[fnbr]) == 0) if(ErrorCheck() == 0) ErrorThrow(9);
    ErrorCheck();
    return c;
}



int MMfeof(int fnbr) {
  int i, c;
  if(fnbr < 0 || fnbr > 10) error("Invalid file number");
  if(fnbr == 0) return 0;
  fnbr--;
  if(MMComPtr[fnbr] != NULL) return SerialEOF(MMComPtr[fnbr]);
  if(MMFilePtr[fnbr] == NULL) error("File number is not open");
  errno = 0;
  c = fgetc(MMFilePtr[fnbr]);                                       // the Watcom compiler will only set eof after it has tried to read beyond the end of file
  if(ErrorCheck()) return 0;
  i = (feof(MMFilePtr[fnbr]) != 0) ? -1 : 0;
  if(ErrorCheck()) return 0;
  ungetc(c, MMFilePtr[fnbr]);                                       // undo the Watcom bug fix
  ErrorCheck();
  return i;
}



char *MMgetcwd(void) {
  char *b;
  b = GetTempStrMemory();
  errno = 0;
  getcwd(b, STRINGSIZE);
  ErrorCheck();
  return b;
}



// finds the first available free file number.
int FindFreeFileNbr(void) {
  int i;
  for(i = 0; i < MAXOPENFILES; i++)
      if(MMFilePtr[i] == NULL && MMComPtr[i] == NULL) return i + 1;
  error("Too many files open");
  return 0;       // keep the compiler quiet
}


