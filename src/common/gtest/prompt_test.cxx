/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <climits>
#include <dirent.h>

#include "test_config.h"

extern "C" {

#include "../console.h"
#include "../prompt.h"
#include "../utility.h"

#define CONSOLE_WIDTH   80

char inpbuf[STRINGSIZE] = { '\0' };
int MMCharPos = 1;
Options mmb_options = { 0 };
PromptState prompt_state;
bool console_bell_sounded;
MmResult path_complete_canned_result;
char path_complete_captured_path[STRINGSIZE];
char MMgetchar_canned_results[512];
char *MMgetchar_canned_results_ptr;
char console_putc_captured_c[512];
char *console_putc_captured_c_ptr;

void console_bell(void) {
    console_bell_sounded = true;
}

void console_cursor_down(uint8_t i) {
    char buf[32];
    sprintf(buf, "\033[%dB", i);
    char *p = buf;
    while (*p) (void) console_putc(*p++);
}

void console_cursor_left(uint8_t i) {
    char buf[32];
    sprintf(buf, "\033[%dD", i);
    char *p = buf;
    while (*p) (void) console_putc(*p++);
    MMCharPos -= i;
}

void console_cursor_right(uint8_t i) {
    char buf[32];
    sprintf(buf, "\033[%dC", i);
    char *p = buf;
    while (*p) (void) console_putc(*p++);
    MMCharPos += i;
}

void console_cursor_up(uint8_t i) {
    char buf[32];
    sprintf(buf, "\033[%dA", i);
    char *p = buf;
    while (*p) (void) console_putc(*p++);
}

int console_get_size(int *width, int *height) {
    *width = CONSOLE_WIDTH;
    *height = 40;
    return 0;
}

char console_putc(char c) {
    if (console_putc_captured_c_ptr >= console_putc_captured_c + 511) {
        fprintf(stderr, "console_putc() overflow:\n");
        fprintf(stderr, "%s\n", console_putc_captured_c);
        exit(-1);
    }
    *console_putc_captured_c_ptr++ = c;
    *console_putc_captured_c_ptr = '\0';
    return c;
}

void error_throw(MmResult error) { }
void error_throw_ex(MmResult error, const char *msg, ...) { }

int MMgetchar(void) {
    int result = *MMgetchar_canned_results_ptr++;
    return result = '\0' ? -1 : result;
}

void MMgetline(int filenbr, char *p) { }

void console_puts(const char* s) {
    while (*s) console_putc(*s++);
}

MmResult path_complete(const char *path, char *out, size_t sz) {
    strcpy(path_complete_captured_path, path); // Capture path.
    if (*path) {
        strcpy(out, "-completed");   // Return canned output.
    } else {
        strcpy(out, "");
    }
    return path_complete_canned_result;
}

}

#define TEST_DIR  TMP_DIR "/PromptTest"

class PromptTest : public ::testing::Test {

private:

    std::string m_cwd;

protected:

    void SetUp() override {
        struct stat st = { 0 };
        if (stat(TEST_DIR, &st) == -1) {
            mkdir(TEST_DIR, 0775);
        }

        // Cache current working directory.
        char *cwd = getcwd(NULL, 0);
        m_cwd = cwd;
        free(cwd);
    }

    void TearDown() override {
        SYSTEM_CALL("rm -rf " TEST_DIR);
        CHDIR(m_cwd.c_str());
    }
};

#define TEST_HANDLE_TAB(input, expected_path, expected_buf, expected_bell) \
    strcpy(inpbuf, input); \
    strcpy(path_complete_captured_path, ""); \
    strcpy(prompt_state.buf, "\tABCDEF"); \
    console_bell_sounded = false; \
    prompt_handle_tab(&prompt_state); \
    EXPECT_STREQ(input, inpbuf); \
    EXPECT_STREQ(expected_path, path_complete_captured_path); \
    EXPECT_STREQ(expected_buf, prompt_state.buf); \
    EXPECT_EQ(expected_bell, console_bell_sounded);

