#include <assert.h>
#include <stdint.h>

#include "../common/console.h"
#include "../common/utility.h"
#include "../common/version.h"

int ErrorCheck(void); // file_io.c

//extern char* g_absolute_file;

#define EDIT_BUFFER_SIZE  512 * 1024
#define MAXDEFINES  256

int nDefines = 0;
int LineCount = 0;

typedef struct sa_dlist {
    char from[STRINGSIZE];
    char to[STRINGSIZE];
} a_dlist;
a_dlist *dlist;

void str_replace(char *target, const char *needle, const char *replacement) {
    char buffer[288] = {0};
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);

    while (1) {
        const char *p = strstr(tmp, needle);

        // walked past last occurrence of needle; copy remaining part
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }

        // copy part before needle
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;

        // copy replacement string
        memcpy(insert_point, replacement, repl_len);
        insert_point += repl_len;

        // adjust pointers, move on
        tmp = p + needle_len;
    }

    // write altered string back to target
    strcpy(target, buffer);
}

void STR_REPLACE(char *target, const char *needle, const char *replacement) {
    char *ip = target;
    int toggle = 0;
    while (*ip) {
        if (*ip == 34) {
            if (toggle == 0)
                toggle = 1;
            else
                toggle = 0;
        }
        if (toggle && *ip == ' ') {
            *ip = 0xFF;
        }
        if (toggle && *ip == '.') {
            *ip = 0xFE;
        }
        if (toggle && *ip == '=') {
            *ip = 0xFD;
        }
        ip++;
    }
    str_replace(target, needle, replacement);
    ip = target;
    while (*ip) {
        if (*ip == 0xFF) *ip = ' ';
        if (*ip == 0xFE) *ip = '.';
        if (*ip == 0xFD) *ip = '=';
        ip++;
    }
}

int massage(char *buff) {
    int i = nDefines;
    while (i--) {
        char *p = dlist[i].from;
        while (*p) {
            *p = toupper(*p);
            p++;
        }
        p = dlist[i].to;
        while (*p) {
            *p = toupper(*p);
            p++;
        }
        STR_REPLACE(buff, dlist[i].from, dlist[i].to);
    }
    STR_REPLACE(buff, "=<", "<=");
    STR_REPLACE(buff, "=>", ">=");
    STR_REPLACE(buff, " ,", ",");
    STR_REPLACE(buff, ", ", ",");
    STR_REPLACE(buff, " *", "*");
    STR_REPLACE(buff, "* ", "*");
    STR_REPLACE(buff, "- ", "-");
    STR_REPLACE(buff, " /", "/");
    STR_REPLACE(buff, "/ ", "/");
    STR_REPLACE(buff, "= ", "=");
    STR_REPLACE(buff, "+ ", "+");
    STR_REPLACE(buff, " )", ")");
    STR_REPLACE(buff, ") ", ")");
    STR_REPLACE(buff, "( ", "(");
    STR_REPLACE(buff, "> ", ">");
    STR_REPLACE(buff, "< ", "<");
    STR_REPLACE(buff, " '", "'");
    return strlen(buff);
}

int cmpstr(char *s1, char *s2) {
    unsigned char *p1 = (unsigned char *)s1;
    unsigned char *p2 = (unsigned char *)s2;
    unsigned char c1, c2;

    if (p1 == p2) return 0;

    do {
        c1 = tolower(*p1++);
        c2 = tolower(*p2++);
        if (c1 == '\0') return 0;
    } while (c1 == c2);

    return c1 - c2;
}

static void program_transform_line(char *line) {
    STR_REPLACE(line,"MM.INFO$","MM.INFO");
}

