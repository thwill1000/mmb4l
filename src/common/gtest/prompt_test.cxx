/*
 * Copyright (c) 2022-2023 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <climits>
#include <dirent.h>

#include "test_config.h"

extern "C" {

#include "../prompt.h"
#include "../utility.h"

char inpbuf[INPBUF_SIZE] = { '\0' };
int MMCharPos = 0;
Options mmb_options;
PromptState prompt_state;
bool console_bell_sounded;
MmResult path_complete_canned_result;
char path_complete_captured_path[STRINGSIZE];

void console_bell(void) { console_bell_sounded = true; }
int console_get_size(int *width, int *height) { return -1; }
char console_putc(char c) { return -1; }
void error_throw(MmResult error) { }
void error_throw_ex(MmResult error, const char *msg, ...) { }
int MMgetchar(void) { return -1; }
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
