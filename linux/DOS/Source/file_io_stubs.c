#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../../Version.h"

#define HANDLE unsigned int

// #include "Configuration.h"

// #define true 1

// extern int MMerrno; // MMBasic.c
// extern char *sret; // MMBasic.c

// void error(char *msg, ...); // MMBasic.c
// int MMgetchar(void); // Main.c
// char MMputchar(char c); // Main.c

HANDLE *MMComPtr[MAXOPENFILES];
FILE *MMFilePtr[MAXOPENFILES];

int OptionFileErrorAbort = true;
char CurrentFile[STRINGSIZE];

char *MMgetcwd(void);

int ErrorThrow(int e) {
    // MMerrno = e;
    // if (e > 0 && e < sys_nerr) {
    //     error(strerror(e));
    // }
    // errno = 0;
    // return e;

    MMerrno = e;
    errno = 0;
    if (MMerrno != 0) {
        error(strerror(MMerrno));
    }
    return MMerrno;
}

int ErrorCheck(void) {
    // int e = errno;
    // errno = 0;
    // if (e < 1 || e > sys_nerr) return e;
    // return ErrorThrow(e);

    MMerrno = errno;
    errno = 0;
    if (MMerrno != 0) {
        error(strerror(MMerrno));
    }
    return MMerrno;
}

void CloseAllFiles(void) {

}

void cmd_chdir(void) {

}

void cmd_close(void) {

}

void cmd_copy(void) {

}

void cmd_files(void) {
    char b[STRINGSIZE] = "ls";

    skipspace(cmdline);
    if (!(*cmdline == 0 || *cmdline == '\'')) {
        strcat(b, " \"");
        strcat(b, getCstring(cmdline));
        strcat(b, "\"");
    }

    system(b);
}

void cmd_kill(void) {

}

void cmd_load(void) {
    int autorun = false;
    getargs(&cmdline, 3, ",");
    if (!(argc & 1) || argc == 0) error("Syntax");
    if (argc == 3) {
        if (toupper(*argv[2]) == 'R') {
            autorun = true;
        } else {
            error("Syntax");
        }
    } else if (CurrentLinePtr != NULL) {
        error("Invalid in a program");
    }

    if (!FileLoadProgram(argv[0])) return;

    if (ErrorCheck()) return;

    if (autorun && (*ProgMemory == T_NEWLINE || *ProgMemory == T_LINENBR)) {
        ClearRuntime();
        PrepareProgram(true);
        nextstmt = ProgMemory;
    }
}

void cmd_mkdir(void) {

}

void cmd_name(void) {

}

void cmd_open(void) {

}

void cmd_rmdir(void) {

}

void cmd_seek(void) {

}

void cmd_save(void) {

}

/** Find the first available free file number. */
int FindFreeFileNbr(void) {
    int i;
    for (i = 0; i < MAXOPENFILES; i++) {
        if (MMFilePtr[i] == NULL && MMComPtr[i] == NULL) return i + 1;
    }
    error("Too many files open");
    return 0;  // keep the compiler quiet
}

void SetConsoleTitle(char* title) {
    // TODO
}

int FileLoadProgram(char *arg) {
    int file_num;
    char *p, *pp, *fname, tmp[STRINGSIZE];

    memcpy(tmp, tknbuf, STRINGSIZE);  // save the token buffer incase we are at
                                      // the command prompt
    fname = getCstring(arg);          // get the filename
    if (strchr(fname, '.') == NULL) strcat(fname, ".bas");

    // first try opening the file to make sure that it is there
    file_num = FindFreeFileNbr();
    MMfopen(fname, "rb", file_num);
    if (errno) return false;
    MMfclose(file_num);

    ClearProgram();  // clear any leftovers from the previous program

    // reopen after clearing the program settings
    file_num = FindFreeFileNbr();
    MMfopen(fname, "rb", file_num);
    if (errno) return false;

    strcpy(CurrentFile, fname);
    p = ProgMemory;
    while (!MMfeof(file_num)) {
        MMgetline(file_num, inpbuf);
        if (errno) {
            memcpy(tknbuf, tmp, STRINGSIZE);
            return false;
        }
        tokenise(false);
        for (pp = tknbuf; !(pp[0] == 0 && pp[1] == 0); p++, pp++) {
            if (p > ProgMemory + PROG_FLASH_SIZE - 3)
                error("Not enough memory");
            *p = *pp;
        }
        *p++ = 0;  // write the terminating zero char
    }
    *p++ = 0;
    *p = 0;  // two zeros terminate the program but as an extra just in case
    memcpy(tknbuf, tmp, STRINGSIZE);
    MMfclose(file_num);
    if (errno) return false;
    strcpy(tmp, "MMBasic - ");
    strcat(tmp, CurrentFile);
    SetConsoleTitle(tmp);
    return true;
}

