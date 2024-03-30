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

#define ERROR_INVALID_FUNCTION_NAME  error_throw_ex(kInvalidName, "Invalid function name")
#define ERROR_INVALID_HEX            ERROR_INVALID("hex word")
#define ERROR_MISSING_END            error_throw_ex(kError, "Missing END command")

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

/** Current insertion point into ProgramMemory. */
static char *program_progmem_insert = NULL;

/** Maximum extent of the program. */
static char *program_progmem_limit = NULL;

static size_t program_comment_level = 0;  // Level of nesting within /* */ style comment ?

static bool program_debug_on = false;  // Is there an active #MMDEBUG ON directive ?

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

static void program_apply_replacements(char *line) {
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
                line,
                program_replace_map->items[i].from,
                program_replace_map->items[i].to);
    }
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

void program_dump_memory() {
    utility_dump_memory(ProgMemory);
}

static MmResult program_append_to_progmem(const char *src) {
    for (const char *p = src; !(p[0] == '\0' && p[1] == '\0'); program_progmem_insert++, p++) {
        if (program_progmem_insert > program_progmem_limit) return kProgramTooLong;
        *program_progmem_insert = *p;
    }
    *program_progmem_insert++ = '\0';  // Include terminating \0.
    return kOk;
}

/**
 * The first line in the ProgramMemory should be a comment containing the CurrentFile.
 */
static MmResult program_append_header() {
    memset(inpbuf, 0, INPBUF_SIZE);
    sprintf(inpbuf, "'%s", CurrentFile);
    tokenise(false);
    MmResult result = program_append_to_progmem(tknbuf);
    return result;
}

