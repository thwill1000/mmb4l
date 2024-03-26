/*-*****************************************************************************

MMBasic for Linux (MMB4L)

program.c

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include "console.h"
#include "cstring.h"
#include "file.h"
#include "mmb4l.h"
#include "parse.h"
#include "path.h"
#include "program.h"
#include "utility.h"
#include "../core/commandtbl.h"

#include <assert.h>
#include <string.h>
#include <unistd.h>

#define ERROR_FUNCTION_NAME                error_throw_ex(kError, "Function name")
#define ERROR_INCLUDE_FILE_NOT_FOUND(s)    error_throw_ex(kFileNotFound, "Include file '$' not found", s)
#define ERROR_INVALID_HEX                  ERROR_INVALID("hex word")
#define ERROR_MISSING_END                  error_throw_ex(kError, "Missing END declaration")
#define ERROR_PROGRAM_FILE_NOT_FOUND       error_throw_ex(kFileNotFound, "Program file not found")
#define ERROR_TOO_MANY_DEFINES             error_throw_ex(kError, "Too many #DEFINE statements")

#define MAXDEFINES  256

#define SINGLE_QUOTE  '\''

// Repetition of last element is deliberate, see implementations of
// program_get_bas_file() and program_get_inc_file().
static const char *BAS_FILE_EXTENSIONS[] = { ".bas", ".BAS", ".Bas", ".bas" };
static const char *INC_FILE_EXTENSIONS[] = { ".inc", ".INC", ".Inc", ".inc" };

char CurrentFile[STRINGSIZE];

typedef struct {
    char from[STRINGSIZE];
    char to[STRINGSIZE];
} Replace;

typedef struct {
    size_t size;
    Replace items[MAXDEFINES];
} ReplaceMap;

static ReplaceMap *program_replace_map = NULL;

typedef struct {
    char filename[STRINGSIZE];
    int fnbr;
    int line_num;
} ProgramFile;

typedef struct {
  size_t size;
  ProgramFile files[MAXOPENFILES];
  ProgramFile *head;
} ProgramFileStack;

static ProgramFileStack *program_file_stack = NULL;

static void STR_REPLACE(char *target, const char *needle, const char *replacement) {
    char *ip = target;
    bool in_quotes = false;
    while (*ip) {
        if (*ip == '"') {
            in_quotes = !in_quotes;
        }
        if (in_quotes && *ip == ' ') {
            *ip = 0xFF;
        }
        if (in_quotes && *ip == '.') {
            *ip = 0xFE;
        }
        if (in_quotes && *ip == '=') {
            *ip = 0xFD;
        }
        ip++;
    }
    cstring_replace(target, needle, replacement);
    ip = target;
    while (*ip) {
        if (*ip == 0xFF) *ip = ' ';
        if (*ip == 0xFE) *ip = '.';
        if (*ip == 0xFD) *ip = '=';
        ip++;
    }
}

static int massage(char *buff) {
    int i = program_replace_map->size;
    while (i--) {
        char *p = program_replace_map->items[i].from;
        while (*p) {
            *p = toupper(*p);
            p++;
        }
        p = program_replace_map->items[i].to;
        while (*p) {
            *p = toupper(*p);
            p++;
        }
        STR_REPLACE(
                buff,
                program_replace_map->items[i].from,
                program_replace_map->items[i].to);
    }
    STR_REPLACE(buff, "=<", "<=");
    STR_REPLACE(buff, "=>", ">=");
    // STR_REPLACE(buff, " ,", ",");
    // STR_REPLACE(buff, ", ", ",");
    // STR_REPLACE(buff, " *", "*");
    // STR_REPLACE(buff, "* ", "*");
    // STR_REPLACE(buff, "- ", "-");
    // STR_REPLACE(buff, " /", "/");
    // STR_REPLACE(buff, "/ ", "/");
    // STR_REPLACE(buff, "= ", "=");
    // STR_REPLACE(buff, "+ ", "+");
    // STR_REPLACE(buff, " )", ")");
    // STR_REPLACE(buff, ") ", ")");
    // STR_REPLACE(buff, "( ", "(");
    // STR_REPLACE(buff, "> ", ">");
    // STR_REPLACE(buff, "< ", "<");
    // STR_REPLACE(buff, " '", "'");
    return strlen(buff);
}

static int cmpstr(const char *s1, const char *s2) {
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

void program_dump_memory() {
    utility_dump_memory(ProgMemory);
}

// Tokenize the string in the edit buffer
static void program_tokenise(const char *file_path, const char *edit_buf) {
    //const char *p = edit_buf;
    //p++;
    //printf("<begin>\n");
    //printf("%s", edit_buf);
    //printf("<end>\n");

    // strcpy(CurrentFile, file_path);

    char *pmem = ProgMemory;

    // First line in the program memory should be a comment containing the 'CurrentFile'.
    memset(inpbuf, 0, INPBUF_SIZE);
    sprintf(inpbuf, "'%s", CurrentFile);
    tokenise(false);
    memcpy(pmem, tknbuf, strlen(tknbuf) + 1);
    pmem += strlen(tknbuf);
    pmem++;

    // Maximum extent of the program;
    // 4 characters are required for termination with 2-3 '\0' and a '\xFF'.
    const char *limit  = (const char *) ProgMemory + PROG_FLASH_SIZE - 5;

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
        if (pend - pstart > STRINGSIZE - 1) ERROR_LINE_TOO_LONG; // TODO: what cleans up the edit buffer ?
        memset(inpbuf, 0, INPBUF_SIZE);
        memcpy(inpbuf, pstart, pend - pstart);
        //printf("%s\n", inpbuf);

        program_transform_line(inpbuf);
        tokenise(false);

        //printf("* %s\n", tknbuf);

        for (char *pbuf = tknbuf; !(pbuf[0] == 0 && pbuf[1] == 0); pmem++, pbuf++) {
            if (pmem > limit) ERROR_OUT_OF_MEMORY;
            *pmem = *pbuf;
        }
        *pmem++ = '\0';  // write the terminating zero char

        pstart = pend + 1;
    }

    //printf("DONE\n");

    *pmem++ = '\0';
    *pmem++ = '\0';    // Two zeros terminate the program, but add an extra just in case.
    *pmem++ = '\xFF';  // A terminating 0xFF may also be expected; it's not completely clear.

    // We want CFunctionFlash to start on a 64-bit boundary.
    while ((uintptr_t) pmem % 8 != 0) *pmem++ = '\0';
    CFunctionFlash = pmem;

    if (errno != 0) error_throw(errno); // Is this really necessary?

    // program_dump_memory();
}

MmResult program_get_inc_file(const char *parent_file, const char *filename, char *out) {

    char path[STRINGSIZE];
    MmResult result = path_munge(filename, path, STRINGSIZE);
    if (FAILED(result)) return result;

    if (!path_is_absolute(path)) {
        char parent_dir[STRINGSIZE];
        result = path_get_parent(parent_file, parent_dir, STRINGSIZE);
        if (FAILED(result)) return result;

        if (FAILED(cstring_cat(parent_dir, "/", STRINGSIZE))
                || FAILED(cstring_cat(parent_dir, path, STRINGSIZE)))
            return kFilenameTooLong;

        strcpy(path, parent_dir);
    }

    assert(path_is_absolute(path));

    // If the file exists, or has a .inc file extension then return it.
    bool has_extension = strcasecmp(path_get_extension(path), INC_FILE_EXTENSIONS[0]) == 0;
    if (path_exists(path) || has_extension)
        return path_get_canonical(path, out, STRINGSIZE);

    // Try looking for the file with each extension/
    char *pend = path + strlen(path);
    for (size_t i = 0; i < sizeof(INC_FILE_EXTENSIONS) / sizeof(const char *); i++) {
        *pend = '\0';
        if (FAILED(cstring_cat(path, INC_FILE_EXTENSIONS[i], STRINGSIZE)))
            return kFilenameTooLong;
        if (path_exists(path))
            return path_get_canonical(path, out, STRINGSIZE);
    }

    // If all else fails return the path with the default '.inc' extension;
    // this will already be present in 'cwd'.
    return path_get_canonical(path, out, STRINGSIZE);
}

void program_init_defines() {
    program_replace_map = GetTempMemory(sizeof(ReplaceMap));
    program_replace_map->size = 0;
    program_file_stack = GetTempMemory(sizeof(ProgramFileStack));
    program_file_stack->size = 0;
}

void program_term_defines() {
    ClearSpecificTempMemory(program_replace_map);
    program_replace_map = NULL;
    ClearSpecificTempMemory(program_file_stack);
    program_file_stack = NULL;
}

MmResult program_add_define(const char *from, const char *to) {
    if (program_replace_map->size >= MAXDEFINES) return kTooManyDefines;
    strcpy(program_replace_map->items[program_replace_map->size].from, from);
    strcpy(program_replace_map->items[program_replace_map->size].to, to);
    program_replace_map->size++;
    return kOk;
}

int program_get_num_defines() {
    return program_replace_map->size;
}

MmResult program_get_define(size_t idx, const char **from, const char **to) {
    if (idx >= program_replace_map->size) return kInternalFault;
    *from = program_replace_map->items[program_replace_map->size - 1].from;
    *to = program_replace_map->items[program_replace_map->size - 1].to;
    return kOk;
}

MmResult program_process_line(char *line) {
    bool in_quotes = false;

    // Convert tab characters into single spaces in the line.
    for (size_t c = 0; c < strlen(line); c++) {
        if (line[c] == TAB) line[c] = ' ';
    }

    // sbuff points to the first non space character in the line.
    cstring_trim(line);

    // Replace REM command with ' comment character.
    // TODO: What if line contains REM command but it is not the first command ?
    if (strncasecmp(line, "REM ", 4) == 0 ||
        (strlen(line) == 3 && strncasecmp(line, "REM", 3) == 0)) {
        line[0] = ' ';
        line[1] = ' ';
        line[2] = SINGLE_QUOTE;
        cstring_trim(line);
    }

    // Are we processing a DATA command ?
    bool data = (strncasecmp(line, "DATA ", 5) == 0);

    // Compress multiple spaces into one.
    char *op = line;
    char *ip = line;
    while (*ip) {
        if (*ip == '"') {
            in_quotes = !in_quotes;
        }
        if (!in_quotes && (*ip == ' ')) {
            *op++ = *ip++;
            while (*ip == ' ') {
                ip++;
            }
        } else {
            *op++ = *ip++;
        }
    }
    *op = '\0';

    // Convert to upper-case and remove comments unless processing a DATA command.
    in_quotes = false;
    ip = line;
    while (*ip) {
        switch (*ip) {
            case '"':
                in_quotes = !in_quotes;
                break;
            case SINGLE_QUOTE:
                if (!in_quotes) {
                    *ip = '\0';
                    in_quotes = false;
                }
                break;
            default:
                if (!in_quotes && !data) *ip = toupper(*ip);
                break;
        }
        if (!*ip) break;
        ip++;
    }

    cstring_trim(line);

    if (*line == '#') return kOk; // pre-processor directive.

    // Close any open double-quote.
    if (in_quotes) cstring_cat(line, "\"", STRINGSIZE);

    massage(line);

    return kOk;
}

static void program_open_file(const char *filename) {
    char full_path[STRINGSIZE];
    MmResult result = program_file_stack->size == 0
            ? program_get_bas_file(filename, full_path)
            : program_get_inc_file(program_file_stack->files[0].filename, filename, full_path);
    switch (result) {
        case kOk:
            break;
        case kFileNotFound:
            if (program_file_stack->size == 0) {
                ERROR_PROGRAM_FILE_NOT_FOUND;
            } else {
                ERROR_INCLUDE_FILE_NOT_FOUND(filename);
            }
            break;
        case kFilenameTooLong:
            ERROR_PATH_TOO_LONG;
            break;
        default:
            error_throw(result);
            break;
    }

    int fnbr = file_find_free();
    file_open(full_path, "rb", fnbr);
    program_file_stack->head = &program_file_stack->files[program_file_stack->size];
    program_file_stack->head->fnbr = fnbr;
    program_file_stack->head->line_num = 0;
    program_file_stack->size++;

    if (program_file_stack->size == 1) {
        strcpy(CurrentFile, full_path);
        strcpy(program_file_stack->head->filename, full_path);
    } else {
        strcpy(program_file_stack->head->filename, filename);
    }
}

static void program_close_file() {
    if (program_file_stack->size == 0) ERROR_INTERNAL_FAULT;
    file_close(program_file_stack->head->fnbr);
    program_file_stack->head->filename[0] = '\0';
    program_file_stack->head->fnbr = -1;
    program_file_stack->head->line_num = -1;
    program_file_stack->size--;
    program_file_stack->head = program_file_stack->size == 0
            ? NULL
            : &program_file_stack->files[program_file_stack->size - 1];
}

MmResult program_process_file(const char *file_path, char **p, char *edit_buffer) {
    char line[STRINGSIZE];

    program_open_file(file_path);

    for (;;) {
        if ((*p - edit_buffer) >= EDIT_BUFFER_SIZE - 256 * 6) ERROR_OUT_OF_MEMORY;

        if (file_eof(program_file_stack->head->fnbr)) {
            program_close_file();
            if (!program_file_stack->head) break;
        }

        program_file_stack->head->line_num++;

        // Read a program line.
        memset(line, 0, STRINGSIZE);
        MMgetline(program_file_stack->head->fnbr, line);

        // Pre-process the line.
        MmResult result = program_process_line(line);
        if (FAILED(result)) return result;

        // Handle pre-processor directives.
        if (*line == '#') {
            const char *tp;
            if ((tp = checkstring(line, "#DEFINE"))) {
                getargs(&tp, 3, ",");
                // TODO: Free these strings.
                result = program_add_define(getCstring(argv[0]), getCstring(argv[2]));
                if (FAILED(result)) error_throw(result);
                continue;  // Don't write to edit buffer.
            } else if ((tp = checkstring(line, "#INCLUDE"))) {
                char *q;
                if ((q = strchr(tp, '"')) == 0) return kSyntax;
                q++;
                if ((q = strchr(q, '"')) == 0) return kSyntax;
                /*const*/ char *include_filename = getCstring(tp);
                program_open_file(include_filename);
                ClearSpecificTempMemory(include_filename);
                continue;  // Don't write to edit buffer.
            } else {
                continue;  // Ignore unknown directive.
            }
        }

        // Append file and line-number.
        result = cstring_cat(line, "'|", STRINGSIZE);
        if (program_file_stack->size > 1) {
            if (SUCCEEDED(result)) result = cstring_cat(line, program_file_stack->head->filename, STRINGSIZE);
            if (SUCCEEDED(result)) result = cstring_cat(line, ",", STRINGSIZE);
        }
        if (SUCCEEDED(result)) result = cstring_cat_int64(line, program_file_stack->head->line_num, STRINGSIZE);
        if (FAILED(result)) ERROR_LINE_TOO_LONG;

        // Copy the transformed line into the edit buffer, terminating with '\n'.
        size_t len = strlen(line);
        if ((line[0] != SINGLE_QUOTE) || (line[0] == SINGLE_QUOTE && line[1] == SINGLE_QUOTE)) {
            memcpy(*p, line, len);
            *p += len;
            **p = '\n';
            *p += 1;
        }
    }

    return kOk;
}