// Tokenize the string in the edit buffer
static void program_tokenise(const char *file_path, const char *edit_buf) {
    //const char *p = edit_buf;
    //p++;
    //printf("<begin>\n");
    //printf("%s", edit_buf);
    //printf("<end>\n");

    strcpy(CurrentFile, file_path);

    char *pmem = ProgMemory;

    // First line in the program memory should be a comment containing the 'file_path'.
    memset(inpbuf, 0, STRINGSIZE);
    sprintf(inpbuf, "'%s", file_path);
    tokenise(false);
    memcpy(pmem, tknbuf, strlen(tknbuf) + 1);
    pmem += strlen(tknbuf);
    pmem++;

    // Loop while data
    // Read a line from edit_buf into tkn_buf
    // Tokenize the line.
    // Copy tokenized result into pmem
    char *pend;
    char *pstart = (char *) edit_buf;
    while (*pstart != '\0') {

        // Note that all lines in the edit buffer should just have a '\n' line-end.
        pend = pstart;
        while (*pend != '\n') pend++;
        if (pend - pstart > STRINGSIZE - 1) error("Line too long"); // TODO: what cleans up the edit buffer ?
        memset(inpbuf, 0, STRINGSIZE);
        memcpy(inpbuf, pstart, pend - pstart);
        //printf("%s\n", inpbuf);

        program_transform_line(inpbuf);
        tokenise(false);

        //printf("* %s\n", tknbuf);

        for (char *pbuf = tknbuf; !(pbuf[0] == 0 && pbuf[1] == 0); pmem++, pbuf++) {
            if (pmem > ProgMemory + PROG_FLASH_SIZE - 3)
                error("Not enough memory");
            *pmem = *pbuf;
        }
        *pmem++ = 0;  // write the terminating zero char

        pstart = pend + 1;
    }

    //printf("DONE\n");

    *pmem++ = 0;
    *pmem = 0;  // two zeros terminate the program but as an extra just in case

    // TODO: this is something to do with restoring the token buffer from what we were doing before the LOAD.
    // memcpy(tknbuf, tmp, STRINGSIZE);

    // Does this happen, won't we have done a longjmp ?
    assert(!errno);
    // if (errno) return false;

    char title[STRINGSIZE + 10];
    sprintf(title, "MMBasic - %s", CurrentFile);
    console_set_title(title);
}

