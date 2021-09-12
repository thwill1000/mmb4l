#include <assert.h>
#include <unistd.h>

#include "error.h"
#include "file.h"
#include "utility.h"

FILE *MMFilePtr[MAXOPENFILES];
HANDLE *MMComPtr[MAXOPENFILES];
int OptionFileErrorAbort = true;
char CurrentFile[STRINGSIZE];

/*******************************************************************************************
I/O related functions called from within MMBasic
********************************************************************************************/

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

    char canonical[STRINGSIZE];
    munge_path(fname, canonical, STRINGSIZE);

    // random writing is not allowed when a file is opened for append so open it
    // first for read+update and if that does not work open it for
    // writing+update.  This has the same effect as opening for append+update
    // but will allow writing
    if (*mode == 'x') {
        MMFilePtr[file_num] = fopen(canonical, "rb+");
        if (MMFilePtr[file_num] == 0) {
            MMFilePtr[file_num] = fopen(canonical, "wb+");
            if (error_check()) return;
        }
        fseek(MMFilePtr[file_num], 0, SEEK_END);
        if (error_check()) return;
    } else {
        MMFilePtr[file_num] = fopen(canonical, mode);
        if (error_check()) return;
    }

    if (MMFilePtr[file_num] == NULL) {
        errno = EBADF;
        if (error_check()) return;
    }
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
        error_check();
    } else {
        assert(false);
        // SerialClose(MMComPtr[file_num]);
        MMComPtr[file_num] = NULL;
    }
}

void CloseAllFiles(void) {
    for (int i = 0; i < MAXOPENFILES; i++) {
        if (MMFilePtr[i] != NULL) fclose(MMFilePtr[i]);
        // if (MMComPtr[i] != NULL) SerialClose(MMComPtr[i]);
        MMComPtr[i] = NULL;
        MMFilePtr[i] = NULL;
    }
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
    error_check();
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
        if (errno = 0) errno = EBADF;
    }
    error_check();
    return c;
}

int MMfeof(int file_num) {
    int i, c;
    if (file_num < 0 || file_num > 10) error("Invalid file number");
    if (file_num == 0) return 0;
    file_num--;
    // if (MMComPtr[file_num] != NULL) return SerialEOF(MMComPtr[file_num]);
    if (MMFilePtr[file_num] == NULL) error("File number is not open");
    errno = 0;
    c = fgetc(
        MMFilePtr[file_num]);  // the Watcom compiler will only set eof after
                               // it has tried to read beyond the end of file
    if (error_check()) return 0;
    i = (feof(MMFilePtr[file_num]) != 0) ? -1 : 0;
    if (error_check()) return 0;
    ungetc(c, MMFilePtr[file_num]);  // undo the Watcom bug fix
    error_check();
    return i;
}

char *MMgetcwd(void) {
    char *cwd = GetTempStrMemory();
    errno = 0;
    char *result = getcwd(cwd, STRINGSIZE);
    error_check();
    return cwd;
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
