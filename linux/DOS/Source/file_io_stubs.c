#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../../Version.h"
#include "console.h"

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
int32_t dirflags;

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

//  TODO: should 'src' and 'dst' be otherway round to match strcpy() ?
/** Copies 'src' to 'dst' replacing '\' with '/'. */
void sanitise_path(const char* src, char* dst) {
    const char *psrc = src;
    char *pdst = dst;
    for (;;) {
        *pdst = (*psrc == '\\') ? '/' : *psrc;
        if (*psrc == 0) break;
        psrc++;
        pdst++;
    }
}

void cmd_chdir(void) {
    // Get the directory name and convert to a standard C string.
    char *dir = getCstring(cmdline);
    char sane_dir[STRINGSIZE];
    sanitise_path(dir, sane_dir);
    errno = 0; // TODO: do we really need this? seems unlikely.
    chdir(sane_dir);
    ErrorCheck();
}

void cmd_close(void) {
    int i;
    // getargs() macro must be first executable statement in a block.
    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");
    if ((argc & 0x01) == 0) error("Invalid syntax");

    for (i = 0; i < argc; i += 2) {
        if (*argv[i] == '#') argv[i]++;
        MMfclose(getinteger(argv[i]));
    }
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
    int file_num;
    char *mode, *fname;
    char ss[3];  // this will be used to split up the argument line

    ss[0] = GetTokenValue("FOR");
    ss[1] = tokenAS;
    ss[2] = 0;
    {  // start a new block
        getargs(&cmdline, 5, ss);  // getargs macro must be the first executable stmt in a block
        fname = getCstring(argv[0]);
        if (argc == 3 && *argv[1] == tokenAS && toupper(fname[0]) == 'C' &&
            toupper(fname[1]) == 'O' && toupper(fname[2]) == 'M') {
            assert(false);
            // SerialOpen(fname, argv[2]);
            return;
        }

        if (argc != 5) error("Invalid Syntax");
        if (str_equal(argv[2], "OUTPUT"))
            mode = "wb";  // binary mode so that we do not have lf to cr/lf
                          // translation
        else if (str_equal(argv[2], "APPEND"))
            mode = "ab";  // binary mode is used in MMfopen()
        else if (str_equal(argv[2], "INPUT"))
            mode = "rb";  // note binary mode
        else if (str_equal(argv[2], "RANDOM"))
            mode = "x";  // a special mode for MMfopen()
        else
            error("Invalid file access mode");
        if (*argv[4] == '#') argv[4]++;
        file_num = getinteger(argv[4]);
        printf("%s\n", fname);
        MMfopen(fname, mode, file_num);
    }
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
    set_console_title(tmp);
    return true;
}

void fun_inputstr(void) {

}

void fun_cwd(void) {
    sret = CtoM(MMgetcwd());
    targ = T_STR;
}

static char get_achar(/* Get a character and advances ptr 1 or 2 */
                      const char **ptr /* Pointer to pointer to the
                                          SBCS/DBCS/Unicode string */
) {
    char chr;

    chr = (char)*(*ptr)++;                         /* Get a byte */
    if ((chr >= 'a') && (chr <= 'z')) chr -= 0x20; /* To upper ASCII char */
    return chr;
}

static int
pattern_matching(                 /* 0:not matched, 1:matched */
                 const char *pat, /* Matching pattern */
                 const char *nam, /* String to be tested */
                 int skip,        /* Number of pre-skip chars (number of ?s) */
                 int inf          /* Infinite search (* specified) */
) {
    const char *pp, *np;
    char pc, nc;
    int nm, nx;

    while (skip--) { /* Pre-skip name chars */
        if (!get_achar(&nam))
            return 0; /* Branch mismatched if less name chars */
    }
    if (!*pat && inf) return 1; /* (short circuit) */

    do {
        pp = pat;
        np = nam; /* Top of pattern and name to match */
        for (;;) {
            if (*pp == '?' || *pp == '*') { /* Wildcard? */
                nm = nx = 0;
                do { /* Analyze the wildcard chars */
                    if (*pp++ == '?')
                        nm++;
                    else
                        nx = 1;
                } while (*pp == '?' || *pp == '*');
                if (pattern_matching(pp, np, nm, nx))
                    return 1; /* Test new branch (recurs upto number of wildcard
                                 blocks in the pattern) */
                nc = *np;
                break; /* Branch mismatched */
            }
            pc = get_achar(&pp); /* Get a pattern char */
            nc = get_achar(&np); /* Get a name char */
            if (pc != nc) break; /* Branch mismatched? */
            if (pc == 0)
                return 1; /* Branch matched? (matched at end of both strings) */
        }
        get_achar(&nam); /* nam++ */
    } while (inf &&
             nc); /* Retry until end of name if infinite search is specified */

    return 0;
}

