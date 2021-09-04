#include "../common/version.h"

extern jmp_buf ErrNext;
extern char MMErrMsg[MAXERRMSG];

char error_buffer[STRINGSIZE] = {0};
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

// throw an error
// displays the error message and aborts the program
// the message can contain variable text which is indicated by a special character in the message string
//  $ = insert a string at this place
//  @ = insert a character
//  % = insert a number
// the optional data to be inserted is the second argument to this function
// this uses longjump to skip back to the command input and cleanup the stack
void error(char *msg, ...) {
    char *cpos, *p, *tp;
    va_list ap;
    // ScrewUpTimer=0;
    if (MMerrno == 0) MMerrno = 16;  // indicate an error
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
    if (CurrentLinePtr) {
        tp = p = (char *)ProgMemory;
        if (*CurrentLinePtr != T_NEWLINE &&
            CurrentLinePtr < ProgMemory + PROG_FLASH_SIZE) {
            // normally CurrentLinePtr points to a T_NEWLINE token but in this
            // case it does not so we have to search for the start of the line
            // and set CurrentLinePtr to that
            while (*p != 0xff) {
                while (*p)
                    p++;  // look for the zero marking the start of an element
                if (p >= CurrentLinePtr ||
                    p[1] ==
                        0) {  // the previous line was the one that we wanted
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
        // we now have CurrentLinePtr pointing to the start of the line
        llist(tknbuf, CurrentLinePtr);
        p = tknbuf;
        skipspace(p);
        if (CurrentLinePtr < ProgMemory + PROG_FLASH_SIZE) {
            if (MMCharPos > 1) MMErrorString("\r\n");
            MMErrorString("Error in ");
            char *ename;
            if ((cpos = strchr(tknbuf, '|')) != NULL) {
                if ((ename = strchr(cpos, ',')) != NULL) {
                    *ename = 0;
                    cpos++;
                    ename++;
                    MMErrorString(cpos);
                    MMErrorString(" line ");
                    MMErrorString(ename);
                } else {
                    cpos++;
                    MMErrorString("line ");
                    IntToStr(inpbuf, atoi(cpos), 10);
                    MMErrorString(inpbuf);
                    if (!OptionErrorSkip) {
                        // StartEditLine =  atoi(cpos)-1;
                        // StartEditCharacter = 0;
                    }
                }
                MMErrorString(": ");
            }
        }
    }
    //    if(!OptionErrorSkip)MMErrorString("Error");
    if (*msg) {
        //            if(!OptionErrorSkip)MMErrorString(": ");
        va_start(ap, msg);
        while (*msg) {
            if (*msg == '$')
                MMErrorString(va_arg(ap, char *));
            else if (*msg == '@')
                MMErrorChar(va_arg(ap, int));
            else if (*msg == '%' || *msg == '|') {
                char buf[20];
                IntToStr(buf, va_arg(ap, int), 10);
                MMErrorString(buf);
            } else if (*msg == '&') {
                char buf[20];
                IntToStr(buf, va_arg(ap, int), 16);
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
        // SCB_CleanInvalidateDCache();
        error_buffer_pos = 0;
        longjmp(ErrNext, 1);
    }
    // int maxH=PageTable[WritePage].ymax;
    // deferredcopy=0;
    // if(Option.showstatus && CurrentY > maxH-(gui_font_height<<1)){
    //         MX470PutS("\r\n",WHITE,BLACK);
    //         CurrentY=maxH-(gui_font_height*2);
    //         ShortScroll=Option.showstatus;
    // }
    // SCB_CleanInvalidateDCache();
    longjmp(mark, 1);
}
