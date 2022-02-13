#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include "mmb4l.h"
#include "console.h"
#include "error.h"
#include "file.h"
#include "program.h"
#include "utility.h"

#define MAXDEFINES  256

char CurrentFile[STRINGSIZE];

static int nDefines = 0;

typedef struct sa_dlist {
    char from[STRINGSIZE];
    char to[STRINGSIZE];
} a_dlist;

static a_dlist *dlist;

static void str_replace(char *target, const char *needle, const char *replacement) {
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

static void STR_REPLACE(char *target, const char *needle, const char *replacement) {
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

static int massage(char *buff) {
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

static int cmpstr(char *s1, char *s2) {
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
            if (pmem > (char *) ProgMemory + PROG_FLASH_SIZE - 3)
                error("Not enough memory");
            *pmem = *pbuf;
        }
        *pmem++ = 0;  // write the terminating zero char

        pstart = pend + 1;
    }

    //printf("DONE\n");

    *pmem++ = 0;
    *pmem++ = 0;  // two zeros terminate the program but add an extra just in case

    // We want CFunctionFlash to start on a 64-bit boundary.
    while ((uintptr_t) pmem % 8 != 0) *pmem++ = 0;
    CFunctionFlash = pmem;

    error_check();
}

/**
 * Gets the absolute canonical path to a .INC file.
 *
 * @param  parent_file  path to .BAS file that is including 'filename'.
 * @param  filename     unprocessed filename.
 * @param  file_path    absolute canonical path to file is returned in this buffer.
 * @return              the value of 'file_path' on success,
 *                      otherwise sets 'errno' and returns NULL.
 */
static char *program_get_inc_file(char *parent_file, char *filename, char *file_path) {

    char tmp_path[STRINGSIZE];
    if (!munge_path(filename, tmp_path, STRINGSIZE)) return NULL;

    if (!is_absolute_path(tmp_path)) {
        char parent_dir[STRINGSIZE];
        if (!get_parent_path(parent_file, parent_dir, STRINGSIZE)) return NULL;

        char tmp_string[STRINGSIZE];
        if (!append_path(parent_dir, tmp_path, tmp_string, STRINGSIZE)) return NULL;

        strcpy(tmp_path, tmp_string);
    }

    // TODO: If file does not exist try appending .inc or .INC to its name.

    return canonicalize_path(tmp_path, file_path, STRINGSIZE);
}

static void importfile(char *parent_file, char *tp, char **p, char *edit_buffer, int convertdebug) {

    char line_buffer[STRINGSIZE];
    char num[10];
    int importlines = 0;
    int ignore = 0;
    char *filename, *sbuff, *op, *ip;
    int c, slen, data;
    int fnbr = file_find_free();
    char *q;
    if ((q = strchr(tp, 34)) == 0) error("Syntax");
    q++;
    if ((q = strchr(q, 34)) == 0) error("Syntax");
    filename = getCstring(tp);

    char file_path[STRINGSIZE];
    errno = 0;
    if (!program_get_inc_file(parent_file, filename, file_path)) {
        switch (errno) {
            case ENOENT:
                error_code(errno, "Include file '$' not found", filename);
                break;
            case ENAMETOOLONG:
                error_code(errno, "Path too long");
                break;
            default:
                error_system(errno);
                break;
        }
    }

    file_open(file_path, "rb", fnbr);
    //    while(!FileEOF(fnbr)) {
    while (!file_eof(fnbr)) {
        int toggle = 0, len = 0;  // while waiting for the end of file
        sbuff = line_buffer;
        if ((*p - edit_buffer) >= EDIT_BUFFER_SIZE - 256 * 6)
            error("Not enough memory");
        //        mymemset(buff,0,256);
        memset(line_buffer, 0, STRINGSIZE);
        MMgetline(fnbr, line_buffer);  // get the input line
        data = 0;
        importlines++;
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
            memcpy(&sbuff[len], filename, strlen(filename));
            len += strlen(filename);
            sbuff[len++] = ',';
            IntToStr(num, importlines, 10);
            strcpy(&sbuff[len], num);
            len += strlen(num);
            if (len > 254) {
                error("Line too long");
            }
            sbuff[len] = 0;
            len = massage(sbuff);  // can't risk crushing lines with a quote in them
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

    file_close(fnbr);
}

/**
 * Gets the absolute canonical path to a .BAS program file.
 *
 * @param  filename   unprocessed filename.
 * @param  file_path  absolute canonical path to file is returned in this buffer.
 * @return            the value of 'file_path' on success,
 *                    otherwise sets 'errno' and returns NULL.
 */
static char *program_get_bas_file(char *filename, char *file_path) {

    char tmp_path[STRINGSIZE];
    if (!munge_path(filename, tmp_path, STRINGSIZE)) return NULL;

    if (CurrentLinePtr && !is_absolute_path(tmp_path)) {
        // If we are in a running program then resolve path relative to the
        // current program directory.
        char current_dir[STRINGSIZE];
        if (!get_parent_path(CurrentFile, current_dir, STRINGSIZE)) return NULL;

        char tmp_string[STRINGSIZE];
        if (!append_path(current_dir, tmp_path, tmp_string, STRINGSIZE)) return NULL;

        strcpy(tmp_path, tmp_string);
    }

    // TODO: If file does not exist try appending .bas or .BAS to its name.

    return canonicalize_path(tmp_path, file_path, STRINGSIZE);
}

// now we must scan the program looking for CFUNCTION/CSUB/DEFINEFONT
// statements, extract their data and program it into the flash used by
// CFUNCTIONs programs are terminated with two zero bytes and one or more
// bytes of 0xff.  The CFunction area starts immediately after that.
//
// The format of a CFunction/CSub/Font in flash is:
//
//   uint64_t     - Address of the CFunction/CSub in program memory (points to
//                  the token representing the "CFunction" keyword) or NULL if
//                  it is a font.
//   uint32_t     - The length of the CFunction/CSub/Font in bytes including
//                  the Offset (see below) but not including any zero padding at
//                  the end.
//   uint32_t     - The Offset (in words) to the main() function (ie, the
//                  entry point to the CFunction/CSub). Omitted in a font.
//   word1..wordN - The CFunction/CSub/Font code (words are 32-bit).
//                - Padding with zeroes to the next 64-bit boundary.
//
// The next CFunction/CSub/Font starts immediately following the last word
// of the previous CFunction/CSub/Font
static void program_process_csubs() {
    char end_token = '\0';
    uint32_t *flash_ptr = (uint32_t *) CFunctionFlash;
    uint32_t *save_addr = NULL;
    char *p = (char *) ProgMemory;  // start scanning program memory

    while (*p != 0xff) {
        if (*p == 0) p++;    // if it is at the end of an element skip the zero marker
        if (*p == 0) break;  // end of the program
        if (*p == T_NEWLINE) {
            CurrentLinePtr = p;
            p++;  // skip the newline token
        }
        if (*p == T_LINENBR) p += 3;  // step over the line number

        skipspace(p);
        if (*p == T_LABEL) {
            p += p[1] + 2;  // skip over the label
            skipspace(p);   // and any following spaces
        }

        if (*p == cmdCSUB) {

            end_token = GetCommandValue("End CSub");
            *((uint64_t *) flash_ptr) = (uintptr_t) p;
            flash_ptr += 2;
            save_addr = flash_ptr; // Save where we are so that we can write the CSub size in here
            flash_ptr ++;
            p++;
            skipspace(p);
            if (!isnamestart(*p)) error("Function name");
            do {
                p++;
            } while (isnamechar(*p));
            skipspace(p);
            if (!(isxdigit(p[0]) && isxdigit(p[1]) && isxdigit(p[2]))) {
                skipelement(p);
                p++;
                if (*p == T_NEWLINE) {
                    CurrentLinePtr = p;
                    p++;  // skip the newline token
                }
                if (*p == T_LINENBR) p += 3;  // skip over a line number
            }
            do {
                while (*p && *p != '\'') {
                    skipspace(p);
                    int n = 0;
                    for (int i = 0; i < 8; i++) {
                        if (!isxdigit(*p)) error("Invalid hex word");
                        n = n << 4;
                        if (*p <= '9')
                            n |= (*p - '0');
                        else
                            n |= (toupper(*p) - 'A' + 10);
                        p++;
                    }
                    if ((char *) flash_ptr >= (char *) ProgMemory + PROG_FLASH_SIZE - 9) error("Not enough memory");
                    *flash_ptr = n;
                    flash_ptr ++;
                    skipspace(p);
                }
                // we are at the end of a embedded code line
                while (*p)
                    p++;  // make sure that we move to the end of the line
                p++;      // step to the start of the next line
                if (*p == 0) error("Missing END declaration");
                if (*p == T_NEWLINE) {
                    CurrentLinePtr = p;
                    p++;  // skip the newline token
                }
                if (*p == T_LINENBR) p += 3;  // skip over the line number
                skipspace(p);
            } while (*p != end_token);

            // Write back the size of the CSUB (in bytes) to the 32-bit slot reserved previously.
            *save_addr = 4 * (flash_ptr - save_addr - 1);

            // Pad to the next 64-bit boundary with zeroes.
            while ((uintptr_t) flash_ptr % 2 != 0) *flash_ptr++ = 0;
        }
        while (*p) p++;  // look for the zero marking the start of the next element
    }

    // Mark the end of the CSUB data.
    *((uint64_t *) flash_ptr) = 0xFFFFFFFFFFFFFFFF;
}

static void get_csub_name(char *p, char *buf) {
    char *p2 = p;
    skipspace(p2);
    while (*p2 == '$' || *p2 == '%' || *p2 == '!' || isnamechar(*p2)) p2++;
    memcpy(buf, p, p2 - p);
    buf[p2 - p] = '\0';
}

static void print_line(char *buf, int* line_count, int all) {
    MMPrintString(buf);
    ListNewLine(line_count, all);
}

void program_list_csubs(int all) {
    uint32_t *p = (uint32_t *) CFunctionFlash;
    char name[64] = { 0 };
    char buf[STRINGSIZE] = { 0 };
    int line_count = 1;

    while (*((uint64_t *) p) != 0xFFFFFFFFFFFFFFFF) {
        if ((char *) p > (char *) CFunctionFlash) ListNewLine(&line_count, all);
        uintptr_t addr = *((uint64_t *) p);
        p += 2;
        get_csub_name((char *) addr + 1, name);
        sprintf(buf, "CSUB %s()", name);
        print_line(buf, &line_count, all);

        sprintf(buf,
#if defined(ENV64BIT)
                "0x%016lX  name   = %s",
#else
                "0x%016llX  name   = %s",
#endif
                (uint64_t) addr, name);
        print_line(buf, &line_count, all);
        int size = *p++;
        sprintf(buf, "0x%08X          size   = %d bytes = %d x 32-bit words", size, size, size / 4);
        print_line(buf, &line_count, all);
        int offset = *p++;
        sprintf(buf, "0x%08X          offset = %d x 32-bit words", offset, offset);
        print_line(buf, &line_count, all);
        size /= 4;  // Convert size to 32-bit words.
        size --;    // Skip the 'offset' word.
        for (int i = 0; i < size; ++i) {
            if (i % 4 == 0) {
                memset(buf, 0, STRINGSIZE);
            } else {
                strcat(buf, ", ");
            }
            sprintf(name, "0x%08X", *p++);
            strcat(buf, name);
            if ((i + 1) % 4 == 0) print_line(buf, &line_count, all);
        }
        //if (size % 4 > 0) ListNewLine(&line_count, all);
    }
    if (strlen(buf) > 0) print_line(buf, &line_count, all);

    print_line("", &line_count, all);
    uint64_t end = *((uint64_t *) p);
    sprintf(buf,
#if defined(ENV64BIT)
            "0x%016lX [%s]",
#else
            "0x%016llX [%s]",
#endif
            end, end == 0xFFFFFFFFFFFFFFFF ? "OK" : "ERROR");
    print_line(buf, &line_count, all);
    print_line("", &line_count, all);
}

static int program_load_file_internal(char *filename) {

    char file_path[STRINGSIZE];
    errno = 0;
    if (!program_get_bas_file(filename, file_path)) {
        switch (errno) {
            case ENOENT:
                error_code(errno, "Program file not found");
                break;
            case ENAMETOOLONG:
                error_code(errno, "Path too long");
                break;
            default:
                error_system(errno);
                break;
        }
    }

    char *p, *op, *ip, *edit_buffer, *sbuff;
    char line_buffer[STRINGSIZE];
    char num[10];
    int c;
    int convertdebug = 1;
    int ignore = 0;
    nDefines = 0;
    int i, importlines = 0, data;

    ClearProgram();
    int fnbr = file_find_free();
    file_open(file_path, "rb", fnbr);

    // TODO: are these being properly released after a longjmp() ?
    p = edit_buffer = GetTempMemory(EDIT_BUFFER_SIZE);
    dlist = GetTempMemory(sizeof(a_dlist) * MAXDEFINES);

    while (!file_eof(fnbr)) {
        int toggle = 0, len = 0, slen;  // while waiting for the end of file
        sbuff = line_buffer;
        if ((p - edit_buffer) >= EDIT_BUFFER_SIZE - 256 * 6)
            ERROR_OUT_OF_MEMORY;
        //        mymemset(buff,0,256);
        memset(line_buffer, 0, STRINGSIZE);
        MMgetline(fnbr, line_buffer);  // get the input line
        data = 0;
        importlines++;
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
            len = massage(sbuff);  // can't risk crushing lines with a quote in them
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

    file_close(fnbr);

    // Ensure every program has an END (and a terminating '\0').
    if (p - edit_buffer > EDIT_BUFFER_SIZE - 5) ERROR_OUT_OF_MEMORY;
    memcpy(p, "END\n", 5);
    p += 5;

    program_tokenise(file_path, edit_buffer);

    ClearSpecificTempMemory(edit_buffer);
    ClearSpecificTempMemory(dlist);

    program_process_csubs();
    // program_list_csubs(1);

    return 0; // Success
}

int program_load_file(char *filename) {
    // Store the current token buffer incase we are at the command prompt.
    char tmp[STRINGSIZE];
    memcpy(tmp, tknbuf, STRINGSIZE);

    int result = program_load_file_internal(filename);

    // Restore the token buffer.
    memcpy(tknbuf, tmp, STRINGSIZE);

    // Set the console window title.
    char title[STRINGSIZE + 10];
    sprintf(title, "MMBasic - %s", CurrentFile);
    console_set_title(title);

    if (error_check()) result = -1; // Error

    return result;
}
