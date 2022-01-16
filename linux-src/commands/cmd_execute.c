#include <ctype.h>
#include <string.h>

#include "../common/version.h"

// char execute_buffer[STRINGSIZE] = { 0 };

// void execute_run() {
//     char *p = inpbuf;
//     char *q, *s;
//     char fn[FF_MAX_LFN];
//     FRESULT fr;
//     FILINFO fno;
//     p[0] = GetCommandValue("RUN");
//     memmove(&p[1], &p[4], strlen(p) - 4);
//     if ((q = strchr(p, ':'))) {
//         q--;
//         *q = '0';
//     }
//     p[strlen(p) - 3] = 0;
//     if ((q = strchr(p, '\"')) != 0) {
//         if ((s = strchr(&q[1], '\"')) != 0) *s = 0;
//         strcpy(fn, &q[1]);
//         strcpy(fn, q);
//         if (strchr(fn, '.') == NULL) {
//             strcat(fn, ".BAS");
//         }
//         if (!InitSDCard()) {
//         };  // make sure the SDcard is there
//         fr = f_stat(&fn[1],
//                     &fno);  // check for the file in the current directory
//         *s = '\"';
//         if (fr != FR_OK ||
//             (fno.fattrib & AM_DIR)) {  // file was not in the current
//                                        // directory so insert the path
//             int slen = strlen(p) - (uint32_t)p + (uint32_t)&q[1];
//             memmove(&q[1 + strlen((char *)Option.path)], &q[1], slen + 1);
//             memcpy(&q[1], (char *)Option.path, strlen((char *)Option.path));
//         }
//     }
//     CloseAudio(1);
//     strcpy(tknbuf, inpbuf);
//     longjmp(run, 1);
// }

void execute_other() {
    int i = 0;
    int toggle = 0;

    while (inpbuf[i]) {
        if (inpbuf[i] == 34) {
            if (toggle == 0)
                toggle = 1;
            else
                toggle = 0;
        }
        if (!toggle) {
            if (inpbuf[i] == ':') error("Only single statements allowed");
            // inpbuf[i] = toupper(inpbuf[i]);
        }
        i++;
    }
    tokenise(true);  // and tokenise it (the result is in tknbuf)
    memset(inpbuf, 0, STRINGSIZE);
    tknbuf[strlen(tknbuf)] = 0;
    tknbuf[strlen(tknbuf) + 1] = 0;

    char *ttp = nextstmt;  // save the globals used by commands
    // ScrewUpTimer = 1000;
    ExecuteProgram(tknbuf);  // execute the function's code
    // ScrewUpTimer = 0;
    // TempMemoryIsChanged = true;                                     //
    // signal that temporary memory should be checked
    nextstmt = ttp;
}

void execute(char *mycmd) {
    skipspace(mycmd);
    strcpy(inpbuf, getCstring(mycmd));
    execute_other();
    // if (toupper(inpbuf[0]) == 'R'
    //         && toupper(inpbuf[1]) == 'U'
    //         && toupper(inpbuf[2]) == 'N') {
    //     execute_run();
    // } else {
    //     execute_other();
    // }
}

void cmd_execute(void) {
    execute(cmdline);
}