void fun_dir(void) {
    static DIR *dp;
    char *p;
    struct dirent *entry;
    static char pp[32];
    getargs(&ep, 3, ",");
    if (argc != 0) dirflags = DT_REG; // 0 ?
    if (argc > 3) error("Syntax");

    if (argc == 3) {
        if (checkstring(argv[2], "DIR"))
            dirflags = DT_DIR;
        else if (checkstring(argv[2], "FILE"))
            dirflags = DT_REG;
        else if (checkstring(argv[2], "ALL"))
            dirflags = 0;
        else
            error("Invalid flag specification");
    }

    if (argc != 0) {
        // This must be the first call eg:  DIR$("*.*", FILE)
        p = getCstring(argv[0]);
        strcpy(pp, p);
        dp = opendir(".");
        if (dp == NULL) {
            error("Unable to open directory");
        }
    }

    if (dirflags == DT_DIR) {
        for (;;) {
            entry = readdir(dp); /* Get a directory item */
            if (!entry) break; /* Terminate if any error or end of directory */
            if (pattern_matching(pp, entry->d_name, 0, 0) &&
                (entry->d_type == DT_DIR))
                break; /* Test for the file name */
        }
    } else if (dirflags == DT_REG) {
        for (;;) {
            entry = readdir(dp); /* Get a directory item */
            if (!entry) break; /* Terminate if any error or end of directory */
            if (pattern_matching(pp, entry->d_name, 0, 0) &&
                (entry->d_type == DT_REG))
                break; /* Test for the file name */
        }
    } else {
        for (;;) {
            entry = readdir(dp); /* Get a directory item */
            if (!entry) break; /* Terminate if any error or end of directory */
            if (pattern_matching(pp, entry->d_name, 0, 0) &&
                (entry->d_type == DT_DIR))
                break; /* Test for the file name */
        }
    }

    if (!entry) {
        closedir(dp);
        sret =
            GetTempStrMemory();  // this will last for the life of the command
        sret[0] = 0;
        CtoM(sret);  // convert to a MMBasic style string
    } else {
        sret =
            GetTempStrMemory();  // this will last for the life of the command
        strcpy(sret, entry->d_name);
        CtoM(sret);  // convert to a MMBasic style string
    }
    targ = T_STR;
}

void fun_eof(void) {
    getargs(&ep, 1, ",");
    targ = T_INT;
    if (argc == 0) error("Invalid syntax");
    if (*argv[0] == '#') argv[0]++;
    iret = MMfeof(getinteger(argv[0]));
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

    // Convert '\' => '/'.
    char clean_fname[STRINGSIZE];
    char *psrc = fname;
    char *pdest = clean_fname;
    for (;;) {
        *pdest = (*psrc == '\\') ? '/' : *psrc;
        if (*psrc == 0) break;
        psrc++;
        pdest++;
    }

    // random writing is not allowed when a file is opened for append so open it
    // first for read+update and if that does not work open it for
    // writing+update.  This has the same effect as opening for append+update
    // but will allow writing
    if (*mode == 'x') {
        MMFilePtr[file_num] = fopen(clean_fname, "rb+");
        if (MMFilePtr[file_num] == 0) {
            MMFilePtr[file_num] = fopen(clean_fname, "wb+");
            if (ErrorCheck()) return;
        }
        fseek(MMFilePtr[file_num], 0, SEEK_END);
        if (ErrorCheck()) return;
    } else {
        MMFilePtr[file_num] = fopen(clean_fname, mode);
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