void fun_inputstr(void) {

}

void fun_cwd(void) {
    sret = CtoM(MMgetcwd());
    targ = T_STR;
}

void fun_eof(void) {

}

void fun_loc(void) {

}

void fun_lof(void) {

}

char *MMgetcwd(void) {
  char *b;
  b = GetTempStrMemory();
  errno = 0;
  getcwd(b, STRINGSIZE);
  ErrorCheck();
  return b;
}

int MMfeof(int file_num) {
    int i, c;
    if (file_num < 0 || file_num > 10) error("Invalid file number");
    if (file_num == 0) return 0;
    file_num--;
    // if (MMComPtr[file_num] != NULL) return SerialEOF(MMComPtr[file_num]);
    if (MMFilePtr[file_num] == NULL) error("File number is not open");
    errno = 0;
    c = fgetc(MMFilePtr[file_num]);  // the Watcom compiler will only set eof after
                                     // it has tried to read beyond the end of file
    if (ErrorCheck()) return 0;
    i = (feof(MMFilePtr[file_num]) != 0) ? -1 : 0;
    if (ErrorCheck()) return 0;
    ungetc(c, MMFilePtr[file_num]);  // undo the Watcom bug fix
    ErrorCheck();
    return i;
}

int MMfgetc(int file_num) {
    unsigned char ch;
    if (file_num < 0 || file_num > 10) error("Invalid file number");
    if (file_num == 0) return MMgetchar();
    file_num--;
    // if (MMComPtr[file_num] != NULL) return Serialgetc(MMComPtr[file_num]);
    if (MMFilePtr[file_num] == NULL) error("File number is not open");
    errno = 0;
    if (fread(&ch, 1, 1, MMFilePtr[file_num]) == 0) {
        ch = -1;
    }
    ErrorCheck();
    return ch;
}

char MMfputc(char c, int file_num) {
    if (file_num < 0 || file_num > 10) error("Invalid file number");
    if (file_num == 0) return MMputchar(c);
    file_num--;
    // if (MMComPtr[file_num] != NULL) return Serialputc(c, MMComPtr[file_num]);
    if (MMFilePtr[file_num] == NULL) error("File number is not open");
    errno = 0;
    if (fwrite(&c, 1, 1, MMFilePtr[file_num]) == 0) {
        if (ErrorCheck() == 0) {
            ErrorThrow(9);
        }
    }
    ErrorCheck();
    return c;
}

/**
 * @param  fname  filename in C-string style, not MMBasic style.
 */
void MMfopen(char *fname, char *mode, int file_num) {
    int err;
    if (file_num < 1 || file_num > 10) error("Invalid file number");
    file_num--;
    if (MMFilePtr[file_num] != NULL || MMComPtr[file_num] != NULL) {
        error("File number is already open");
    }
    errno = 0;

    // random writing is not allowed when a file is opened for append so open it
    // first for read+update and if that does not work open it for
    // writing+update.  This has the same effect as opening for append+update
    // but will allow writing
    if (*mode == 'x') {
        MMFilePtr[file_num] = fopen(fname, "rb+");
        if (MMFilePtr[file_num] == 0) {
            MMFilePtr[file_num] = fopen(fname, "wb+");
            if (ErrorCheck()) return;
        }
        fseek(MMFilePtr[file_num], 0, SEEK_END);
        if (ErrorCheck()) return;
    } else {
        MMFilePtr[file_num] = fopen(fname, mode);
        if (ErrorCheck()) return;
    }

    if (MMFilePtr[file_num] == NULL) ErrorThrow(9);
}

void MMfclose(int file_num) {
    if (file_num < 1 || file_num > 10) error("Invalid file number");
    file_num--;
    if (MMFilePtr[file_num] == NULL && MMComPtr[file_num] == NULL) {
        error("File number is not open");
    }
    if (MMFilePtr[file_num] != NULL) {
        errno = 0;
        fclose(MMFilePtr[file_num]);
        MMFilePtr[file_num] = NULL;
        ErrorCheck();
    } else {
        assert(false);
        // SerialClose(MMComPtr[file_num]);
        MMComPtr[file_num] = NULL;
    }
}