static MmResult program_append_footer() {
    // Ensure program has a final END command.
    memset(inpbuf, 0, INPBUF_SIZE);
    sprintf(inpbuf, "END");  // NOTE: There is no line number comment on this line.
    tokenise(false);
    MmResult result = program_append_to_progmem(tknbuf);
    if (FAILED(result)) return result;

    // The program should be terminated by at least two zeroes (NOTE: it may already have one).
    // A terminating 0xFF may also be expected; it's not completely clear.
    *program_progmem_insert++ = '\0';
    *program_progmem_insert++ = '\0';
    *program_progmem_insert++ = '\xFF';

    // We want CFunctionFlash to start on a 64-bit boundary.
    while ((uintptr_t) program_progmem_insert % 8 != 0) *program_progmem_insert++ = '\0';
    CFunctionFlash = program_progmem_insert;

    return kOk;
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

/**
 * @brief Allocates memory for internal data-structures.
 *
 * Only visible for unit-testing.
 */
static void program_internal_alloc() {
    program_replace_map = GetTempMemory(sizeof(ReplaceMap));
    program_replace_map->size = 0;
    program_file_stack = GetTempMemory(sizeof(ProgramFileStack));
    program_file_stack->size = 0;
    program_progmem_insert = ProgMemory;

    // 4 characters are required for termination with 2-3 '\0' and a '\xFF'.
    program_progmem_limit = ProgMemory + PROG_FLASH_SIZE - 5;
}

/**
 * @brief Frees memory for internal data-structures.
 *
 * Only visible for unit-testing.
 */
static void program_internal_free() {
    ClearSpecificTempMemory(program_replace_map);
    program_replace_map = NULL;
    ClearSpecificTempMemory(program_file_stack);
    program_file_stack = NULL;
    program_progmem_insert = NULL;
    program_progmem_limit = NULL;
}

/**
 * @brief Add an entry to the string replacement map.
 *
 * @param  from  The 'from' string.
 * @param  to    The 'to' string.
 * @return       kOk on success,
 *               kTooManyDefines if there are too many #DEFINEs.
 */
static MmResult program_add_define(const char *from, const char *to) {
    if (program_replace_map->size >= MAXDEFINES) return kTooManyDefines;
    strcpy(program_replace_map->items[program_replace_map->size].from, from);
    strcpy(program_replace_map->items[program_replace_map->size].to, to);
    program_replace_map->size++;
    return kOk;
}

/**
 * @brief Gets the number of entries in the string replacement map.
 */
static int program_get_num_defines() {
    return program_replace_map->size;
}

/**
 * @brief Gets an entry from the string replacement map.
 *
 * @param       idx   Index of the entry to get.
 * @param[out]  from  On exit points to the entry's 'from' property.
 * @param[out]  to    On exit points to the entry's 'from' property.
 * @return            kOk on success.
 */
static MmResult program_get_define(size_t idx, const char **from, const char **to) {
    if (idx >= program_replace_map->size) return kInternalFault;
    *from = program_replace_map->items[program_replace_map->size - 1].from;
    *to = program_replace_map->items[program_replace_map->size - 1].to;
    return kOk;
}

/**
 * @brief Pre-process a single line of the program (in place).
 *
 * @param[in,out]  line  The line to pre-process.
 * @return               kOk on success.
 */
static MmResult program_process_line(char *line) {
    char *op = line;
    const char *ip = line;
    bool expecting_command = true;  // Are we expecing a BASIC command ?
    bool in_data = false;           // Are we processing a DATA command ?
    bool in_quotes = false;         // Are we within double-quotes ?

    while (*ip) {

        // Handle multiline /* */ comments.
        if (program_comment_level) {
            if (*ip == '*' && *(ip + 1) == '/') {
                program_comment_level--;
                ip++;
            } else if (*ip == '/' && *(ip + 1) == '*') {
                program_comment_level++;
                ip++;
            } else if (*ip == '#') {
                // ... and CMM2 style #COMMENT {START|END}.
                const char *q, *r;
                if ((q = checkstring(ip + 1, "COMMENT"))) {
                    if ((r = checkstring(q, "START"))) {
                        program_comment_level++;
                        ip = r;
                    } else if ((r = checkstring(q, "END"))) {
                        program_comment_level--;
                        ip = r;
                    } else {
                        return kSyntax;
                    }
                    if (!parse_is_end(r)) return kSyntax;
                }
            }
            ip++;
            continue;
        }

        switch (*ip) {
            case '"':
                in_quotes = !in_quotes;
                *op++ = *ip++;
                break;

            case ' ':
            case TAB:
                if (expecting_command) {
                    // Ignore leading spaces before a command.
                    while (*ip == ' ' || *ip == TAB) ip++;
                    continue;  // So we don't set expecting_command = false.
                }
                // Convert tabs into spaces.
                //   ... though this may be irrelevant since lines will have been processed
                //       by MMgetline which already expands tabs according to the value of
                //       mmb_options.tab.
                *op++ = ' ';
                ip++;
                if (!in_quotes) {
                    // Compress multiple spaces except within a string
                    while (*ip == ' ' || *ip == TAB) ip++;
                }
                break;

            case SINGLE_QUOTE:
                if (in_quotes) {
                    *op++ = *ip++;
                } else {
                    // Strip single-quote comments.
                    while (*ip) ip++;
                }
                break;

            case ':':
                *op++ = *ip++;
                if (!in_quotes) {
                    expecting_command = true;
                    continue;  // So we don't set expecting_command = false.
                }
                break;

            case '/':
                if (!in_quotes && *(ip + 1) == '*') {
                    program_comment_level++;
                    ip += 2;
                } else {
                    *op++ = *ip++;
                }
                break;

            case '*':
                if (!in_quotes && *(ip + 1) == '/') return kNoCommentToTerminate;
                *op++ = *ip++;
                break;

            default:
                if (expecting_command) {
                    if (strncasecmp(ip, "DATA", 4) == 0 && !isnamechar(*(ip + 4))) {
                        in_data = true;
                    } else if (strncasecmp(ip, "MMDEBUG", 7) == 0 && !isnamechar(*(ip + 7))) {
                        if (!program_debug_on) {
                            // If not within #MMDEBUG ON then strip line.
                            // BUG! Strips entire line even if there are multiple commands.
                            while (*ip) ip++;
                            break;
                        }
                    } else if (strncasecmp(ip, "REM", 3) == 0 && !isnamechar(*(ip + 3))) {
                        // Strip REM comments.
                        while (*ip) ip++;
                        break;
                    }
                }
                // Convert to upper-case except within strings or DATA commands.
                *op++ = (in_quotes || in_data) ? *ip++ : toupper(*ip++);
                break;
        }
        expecting_command = false;
    }

    // Strip trailing spaces.
    // NOTE: We do not naively strip trailing ':' because they may terminate a label.
     while (op != line && *(op - 1) == ' ') op--;

    // Close any open-quote.
    if (in_quotes) {
        if (op - line >= STRINGSIZE - 1) return kLineTooLong;
        *op++ = '"';
    }

    // Terminate the buffer.
    *op = '\0';

    // Apply replacements unless the line starts with a directive.
    if (*line != '#') program_apply_replacements(line);

    return kOk;
}

static MmResult program_open_file(const char *filename) {
    char full_path[STRINGSIZE];
    MmResult result = program_file_stack->size == 0
            ? program_get_bas_file(filename, full_path)
            : program_get_inc_file(program_file_stack->files[0].filename, filename, full_path);
    if (FAILED(result)) return result;
    if (!path_exists(full_path)) return kFileNotFound;

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

    // Override default error handling to report file/line even though we are
    // not yet executing a program.
    mmb_error_state_ptr->override_line = true;
    mmb_error_state_ptr->line = 0;
    strcpy(mmb_error_state_ptr->file, program_file_stack->head->filename);

    return kOk;
}

static MmResult program_close_file() {
    if (program_file_stack->size == 0) return kInternalFault;
    file_close(program_file_stack->head->fnbr);
    program_file_stack->head->filename[0] = '\0';
    program_file_stack->head->fnbr = -1;
    program_file_stack->head->line_num = -1;
    program_file_stack->size--;
    program_file_stack->head = program_file_stack->size == 0
            ? NULL
            : &program_file_stack->files[program_file_stack->size - 1];

    if (program_file_stack->head) {
        strcpy(mmb_error_state_ptr->file, program_file_stack->head->filename);
    }

    return kOk;
}

static MmResult program_handle_comment_directive(const char *p) {
    const char *q;
    if ((q = checkstring(p, "START"))) {
        program_comment_level++;
    } else if ((q = checkstring(p, "END"))) {
        // Should never get here, instead handled in program_process_line().
        return kInternalFault;
    } else {
        return kSyntax;
    }
    return parse_is_end(q) ? kOk : kSyntax;
}

static MmResult program_handle_define_directive(const char *p) {
    getargs(&p, 3, ",");
    if (argc != 3) return kSyntax;
    /*const*/ char *from = getCstring(argv[0]);
    /*const*/ char *to = getCstring(argv[2]);
    MmResult result = program_add_define(from, to);
    ClearSpecificTempMemory(from);
    ClearSpecificTempMemory(to);
    return result;
}

static MmResult program_handle_include_directive(const char *p) {
    char *q;
    if ((q = strchr(p, '"')) == 0) return kSyntax;
    q++;
    if ((q = strchr(q, '"')) == 0) return kSyntax;
    /*const*/ char *include_filename = getCstring(p);
    MmResult result = program_open_file(include_filename);
    ClearSpecificTempMemory(include_filename);
    return result;
}

static MmResult program_handle_mmdebug_directive(const char *p) {
    const char *q;
    if ((q = checkstring(p, "ON"))) {
        // NOTE: There is no error/warning if already ON.
        program_debug_on = true;
    } else if ((q = checkstring(p, "OFF"))) {
        // NOTE: There is no error/warning if already OFF.
        program_debug_on = false;
    } else {
        return kSyntax;
    }
    return parse_is_end(q) ? kOk : kSyntax;
}

static MmResult program_handle_directive(const char *line) {
    const char *p;
    if ((p = checkstring(line, "#COMMENT"))) {
        return program_handle_comment_directive(p);
    } else if ((p = checkstring(line, "#DEFINE"))) {
        return program_handle_define_directive(p);
    } else if ((p = checkstring(line, "#INCLUDE"))) {
        return program_handle_include_directive(p);
    } else if ((p = checkstring(line, "#MMDEBUG"))) {
        return program_handle_mmdebug_directive(p);
    } else {
        // Unknown directives are ignored.
        return kOk;
    }
}

/**
 * @brief Process files from the stack a line at a time.
 *
 * @return  kOk on success.
 */
MmResult program_process_file() {
    MmResult result = kOk;
    program_comment_level = 0;

    for (;;) {
        if (file_eof(program_file_stack->head->fnbr)) {
            result = program_close_file();
            if (FAILED(result)) break;
            if (!program_file_stack->head) break;
        }

        program_file_stack->head->line_num++;
        mmb_error_state_ptr->line = program_file_stack->head->line_num;

        // Read a program line.
        memset(inpbuf, 0, STRINGSIZE);
        MMgetline(program_file_stack->head->fnbr, inpbuf);

        // Pre-process the line.
        result = program_process_line(inpbuf);
        if (FAILED(result)) break;

        // Handle pre-processor directives.
        // A directive must be the first/only thing on a line. Do not catch illegal usage here
        // but instead rely on the interpreter reporting invalid usage of the # character.
        if (*inpbuf == '#') {
            result = program_handle_directive(inpbuf);
            if (FAILED(result)) break;
            continue; // Don't write to ProgMemory.
        }

        if (*inpbuf) {
            // Append file and line-number.
            result = cstring_cat(inpbuf, "'|", STRINGSIZE);
            if (program_file_stack->size > 1) {
                if (SUCCEEDED(result)) result = cstring_cat(inpbuf, program_file_stack->head->filename, STRINGSIZE);
                if (SUCCEEDED(result)) result = cstring_cat(inpbuf, ",", STRINGSIZE);
            }
            if (SUCCEEDED(result)) result = cstring_cat_int64(inpbuf, program_file_stack->head->line_num, STRINGSIZE);
            if (FAILED(result)) {
                result = kLineTooLong;
                break;
            }

            // Tokenise and append to program.
            tokenise(false);
            result = program_append_to_progmem(tknbuf);
            if (FAILED(result)) break;
        }
    }

    if (SUCCEEDED(result) && program_comment_level) {
        result = kUnterminatedComment;
    }

    if (SUCCEEDED(result)) {
        // Restore default error handling.
        mmb_error_state_ptr->override_line = false;
    }

    return result;
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
            if (!isnamestart(*p)) ERROR_INVALID_FUNCTION_NAME;
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

MmResult program_load_file(const char *filename) {
    // Store the current token buffer incase we are at the command prompt.
    char tmp[TKNBUF_SIZE];
    memcpy(tmp, tknbuf, TKNBUF_SIZE);

    // Store a copy of the filename on the stack so it is not trampled on by ClearProgram().
    char filename2[STRINGSIZE];
    strcpy(filename2, filename);

    ClearProgram();

    program_internal_alloc();

    MmResult result = program_open_file(filename2);
    if (SUCCEEDED(result)) result = program_append_header();
    if (SUCCEEDED(result)) result = program_process_file();
    if (SUCCEEDED(result)) result = program_append_footer();
    program_internal_free();
    if (SUCCEEDED(result)) program_process_csubs();
    memcpy(tknbuf, tmp, TKNBUF_SIZE);  // Restore the token buffer.

    if (SUCCEEDED(result)) {
        // Set the console window title.
        char title[STRINGSIZE + 10];
        sprintf(title, "MMBasic - %s", CurrentFile);
        console_set_title(title);
    }

    // TODO: Is the 'errno' check really necessary?
    return SUCCEEDED(result) ? errno : result;
}