static void importfile(
        char *parent_file, char *tp, char **p, char *edit_buffer, int convertdebug) {
    int file_num;
    char line_buffer[STRINGSIZE];
    char num[10];
    int importlines = 0;
    int ignore = 0;
    char *filename, *sbuff, *op, *ip;
    int c, f, slen, data;
    file_num = FindFreeFileNbr();
    char *q;
    if ((q = strchr(tp, 34)) == 0) error("Syntax");
    q++;
    if ((q = strchr(q, 34)) == 0) error("Syntax");
    filename = getCstring(tp);
    if (strchr(&filename[strlen(filename) - 4], '.') == NULL) strcat(filename, ".INC");
    f = strlen(filename);
    q = &filename[strlen(filename) - 4];
    if (strcasecmp(q, ".inc") != 0) error("must be a .inc file");
    // if (!(filename[1] == ':' || filename[0] == 92 || filename[0] == 47)) {
    //     strcpy(qq, parent_file);
    //     strcat(qq, filename);
    // } else
    //     strcpy(qq, filename);
    //      BasicFileOpen(qq, file_num, FA_READ);

    // Determine the absolute path to the included file.
    char file_path[FF_MAX_LFN];
    if (is_absolute_path(filename)) {
        canonicalize_path(filename, file_path, FF_MAX_LFN - 1);
    } else {
        char parent_dir[FF_MAX_LFN];
        //printf("parent_file = %s\n", parent_file);
        get_parent_path(parent_file, parent_dir, FF_MAX_LFN - 1);
        //printf("parent_dir = %s\n", parent_dir);
        char tmp[FF_MAX_LFN];
        append_path(parent_dir, filename, tmp, FF_MAX_LFN - 1);
        //printf("tmp = %s\n", parent_dir);
        canonicalize_path(tmp, file_path, FF_MAX_LFN - 1);
    }

    MMfopen(file_path, "rb", file_num);
    //    while(!FileEOF(file_num)) {
    while (!MMfeof(file_num)) {
        int toggle = 0, len = 0;  // while waiting for the end of file
        sbuff = line_buffer;
        if ((*p - edit_buffer) >= EDIT_BUFFER_SIZE - 256 * 6)
            error("Not enough memory");
        //        mymemset(buff,0,256);
        memset(line_buffer, 0, STRINGSIZE);
        MMgetline(file_num, line_buffer);  // get the input line
        data = 0;
        importlines++;
        LineCount++;
        //        routinechecks(1);
        len = strlen(line_buffer);
        toggle = 0;
        for (c = 0; c < strlen(line_buffer); c++) {
            if (line_buffer[c] == TAB) line_buffer[c] = ' ';
        }
        while (*sbuff == ' ') {
            sbuff++;
            len--;
        }
        if (ignore && sbuff[0] != '#') *sbuff = '\'';
        if (strncasecmp(sbuff, "rem ", 4) == 0 ||
            (len == 3 && strncasecmp(sbuff, "rem", 3) == 0)) {
            sbuff += 2;
            *sbuff = '\'';
            continue;
        }
        if (strncasecmp(sbuff, "data ", 5) == 0) data = 1;
        slen = len;
        op = sbuff;
        ip = sbuff;
        while (*ip) {
            if (*ip == 34) {
                if (toggle == 0)
                    toggle = 1;
                else
                    toggle = 0;
            }
            if (!toggle && (*ip == ' ' || *ip == ':')) {
                *op++ = *ip++;  // copy the first space
                while (*ip == ' ') {
                    ip++;
                    len--;
                }
            } else
                *op++ = *ip++;
        }
        slen = len;
        for (c = 0; c < slen; c++) {
            if (sbuff[c] == 34) {
                if (toggle == 0)
                    toggle = 1;
                else
                    toggle = 0;
            }
            if (!(toggle || data)) sbuff[c] = toupper(sbuff[c]);
            if (!toggle && sbuff[c] == 39 && len == slen) {
                len = c;  // get rid of comments
                break;
            }
        }
        if (sbuff[0] == '#') {
            char *tp = checkstring(&sbuff[1], "DEFINE");
            if (tp) {
                getargs(&tp, 3, ",");
                if (nDefines >= MAXDEFINES) {
                    error("Too many #DEFINE statements");
                }
                strcpy(dlist[nDefines].from, getCstring(argv[0]));
                strcpy(dlist[nDefines].to, getCstring(argv[2]));
                nDefines++;
            } else {
                if (cmpstr("COMMENT END", &sbuff[1]) == 0) ignore = 0;
                if (cmpstr("COMMENT START", &sbuff[1]) == 0) ignore = 1;
                if (cmpstr("MMDEBUG ON", &sbuff[1]) == 0) convertdebug = 0;
                if (cmpstr("MMDEBUG OFF", &sbuff[1]) == 0) convertdebug = 1;
                if (cmpstr("INCLUDE ", &sbuff[1]) == 0) {
                    error("Can't import from an import");
                }
            }
        } else {
            if (toggle) sbuff[len++] = 34;
            sbuff[len++] = 39;
            sbuff[len++] = '|';
            memcpy(&sbuff[len], filename, f);
            len += strlen(filename);
            sbuff[len++] = ',';
            IntToStr(num, importlines, 10);
            strcpy(&sbuff[len], num);
            len += strlen(num);
            if (len > 254) {
                error("Line too long");
            }
            sbuff[len] = 0;
            len = massage(
                sbuff);  // can't risk crushing lines with a quote in them
            if ((sbuff[0] != 39) || (sbuff[0] == 39 && sbuff[1] == 39)) {
                // if(Option.profile){
                //     while(strlen(sbuff)<9){
                //         strcat(sbuff," ");
                //         len++;
                //     }
                // }
                memcpy(*p, sbuff, len);
                *p += len;
                **p = '\n';
                *p += 1;
            }
        }
    }
    //    FileClose(file_num);
    MMfclose(file_num);
}