TEST_F(PromptTest, HandleTab_GivenSuccess) {
    path_complete_canned_result = kOk;

    // Path is the first thing in the inpbuf.
    TEST_HANDLE_TAB("foo.b", "foo.b", "\t-completed", false);
    TEST_HANDLE_TAB(TEST_DIR "/foo.b", TEST_DIR "/foo.b", "\t-completed", false);

    // Path is the second and last thing in the inpbuf.
    TEST_HANDLE_TAB("!ls foo.b", "foo.b", "\t-completed", false);
    TEST_HANDLE_TAB("!ls  foo.b", "foo.b", "\t-completed", false);
    TEST_HANDLE_TAB("!ls " TEST_DIR "/foo.b", TEST_DIR "/foo.b", "\t-completed", false);

    // Path is quoted.
    TEST_HANDLE_TAB("!ls \"foo.b", "foo.b", "\t-completed", false);
    TEST_HANDLE_TAB("!ls \"", "", "\t", true);
    TEST_HANDLE_TAB("!ls \"has spaces", "has spaces", "\t-completed", false);
    TEST_HANDLE_TAB("!ls \"has  spaces", "has  spaces", "\t-completed", false);
    TEST_HANDLE_TAB("!ls \"foo.b\"", "", "\t", true);

    // Path is third thing in the inpbuf.
    TEST_HANDLE_TAB("!ls wombat  foo.b", "foo.b", "\t-completed", false);
    TEST_HANDLE_TAB("!ls wombat \"has spaces", "has spaces", "\t-completed", false);
    TEST_HANDLE_TAB("!ls   wombat \"has spaces", "has spaces", "\t-completed", false);
    TEST_HANDLE_TAB("!ls wombat \"foo.b\"", "", "\t", true);
}

TEST_F(PromptTest, HandleTab_GivenFailure) {
    path_complete_canned_result = kError;

    TEST_HANDLE_TAB("foo.b", "foo.b", "\t", true);
}

#define TEST_GET_INPUT(input, expected_inpbuf, expected_console_output) \
    strcpy(inpbuf, ""); \
    strcpy(MMgetchar_canned_results, input); \
    MMgetchar_canned_results_ptr = MMgetchar_canned_results; \
    strcpy(console_putc_captured_c, ""); \
    console_putc_captured_c_ptr = console_putc_captured_c; \
    prompt_get_input(); \
    EXPECT_STREQ(expected_inpbuf, inpbuf); \
    EXPECT_STREQ(expected_console_output, console_putc_captured_c)


TEST_F(PromptTest, GetInput_GivenEmptyInput) {
    TEST_GET_INPUT("\n", "", "\r\n");
}

TEST_F(PromptTest, GetInput_GivenSimpleInput) {
    TEST_GET_INPUT("foo bar\n", "foo bar", "foo bar\r\n");
}

TEST_F(PromptTest, GetInput_GivenInputOfMoreThanConsoleWidthChars) {
    char input[256];
    memset(input, 'a', CONSOLE_WIDTH + 20);
    input[CONSOLE_WIDTH + 20] = '\n';
    char expected_inpbuf[256];
    memset(expected_inpbuf, 'a', 100);
    expected_inpbuf[CONSOLE_WIDTH + 20] = '\0';
    char expected_console_output[256];
    memset(expected_console_output, 'a', CONSOLE_WIDTH + 20);
    expected_console_output[CONSOLE_WIDTH + 20] = '\r';
    expected_console_output[CONSOLE_WIDTH + 21] = '\n';

    TEST_GET_INPUT(input, expected_inpbuf, expected_console_output);
}

TEST_F(PromptTest, GetInput_GivenInputOf255Chars) {
    char input[300];
    memset(input, 'a', 255);
    input[255] = '\n';
    char expected_inpbuf[300];
    memset(expected_inpbuf, 'a', 255);
    expected_inpbuf[255] = '\0';
    char expected_console_output[300];
    memset(expected_console_output, 'a', 255);
    expected_console_output[255] = '\r';
    expected_console_output[256] = '\n';

    TEST_GET_INPUT(input, expected_inpbuf, expected_console_output);
}

TEST_F(PromptTest, GetInput_GivenInputOfMoreThan255Chars) {
    char input[300];
    memset(input, 'a', 256);
    input[256] = '\n';
    char expected_inpbuf[300];
    memset(expected_inpbuf, 'a', 255);
    expected_inpbuf[255] = '\0';
    char expected_console_output[300];
    memset(expected_console_output, 'a', 255);
    expected_console_output[255] = '\r';
    expected_console_output[256] = '\n';

    TEST_GET_INPUT(input, expected_inpbuf, expected_console_output);
}

TEST_F(PromptTest, GetInput_GivenBackspaceAtEndOfInput) {
    TEST_GET_INPUT("foobar\b\n", "fooba", "foobar\b \b\r\n");
}

TEST_F(PromptTest, GetInput_GivenBackspaceInMiddleOfInput) {
    // 0x82 is left arrow
    TEST_GET_INPUT("foobar\x82\x82\x82\b\n", "fobar", "foobar\b\b\b\bbar \b\b\b\b\r\n");
}
