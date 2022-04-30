#include <stdlib.h>

#include "../common/mmb4l.h"
#include "../common/error.h"

// count the number of lines up to and including the line pointed to by the argument
// used for error reporting in programs that do not use line numbers
int TraceLines(char *target) {
    char *p;
    int i, cnt;

    p = (char *) ProgMemory;
    cnt = 0;
    if (target == NULL) return cnt;
    while (1) {
        if (*p == 0xff || (p[0] == 0 && p[1] == 0))  // end of the program
            return cnt;

        if (*p == T_NEWLINE) {
            p++;  // and step over the line number
            if (p >= target) {
                char buf[STRINGSIZE], buff[10];
                char *ename, *cpos = NULL;
                memcpy(buf, p, STRINGSIZE);
                i = 0;
                while (!(buf[i] == 0 && buf[i + 1] == 0)) i++;
                while (i > 0) {
                    if (buf[i] == '|') cpos = &buf[i];
                    i--;
                }
                if (cpos != NULL) {
                    MMPrintString("[");
                    if ((ename = strchr(cpos, ',')) != NULL) {
                        *ename = 0;
                        cpos++;
                        ename++;
                        MMPrintString(cpos);
                        MMPrintString(":");
                        MMPrintString(ename);
                    } else {
                        cpos++;
                        IntToStr(buff, atoi(cpos), 10);
                        MMPrintString(buff);
                    }
                    MMPrintString("] ");
                }
            }
            continue;
        }

        if (*p == T_LINENBR) {
            p += 3;  // and step over the line number
            continue;
        }

        if (*p == T_LABEL) {
            p += p[0] + 2;  // still looking! skip over the label
            continue;
        }

        if (p++ > target) return cnt;
    }
}

void cmd_trace(void) {
    if (checkstring(cmdline, "ON"))
        TraceOn = true;
    else if (checkstring(cmdline, "OFF"))
        TraceOn = false;
    else if (checkstring(cmdline, "LIST")) {
        int i;
        cmdline += 4;
        skipspace(cmdline);
        if (*cmdline == 0 || *cmdline == '\'')  //'
            i = TRACE_BUFF_SIZE - 1;
        else
            i = getint(cmdline, 0, TRACE_BUFF_SIZE - 1);
        i = TraceBuffIndex - i;
        if (i < 0) i += TRACE_BUFF_SIZE;
        while (i != TraceBuffIndex) {
            TraceLines(TraceBuff[i]);
            if (++i >= TRACE_BUFF_SIZE) i = 0;
        }
    } else {
        ERROR_UNKNOWN_SUBCOMMAND;
    }
}