// filename is a C-string.
static int program_load_file_internal(char *filename) {
    int file_num, size = 0;
    char *p, *op, *ip, *edit_buffer, *sbuff; //, name[FF_MAX_LFN] = {0},
    char line_buffer[STRINGSIZE];
    //char pp[FF_MAX_LFN] = {0};
    char num[10];
    int c;
    int convertdebug = 1;
    int ignore = 0;
    nDefines = 0;
    LineCount = 0;
    int i, importlines = 0, data;
    // if(mode){
    //     strcpy(buff,getCstring(filename));
    // } else strcpy(buff,filename);
    strcpy(line_buffer, filename);
    if (strchr(&line_buffer[strlen(line_buffer) - 4], '.') == NULL) {
        strcat(line_buffer, ".BAS");
    }

    ClearProgram(); // Clear any leftovers from the previous program.

                     //    if(!InitSDCard()) return false;
    file_num = FindFreeFileNbr();
    char file_path[FF_MAX_LFN];
    canonicalize_path(line_buffer, file_path, FF_MAX_LFN - 1);

    //    if(!BasicFileOpen(buff, file_num, FA_READ)) return false;
//    MMfopen(filename, "rb", file_num);
    MMfopen(file_path, "rb", file_num);
//    strcpy(name, g_absolute_file);

    // i = strlen(name) - 1;
    // while (i > 0 && !(name[i] == 92 || name[i] == 47)) i--;
    // memcpy(pp, name, i + 1);

    // TODO: are these being properly released after a longjmp() ?
    p = edit_buffer = GetTempMemory(EDIT_BUFFER_SIZE);
    dlist = GetTempMemory(sizeof(a_dlist) * MAXDEFINES);

    //    while(!FileEOF(file_num)) {                                     // while
    //    waiting for the end of file
    while (!MMfeof(file_num)) {
        int toggle = 0, len = 0, slen;  // while waiting for the end of file
        sbuff = line_buffer;
        if ((p - edit_buffer) >= EDIT_BUFFER_SIZE - 256 * 6)
            error("Not enough memory");
        //        mymemset(buff,0,256);
        memset(line_buffer, 0, STRINGSIZE);
        MMgetline(file_num, line_buffer);  // get the input line
        data = 0;
        importlines++;
        LineCount++;
        //        routinechecks(1);
        len = strlen(line_buffer);
        toggle = 0;
        for (c = 0; c < strlen(line_buffer); c++) {
            if (line_buffer[c] == TAB) line_buffer[c] = ' ';
        }
        while (sbuff[0] == ' ') {  // strip leading spaces
            sbuff++;
            len--;
        }
        if (ignore && sbuff[0] != '#') *sbuff = '\'';
        if (strncasecmp(sbuff, "rem ", 4) == 0 ||
            (len == 3 && strncasecmp(sbuff, "rem", 3) == 0)) {
            sbuff += 2;
            *sbuff = '\'';
            continue;
        }
        if (strncasecmp(sbuff, "mmdebug ", 7) == 0 && convertdebug == 1) {
            sbuff += 6;
            *sbuff = '\'';
            continue;
        }
        if (strncasecmp(sbuff, "data ", 5) == 0) data = 1;
        slen = len;
        op = sbuff;
        ip = sbuff;
        while (*ip) {
            if (*ip == 34) {
                if (toggle == 0)
                    toggle = 1;
                else
                    toggle = 0;
            }
            if (!toggle && (*ip == ' ' || *ip == ':')) {
                *op++ = *ip++;  // copy the first space
                while (*ip == ' ') {
                    ip++;
                    len--;
                }
            } else
                *op++ = *ip++;
        }
        slen = len;
        if (sbuff[0] == '#') {
            char *tp = checkstring(&sbuff[1], "DEFINE");
            if (tp) {
                getargs(&tp, 3, ",");
                if (nDefines >= MAXDEFINES) {
                    error("Too many #DEFINE statements");
                }
                strcpy(dlist[nDefines].from, getCstring(argv[0]));
                strcpy(dlist[nDefines].to, getCstring(argv[2]));
                nDefines++;
            } else {
                if (cmpstr("COMMENT END", &sbuff[1]) == 0) ignore = 0;
                if (cmpstr("COMMENT START", &sbuff[1]) == 0) ignore = 1;
                if (cmpstr("MMDEBUG ON", &sbuff[1]) == 0) convertdebug = 0;
                if (cmpstr("MMDEBUG OFF", &sbuff[1]) == 0) convertdebug = 1;
                if (cmpstr("INCLUDE", &sbuff[1]) == 0) {
                    importfile(file_path, &sbuff[8], &p, edit_buffer, convertdebug);
                }
            }
        } else {
            toggle = 0;
            for (c = 0; c < slen; c++) {
                if (sbuff[c] == 34) {
                    if (toggle == 0)
                        toggle = 1;
                    else
                        toggle = 0;
                }
                if (!(toggle || data)) sbuff[c] = toupper(sbuff[c]);
                if (!toggle && sbuff[c] == 39 && len == slen) {
                    len = c;  // get rid of comments
                    break;
                }
            }
            if (toggle) sbuff[len++] = 34;
            sbuff[len++] = 39;
            sbuff[len++] = '|';
            IntToStr(num, importlines, 10);
            strcpy(&sbuff[len], num);
            len += strlen(num);
            if (len > 254) {
                error("Line too long");
            }
            sbuff[len] = 0;
            len = massage(
                sbuff);  // can't risk crushing lines with a quote in them
            if ((sbuff[0] != 39) || (sbuff[0] == 39 && sbuff[1] == 39)) {
                // if(Option.profile){
                //     while(strlen(sbuff)<9){
                //         strcat(sbuff," ");
                //         len++;
                //     }
                // }
                //                mycpy(p,sbuff,len);
                memcpy(p, sbuff, len);
                p += len;
                *p++ = '\n';
            }
        }
    }
    *p = 0;  // terminate the string in RAM
             //    FileClose(file_num);
    MMfclose(file_num);

    program_tokenise(file_path, edit_buffer);

#if 0
    int load=0;
    if(Option.ProgramStartCode>=0){
        size=SaveProgramToMemory(buf, false, name);
        uint32_t *top=(uint32_t *)((uint32_t)SDMemory+512*1024-4);
        top--;
        i=0;
        ip=(char *)SDMemory;
        op=(char *)ProgMemory;
        while(i<size){
            i++;
            if(*ip != *op){
                load=1;
                break;
            }
            op++;
            ip++;
        }
        while(*top-- == 0xFFFFFFFF){};
        size=(uint32_t)top - (uint32_t)SDMemory+256;
        FreeMemorySafe((void *)&SDMemory);
    }
    if(load || Option.ProgramStartCode<0)SaveProgramToFlash(buf, false, name, size);
#endif

    ClearSpecificTempMemory(edit_buffer);
    ClearSpecificTempMemory(dlist);

    return true;
}

int program_load_file(char *filename) {
    // Store the current token buffer incase we are at the command prompt.
    char tmp[STRINGSIZE];
    memcpy(tmp, tknbuf, STRINGSIZE);

    int result = program_load_file_internal(filename);

    // Restore the token buffer.
    memcpy(tknbuf, tmp, STRINGSIZE);

    assert(errno == 0);
    if (ErrorCheck()) result = false;

    return result;
}

void cmd_program(void) {
    getargs(&cmdline, 3, " ,");
    if (argc == 3 && checkstring(argv[0], "LOAD")) {
        char *filename = getCstring(argv[2]);
        program_load_file(filename);
    } else {
        error("Syntax");
    }
}
