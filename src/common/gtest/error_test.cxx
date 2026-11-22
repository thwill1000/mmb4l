/*
 * Copyright (c) 2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "../error.h"
#include "../mmb4l.h"

// Globals and functions required by error.c.
char CurrentFile[STRINGSIZE];
const char *CurrentLinePtr = nullptr;
char ProgMemory[PROG_FLASH_SIZE];
char tknbuf[STRINGSIZE];
jmp_buf ErrNext;
jmp_buf mark;

Options mmb_options;
char options_filename[PATH_MAX];

ErrorState mmb_normal_error_state;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;

LoggerLevel logger_min_level = kLoggerLevelNone;

const char *llist(char *b, const char *p) {
    (void) b;
    return p;
}

MmResult options_load(Options *options, const char *filename, OPTIONS_WARNING_CB warning_cb) {
    (void) options;
    (void) filename;
    (void) warning_cb;
    return kOk;
}

MmResult path_get_parent(const char *path, char *parent_path, size_t sz) {
    (void) path;
    (void) parent_path;
    (void) sz;
    return kError;
}

MmResult path_get_canonical(const char *path, char *canonical_path, size_t sz) {
    (void) path;
    (void) canonical_path;
    (void) sz;
    return kError;
}

void logger_write(LoggerLevel level, const char *file, unsigned line, const char *function,
                  const char *format, ...) {
    (void) level;
    (void) file;
    (void) line;
    (void) function;
    (void) format;
}

void IntToStr(char *strr, MMINTEGER nbr, unsigned int base) {
    if (base == 16) {
        std::snprintf(strr, 20, "%llX", (long long) nbr);
    } else {
        std::snprintf(strr, 20, "%lld", (long long) nbr);
    }
}

}  // extern "C"

namespace {

void format_error(const char *msg, ...) {
    va_list argp;
    va_start(argp, msg);
    error_set_message(kError, msg, argp);
    va_end(argp);
}

TEST(ErrorTest, FormatsMessageForCurrentFile) {
    ASSERT_EQ(kOk, error_init(&mmb_normal_error_state));
    std::strcpy(CurrentFile, "/tmp/main.bas");
    std::strcpy(mmb_error_state_ptr->file, CurrentFile);
    mmb_error_state_ptr->line = 42;

    format_error("Bad $ @ % | & \\$ \\@ \\% \\| \\&", (char *) "thing", '!', 10, 11, 12);

    EXPECT_STREQ("Error in line 42: Bad thing ! 10 11 C $ @ % | &", mmb_error_state_ptr->message);
}

TEST(ErrorTest, FormatsMessageForDifferentFile) {
    ASSERT_EQ(kOk, error_init(&mmb_normal_error_state));
    std::strcpy(CurrentFile, "/tmp/main.bas");
    std::strcpy(mmb_error_state_ptr->file, "/tmp/inc.bas");
    mmb_error_state_ptr->line = 9;

    format_error("Oops");

    EXPECT_STREQ("Error in /tmp/inc.bas line 9: Oops", mmb_error_state_ptr->message);
}

TEST(ErrorTest, FormatsMessageWithoutLineNumber) {
    ASSERT_EQ(kOk, error_init(&mmb_normal_error_state));
    std::strcpy(CurrentFile, "/tmp/main.bas");
    std::strcpy(mmb_error_state_ptr->file, CurrentFile);
    mmb_error_state_ptr->line = -1;

    format_error("Oops");

    EXPECT_STREQ("Error: Oops", mmb_error_state_ptr->message);
}

}  // namespace
