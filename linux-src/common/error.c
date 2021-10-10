#include <assert.h>
#include <stdarg.h>

#include "../common/error.h"
#include "../common/utility.h"
#include "../common/version.h"

extern jmp_buf ErrNext;
extern char MMErrMsg[MAXERRMSG];
// extern char StartEditFile[STRINGSIZE];
// extern int StartEditLine = 0;

// File that the last error was reported from.
char error_file[STRINGSIZE] = { 0 };

// Line that the last error was reported from.
int error_line = -1;

char error_buffer[STRINGSIZE] = { 0 };
size_t error_buffer_pos = 0;

static void MMErrorString(const char *msg) {
    char *src = (char *) msg;
    char *dst = error_buffer + error_buffer_pos;
    while (*src) *dst++ = *src++;
    error_buffer_pos += strlen(msg);
    error_buffer[error_buffer_pos] = '\0';
}

void MMErrorChar(char c) {
    error_buffer[error_buffer_pos++] = c;
}

static void get_line_and_file(int *line, char *file_path) {

    *line = -1;
    memset(file_path, 0, STRINGSIZE);

    if (!CurrentLinePtr) return;

    if (CurrentLinePtr >= (char *) ProgMemory + PROG_FLASH_SIZE) return;

    if (*CurrentLinePtr != T_NEWLINE) {
        // normally CurrentLinePtr points to a T_NEWLINE token but in this
        // case it does not so we have to search for the start of the line
        // and set CurrentLinePtr to that
        char *p = (char *) ProgMemory;
        char *tp = p;
        while (*p != 0xff) {
            while (*p)
                p++;  // look for the zero marking the start of an element
            if (p >= CurrentLinePtr ||
                p[1] == 0) {  // the previous line was the one that we wanted
                CurrentLinePtr = tp;
                break;
            }
            if (p[1] == T_NEWLINE) {
                tp = ++p;  // save because it might be the line we want
            }
            p++;  // step over the zero marking the start of the element
            skipspace(p);
            if (p[0] == T_LABEL) p += p[1] + 2;  // skip over the label
        }
    }

    if (CurrentLinePtr >= (char *) ProgMemory + PROG_FLASH_SIZE) return;

    // We now have CurrentLinePtr pointing to the start of the line.
    llist(tknbuf, CurrentLinePtr);
    // p = tknbuf;
    // skipspace(p);

    char *pipe_pos = strchr(tknbuf, '|');
    if (!pipe_pos) return;

    char *comma_pos = strchr(pipe_pos, ',');
    if (comma_pos) {
        // Line is from a #included file.
        pipe_pos++;
        comma_pos++;
        *line = atoi(comma_pos);

        if (!get_parent_path(CurrentFile, file_path, STRINGSIZE)) return;
        // TODO: prevent buffer overflow.
        int len = strlen(file_path);
        file_path[len++] = '/';
        memcpy(file_path + len, pipe_pos, comma_pos - pipe_pos - 1);
        file_path[len + comma_pos - pipe_pos] = '\0';
    } else {
        // Line is from the main file.
        pipe_pos++;
        *line = atoi(pipe_pos);
        strcpy(file_path, CurrentFile);
    }
}

// throw an error
// displays the error message and aborts the program
// the message can contain variable text which is indicated by a special character in the message string
//  $ = insert a string at this place
//  @ = insert a character
//  % = insert a number
// the optional data to be inserted is the second argument to this function
// this uses longjump to skip back to the command input and cleanup the stack
static void verror(int32_t error, char *msg, va_list argp) {
        va_list ap;
    // ScrewUpTimer=0;
    MMerrno = error;
    memset(error_buffer, 0, STRINGSIZE);
    error_buffer_pos = 0;
    LoadOptions();  // make sure that the option struct is in a clean state

    // if((OptionConsole & 2) && !OptionErrorSkip) {
    //     SetFont(PromptFont);
    //     gui_fcolour = PromptFC;
    //     gui_bcolour = PromptBC;
    //     if(CurrentX != 0) MMErrorString("\r\n");                   // error
    //     message should be on a new line
    // }

    if (MMCharPos > 1 && !OptionErrorSkip) MMErrorString("\r\n");

    get_line_and_file(&error_line, error_file);

    if (error_line > 0) {
        char buf[STRINGSIZE * 2];
        if (strcmp(error_file, CurrentFile) == 0) {
            sprintf(buf, "Error in line %d: ", error_line);
        } else {
            sprintf(buf, "Error in %s line %d: ", error_file, error_line);
        }
        MMErrorString(buf);
    } else {
        MMErrorString("Error: ");
    }

    if (OptionErrorSkip) {
        memset(error_file, 0, STRINGSIZE);
        error_line = -1;
    }

    if (*msg) {
        while (*msg) {
            if (*msg == '$')
                MMErrorString(va_arg(argp, char *));
            else if (*msg == '@')
                MMErrorChar(va_arg(argp, int));
            else if (*msg == '%' || *msg == '|') {
                char buf[20];
                IntToStr(buf, va_arg(argp, int), 10);
                MMErrorString(buf);
            } else if (*msg == '&') {
                char buf[20];
                IntToStr(buf, va_arg(argp, int), 16);
                MMErrorString(buf);
            } else {
                MMErrorChar(*msg);
            }
            msg++;
        }
        if (!OptionErrorSkip) MMErrorString("\r\n");
    }

    strcpy(MMErrMsg, error_buffer);

    if (OptionErrorSkip) {
        error_buffer_pos = 0;
        longjmp(ErrNext, 1);
    } else {
        longjmp(mark, 1);
    }
}

void error(char *msg, ...) {
    va_list argp;
	va_start(argp, msg);
	verror(MMerrno == 0 ? ERRNO_DEFAULT : MMerrno, msg, argp);
    assert(0); // Don't expect to get here because of long_jmp().
	va_end(argp);
}

void error_code(int32_t error, char *msg, ...) {
    va_list argp;
	va_start(argp, msg);
	verror(error, msg, argp);
    assert(0); // Don't expect to get here because of long_jmp().
	va_end(argp);
}

void error_system(int32_t error) {
    error_code(error, strerror(error));
}

int error_check(void) {
    if (errno) {
        int tmp = errno;
        errno = 0;
        error_system(tmp);
        assert(0); // Don't expect to get here because of long_jmp().
        return MMerrno;
    } else {
        return 0;
    }
}