static bool program_path_exists(const char *root, const char *stem, const char *extension) {
    char path[STRINGSIZE] = { '\0' };
    if (FAILED(cstring_cat(path, root, STRINGSIZE))
            || FAILED(cstring_cat(path, stem, STRINGSIZE))
            || FAILED(cstring_cat(path, extension, STRINGSIZE))) {
        return false;
    }
    return path_exists(path);
}

MmResult program_get_bas_file(const char *filename, char *out) {

    char path[STRINGSIZE];
    MmResult result = path_munge(filename, path, STRINGSIZE);
    if (FAILED(result)) return result;

    bool is_absolute = path_is_absolute(path);
    bool has_extension = strcasecmp(path_get_extension(path), BAS_FILE_EXTENSIONS[0]) == 0;

    // If the specified file exists, or is absolute and has a .bas file
    // extension then return it.
    if (path_exists(path) || (is_absolute && has_extension))
        return path_get_canonical(path, out, STRINGSIZE);

    // If the specified file is absolute but doesn't have a .bas file
    // extension then try looking for the file with each extension
    // in turn. If none of them are found then use the last extension;
    // which is the default (and the the same as the first extension).
    if (is_absolute) {
        char *p = path + strlen(path);
        for (size_t i = 0; i < sizeof(BAS_FILE_EXTENSIONS) / sizeof(const char *); i++) {
            *p = '\0';
            if (FAILED(cstring_cat(path, BAS_FILE_EXTENSIONS[i], STRINGSIZE)))
                return kFilenameTooLong;
            if (path_exists(path)) break;
        }
        return path_get_canonical(path, out, STRINGSIZE);
    }

    // If we get here then the filename is relative and does not exist as
    // specified, it may or may not have a .bas file extension.

    // Get the path resolved relative to the current working directory (CWD).
    char cwd[STRINGSIZE] = { '\0'};
    errno = 0;
    if (!getcwd(cwd, STRINGSIZE)) return errno;
    if (FAILED(cstring_cat(cwd, "/", STRINGSIZE))
            || FAILED(cstring_cat(cwd, path, STRINGSIZE)))
        return kFilenameTooLong;

    // Get the path resolved relative to the SEARCH PATH.
    char search_path[STRINGSIZE] = { '\0' };
    if (FAILED(cstring_cat(search_path, mmb_options.search_path, STRINGSIZE)))
        return kFilenameTooLong;
    if (*search_path) {
        if (FAILED(cstring_cat(search_path, "/", STRINGSIZE))
                || FAILED(cstring_cat(search_path, path, STRINGSIZE)))
            return kFilenameTooLong;
    }

    // Note we don't have to check here if the file exists in the CWD;
    // if that were the case we would have caught it in the first IF statement.

    // If the file exists in the SEARCH PATH then return it.
    if (*search_path && path_exists(search_path))
        return path_get_canonical(search_path, out, STRINGSIZE);

    // If the file has .bas extension then return path resolved relative to CWD.
    if (has_extension)
        return path_get_canonical(cwd, out, STRINGSIZE);

    // Try looking for the file with each extension resolved relative to CWD.
    char *pend = cwd + strlen(cwd);
    for (size_t i = 0; i < sizeof(BAS_FILE_EXTENSIONS) / sizeof(const char *); i++) {
        *pend = '\0';
        if (FAILED(cstring_cat(cwd, BAS_FILE_EXTENSIONS[i], STRINGSIZE)))
            return kFilenameTooLong;
        if (path_exists(cwd))
            return path_get_canonical(cwd, out, STRINGSIZE);
    }

    // Try looking for the file with each extension resolved relative to SEARCH PATH.
    pend = search_path + strlen(search_path);
    for (size_t i = 0; i < sizeof(BAS_FILE_EXTENSIONS) / sizeof(const char *); i++) {
        *pend = '\0';
        if (FAILED(cstring_cat(search_path, BAS_FILE_EXTENSIONS[i], STRINGSIZE)))
            return kFilenameTooLong;
        if (path_exists(search_path))
            return path_get_canonical(search_path, out, STRINGSIZE);
    }

    // If all else fails return the path resolved relative to CWD with the
    // default .bas extension; this will already be present in 'cwd'.
    return path_get_canonical(cwd, out, STRINGSIZE);
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
    CommandToken end_token = INVALID_COMMAND_TOKEN;
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

        if (commandtbl_decode(p) == cmdCSUB) {

            end_token = cmdEND_CSUB;
            *((uint64_t *) flash_ptr) = (uintptr_t) p;
            flash_ptr += 2;
            save_addr = flash_ptr; // Save where we are so that we can write the CSub size in here
            flash_ptr ++;
            p += sizeof(CommandToken);
            skipspace(p);
            if (!isnamestart(*p)) ERROR_FUNCTION_NAME;
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
                while (*p && *p != SINGLE_QUOTE) {
                    skipspace(p);
                    int n = 0;
                    for (int i = 0; i < 8; i++) {
                        if (!isxdigit(*p)) ERROR_INVALID_HEX;
                        n = n << 4;
                        if (*p <= '9')
                            n |= (*p - '0');
                        else
                            n |= (toupper(*p) - 'A' + 10);
                        p++;
                    }
                    if ((char *) flash_ptr >= (char *) ProgMemory + PROG_FLASH_SIZE - 9) ERROR_OUT_OF_MEMORY;
                    *flash_ptr = n;
                    flash_ptr ++;
                    skipspace(p);
                }
                // we are at the end of a embedded code line
                while (*p) p++;  // make sure that we move to the end of the line
                p++;      // step to the start of the next line
                if (*p == 0) ERROR_MISSING_END;
                if (*p == T_NEWLINE) {
                    CurrentLinePtr = p;
                    p++;  // skip the newline token
                }
                if (*p == T_LINENBR) p += 3;  // skip over the line number
                skipspace(p);
            } while (commandtbl_decode(p) != end_token);

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

static void print_line(const char *buf, int* line_count, int all) {
    console_puts(buf);
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
                cstring_cat(buf, ", ", STRINGSIZE);
            }
            sprintf(name, "0x%08X", *p++);
            cstring_cat(buf, name, STRINGSIZE);
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

int program_load_file(char *filename) {
    // Store the current token buffer incase we are at the command prompt.
    char tmp[TKNBUF_SIZE];
    memcpy(tmp, tknbuf, TKNBUF_SIZE);

    // Store a copy of the filename on the stack so it is not trampled on by ClearProgram().
    char filename2[STRINGSIZE];
    strcpy(filename2, filename);

    ClearProgram();

    // TODO: are these being properly released after a longjmp() ?
    char *edit_buffer = GetTempMemory(EDIT_BUFFER_SIZE);
    char *p = edit_buffer;
    program_init_defines();
    MmResult result = program_process_file(filename2, &p, edit_buffer);

    // Ensure every program has an END (and a terminating '\0').
    if (p - edit_buffer > EDIT_BUFFER_SIZE - 5) ERROR_OUT_OF_MEMORY;
    memcpy(p, "END\n", 5);
    p += 5;

    program_tokenise(CurrentFile, edit_buffer);

    ClearSpecificTempMemory(edit_buffer);
    program_term_defines();

    program_process_csubs();

    // Restore the token buffer.
    memcpy(tknbuf, tmp, TKNBUF_SIZE);

    // Set the console window title.
    char title[STRINGSIZE + 10];
    sprintf(title, "MMBasic - %s", CurrentFile);
    console_set_title(title);

    if (errno != 0) error_throw(errno); // Is this really necessary?

    return result;
}
