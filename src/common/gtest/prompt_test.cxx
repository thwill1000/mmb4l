/*
 * Copyright (c) 2022-2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <climits>
#include <filesystem>

extern "C" {

#include "../cstring.h"
#include "../file.h"
#include "../path.h"
#include "../prompt.h"
#include "../prompt_private.h"
#include "../utility.h"

extern bool display_bell_sounded;
char inpbuf[INPBUF_SIZE] = { '\0' };
Options mmb_options;
PromptState prompt_state;
MmResult path_complete_canned_result;
char path_complete_captured_path[STRINGSIZE];
char config_dir[PATH_MAX] = { '\0' };

// Defined in "core/MMBasic.c"
void perform_background_tasks() {}

MmResult file_test_get_config_dir(char *buf, size_t size) {
    if (FAILED(cstring_cpy(buf, config_dir, size))) {
        return kFilenameTooLong;
    }
    return kOk;
}

MmResult path_test_complete(const char *path, char *out, size_t sz) {
    strcpy(path_complete_captured_path, path);
    if (*path) {
        strcpy(out, "-completed");
    } else {
        strcpy(out, "");
    }
    return path_complete_canned_result;
}

} // extern "C"

class PromptTestBase : public ::testing::Test {
protected:
    std::string test_dir;

    void SetUp() override {
        file_get_config_dir = file_test_get_config_dir;
        path_complete = path_test_complete;

        test_dir = ::testing::TempDir() + "PromptTest";

        std::filesystem::create_directories(test_dir);

        // Create config dir
        ASSERT_EQ(0, cstring_cpy(config_dir, test_dir.c_str(), sizeof(config_dir)));
        ASSERT_EQ(kOk, file_append_path(config_dir, ".mmbasic", sizeof(config_dir)));
        std::filesystem::create_directories(config_dir);
    }

    void TearDown() override {
        std::filesystem::remove_all(test_dir);
    }
};

////////////////////////////////////////////////////////////////////////////////
// Tests for prompt_handle_tab()
////////////////////////////////////////////////////////////////////////////////

class PromptHandleTabTest : public PromptTestBase {};

#define TEST_HANDLE_TAB(input, expected_path, expected_buf, expected_bell) \
    strcpy(inpbuf, input); \
    strcpy(path_complete_captured_path, ""); \
    strcpy(prompt_state.buf, "\tABCDEF"); \
    display_bell_sounded = false; \
    EXPECT_EQ(kOk, prompt_handle_tab(&prompt_state)); \
    EXPECT_STREQ(input, inpbuf); \
    EXPECT_STREQ(expected_path, path_complete_captured_path); \
    EXPECT_STREQ(expected_buf, prompt_state.buf); \
    EXPECT_EQ(expected_bell, display_bell_sounded);

TEST_F(PromptHandleTabTest, HandleTab_GivenSuccess) {
    path_complete_canned_result = kOk;

    // Path is the first thing in the inpbuf.
    TEST_HANDLE_TAB("foo.b", "foo.b", "\t-completed", false);

    // Using the dynamic test_dir path
    std::string full_path = test_dir + "/foo.b";
    TEST_HANDLE_TAB(full_path.c_str(), full_path.c_str(), "\t-completed", false);

    // Path is the second and last thing in the inpbuf.
    TEST_HANDLE_TAB("!ls foo.b", "foo.b", "\t-completed", false);

    std::string cmd_with_path = "!ls " + test_dir + "/foo.b";
    TEST_HANDLE_TAB(cmd_with_path.c_str(), full_path.c_str(), "\t-completed", false);

    // Path is quoted.
    TEST_HANDLE_TAB("!ls \"foo.b", "foo.b", "\t-completed", false);
    TEST_HANDLE_TAB("!ls \"", "", "\t", true);
    TEST_HANDLE_TAB("!ls \"has spaces", "has spaces", "\t-completed", false);
    TEST_HANDLE_TAB("!ls \"foo.b\"", "", "\t", true);
}

TEST_F(PromptHandleTabTest, HandleTab_GivenFailure) {
    path_complete_canned_result = kError;
    TEST_HANDLE_TAB("foo.b", "foo.b", "\t", true);
}

////////////////////////////////////////////////////////////////////////////////
// Tests for prompt_restore_history()
////////////////////////////////////////////////////////////////////////////////

class PromptRestoreHistoryTest : public PromptTestBase {};

// Test restoring history from default location
TEST_F(PromptRestoreHistoryTest, RestoreFromDefaultLocation) {
    // Create default history file
    std::string history_file = std::string(config_dir) + "/mmbasic.history";

    FILE* f = fopen(history_file.c_str(), "w");
    ASSERT_NE(nullptr, f);
    fprintf(f, "PRINT \"Hello\"\n");
    fprintf(f, "LIST\n");
    fprintf(f, "RUN\n");
    fclose(f);

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history(NULL);

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("RUN", prompt_get_history_item(0));
    EXPECT_STREQ("LIST", prompt_get_history_item(1));
    EXPECT_STREQ("PRINT \"Hello\"", prompt_get_history_item(2));
}

// Test restoring history from explicit file path
TEST_F(PromptRestoreHistoryTest, RestoreFromExplicitPath) {
    std::string history_file = test_dir + "/my_history.txt";

    FILE* f = fopen(history_file.c_str(), "w");
    ASSERT_NE(nullptr, f);
    fprintf(f, "Line1\n");
    fprintf(f, "Line2\n");
    fprintf(f, "Line3\n");
    fclose(f);

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history(history_file.c_str());

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("Line3", prompt_get_history_item(0));
    EXPECT_STREQ("Line2", prompt_get_history_item(1));
    EXPECT_STREQ("Line1", prompt_get_history_item(2));
}

// Test restoring empty history file
TEST_F(PromptRestoreHistoryTest, RestoreEmptyFile) {
    std::string history_file = test_dir + "/empty_history.txt";

    FILE* f = fopen(history_file.c_str(), "w");
    ASSERT_NE(nullptr, f);
    fclose(f);

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history(history_file.c_str());

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, prompt_get_history_count());
}

// Test restoring history with single item
TEST_F(PromptRestoreHistoryTest, RestoreSingleItem) {
    std::string history_file = test_dir + "/single_history.txt";

    FILE* f = fopen(history_file.c_str(), "w");
    ASSERT_NE(nullptr, f);
    fprintf(f, "SINGLE LINE\n");
    fclose(f);

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history(history_file.c_str());

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, prompt_get_history_count());
    EXPECT_STREQ("SINGLE LINE", prompt_get_history_item(0));
}

// Test restoring history with empty lines
TEST_F(PromptRestoreHistoryTest, RestoreWithEmptyLines) {
    std::string history_file = test_dir + "/empty_lines_history.txt";

    FILE* f = fopen(history_file.c_str(), "w");
    ASSERT_NE(nullptr, f);
    fprintf(f, "Line1\n");
    fprintf(f, "\n");
    fprintf(f, "Line3\n");
    fprintf(f, "\n");
    fclose(f);

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history(history_file.c_str());

    EXPECT_EQ(kOk, result);
    // Empty lines should have been skipped
    EXPECT_EQ(2, prompt_get_history_count());
    EXPECT_STREQ("Line3", prompt_get_history_item(0));
    EXPECT_STREQ("Line1", prompt_get_history_item(1));
}

// Test restoring history with special characters
TEST_F(PromptRestoreHistoryTest, RestoreWithSpecialCharacters) {
    std::string history_file = test_dir + "/special_history.txt";

    FILE* f = fopen(history_file.c_str(), "w");
    ASSERT_NE(nullptr, f);
    fprintf(f, "PRINT \"Hello World\"\n");
    fprintf(f, "x = 5 * 3\n");
    fprintf(f, "!ls /tmp/*.txt\n");
    fclose(f);

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history(history_file.c_str());

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("!ls /tmp/*.txt", prompt_get_history_item(0));
    EXPECT_STREQ("x = 5 * 3", prompt_get_history_item(1));
    EXPECT_STREQ("PRINT \"Hello World\"", prompt_get_history_item(2));
}

// Test restoring history with very long lines
TEST_F(PromptRestoreHistoryTest, RestoreWithLongLines) {
    std::string history_file = test_dir + "/long_history.txt";
    std::string long_line(STRINGSIZE - 10, 'A');

    FILE* f = fopen(history_file.c_str(), "w");
    ASSERT_NE(nullptr, f);
    fprintf(f, "%s\n", long_line.c_str());
    fprintf(f, "Short\n");
    fclose(f);

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history(history_file.c_str());

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("Short", prompt_get_history_item(0));
    EXPECT_STREQ(long_line.c_str(), prompt_get_history_item(1));
}

// Test restoring history from non-existent file
TEST_F(PromptRestoreHistoryTest, RestoreFromNonExistentFile) {
    std::string history_file = test_dir + "/does_not_exist.txt";

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history(history_file.c_str());

    EXPECT_NE(kOk, result);
    EXPECT_EQ(0, prompt_get_history_count());
}

// Test restoring history with relative path
TEST_F(PromptRestoreHistoryTest, RestoreWithRelativePath) {
    std::string history_file = test_dir + "/relative_history.txt";

    FILE* f = fopen(history_file.c_str(), "w");
    ASSERT_NE(nullptr, f);
    fprintf(f, "RELATIVE PATH TEST\n");
    fclose(f);

    // Change to test directory and use relative path
    char old_cwd[PATH_MAX];
    ASSERT_EQ(kOk, file_getcwd(old_cwd, sizeof(old_cwd)));
    ASSERT_EQ(kOk, file_chdir(test_dir.c_str()));

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history("./relative_history.txt");

    ASSERT_EQ(kOk, file_chdir(old_cwd));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("RELATIVE PATH TEST", prompt_get_history_item(0));
}

// Test restoring history with mixed line endings
TEST_F(PromptRestoreHistoryTest, RestoreWithMixedLineEndings) {
    std::string history_file = test_dir + "/mixed_endings_history.txt";

    FILE* f = fopen(history_file.c_str(), "wb");
    ASSERT_NE(nullptr, f);
    fprintf(f, "Unix\n");
    fprintf(f, "Windows\r\n");
    fprintf(f, "Mac\r");
    fclose(f);

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history(history_file.c_str());

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(3, prompt_get_history_count());
    EXPECT_STREQ("Mac", prompt_get_history_item(0));
    EXPECT_STREQ("Windows", prompt_get_history_item(1));
    EXPECT_STREQ("Unix", prompt_get_history_item(2));
}

// Test restoring history doesn't overflow buffer
TEST_F(PromptRestoreHistoryTest, RestoreDoesNotOverflowBuffer) {
    std::string history_file = test_dir + "/huge_history.txt";

    FILE* f = fopen(history_file.c_str(), "w");
    ASSERT_NE(nullptr, f);
    // Write more lines than can fit in HISTORY_SIZE
    for (int i = 0; i < 1000; i++) {
        fprintf(f, "Line%d\n", i);
    }
    fclose(f);

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history(history_file.c_str());

    EXPECT_EQ(kOk, result);
    // Should have loaded lines but truncated to fit buffer
    int count = prompt_get_history_count();
    EXPECT_GT(count, 0);
    EXPECT_LT(count, 1000);
}

// Test restoring history with trailing content after last newline
TEST_F(PromptRestoreHistoryTest, RestoreWithTrailingContent) {
    std::string history_file = test_dir + "/trailing_history.txt";

    FILE* f = fopen(history_file.c_str(), "w");
    ASSERT_NE(nullptr, f);
    fprintf(f, "Line1\n");
    fprintf(f, "Line2\n");
    fprintf(f, "NoNewline");
    fclose(f);

    memset(prompt_history, 0, sizeof(prompt_history));
    MmResult result = prompt_restore_history(history_file.c_str());

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(3, prompt_get_history_count());
    EXPECT_STREQ("NoNewline", prompt_get_history_item(0));
    EXPECT_STREQ("Line2", prompt_get_history_item(1));
    EXPECT_STREQ("Line1", prompt_get_history_item(2));
}

////////////////////////////////////////////////////////////////////////////////
// Tests for prompt_save_history()
////////////////////////////////////////////////////////////////////////////////

class PromptSaveHistoryTest : public PromptTestBase {};

// Test saving history to default location
TEST_F(PromptSaveHistoryTest, SaveToDefaultLocation) {
    memset(prompt_history, 0, sizeof(prompt_history));
    prompt_put_history_item("Line1");
    prompt_put_history_item("Line2");
    prompt_put_history_item("Line3");

    MmResult result = prompt_save_history(NULL);

    EXPECT_EQ(kOk, result);

    // Verify file contents
    std::string history_file = std::string(config_dir) + "/mmbasic.history";
    FILE* f = fopen(history_file.c_str(), "r");
    ASSERT_NE(nullptr, f);

    char line[256];
    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("Line1", line);

    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("Line2", line);

    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("Line3", line);

    fclose(f);
}

// Test saving history to explicit file path
TEST_F(PromptSaveHistoryTest, SaveToExplicitPath) {
    std::string history_file = test_dir + "/my_history.txt";

    memset(prompt_history, 0, sizeof(prompt_history));
    prompt_put_history_item("Command1");
    prompt_put_history_item("Command2");

    MmResult result = prompt_save_history(history_file.c_str());

    EXPECT_EQ(kOk, result);

    // Verify file contents
    FILE* f = fopen(history_file.c_str(), "r");
    ASSERT_NE(nullptr, f);

    char line[256];
    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("Command1", line);

    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("Command2", line);

    fclose(f);
}

// Test saving empty history
TEST_F(PromptSaveHistoryTest, SaveEmptyHistory) {
    std::string history_file = test_dir + "/empty_history.txt";

    memset(prompt_history, 0, sizeof(prompt_history));

    MmResult result = prompt_save_history(history_file.c_str());

    EXPECT_EQ(kOk, result);

    // Verify file is empty
    FILE* f = fopen(history_file.c_str(), "r");
    ASSERT_NE(nullptr, f);

    char line[256];
    EXPECT_EQ(nullptr, fgets(line, sizeof(line), f));

    fclose(f);
}

// Test saving single history item
TEST_F(PromptSaveHistoryTest, SaveSingleItem) {
    std::string history_file = test_dir + "/single_history.txt";

    memset(prompt_history, 0, sizeof(prompt_history));
    prompt_put_history_item("ONLY ONE");

    MmResult result = prompt_save_history(history_file.c_str());

    EXPECT_EQ(kOk, result);

    // Verify file contents
    FILE* f = fopen(history_file.c_str(), "r");
    ASSERT_NE(nullptr, f);

    char line[256];
    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("ONLY ONE", line);

    EXPECT_EQ(nullptr, fgets(line, sizeof(line), f));

    fclose(f);
}

// Test saving history with special characters
TEST_F(PromptSaveHistoryTest, SaveWithSpecialCharacters) {
    std::string history_file = test_dir + "/special_history.txt";

    memset(prompt_history, 0, sizeof(prompt_history));
    prompt_put_history_item("PRINT \"Quotes\"");
    prompt_put_history_item("x = 5 * 3");
    prompt_put_history_item("!ls /tmp/*.txt");

    MmResult result = prompt_save_history(history_file.c_str());

    EXPECT_EQ(kOk, result);

    // Verify file contents
    FILE* f = fopen(history_file.c_str(), "r");
    ASSERT_NE(nullptr, f);

    char line[256];
    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("PRINT \"Quotes\"", line);

    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("x = 5 * 3", line);

    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("!ls /tmp/*.txt", line);

    fclose(f);
}

// Test saving history with very long lines
TEST_F(PromptSaveHistoryTest, SaveWithLongLines) {
    std::string history_file = test_dir + "/long_history.txt";
    std::string long_line(STRINGSIZE - 10, 'A');

    memset(prompt_history, 0, sizeof(prompt_history));
    prompt_put_history_item(const_cast<char*>(long_line.c_str()));
    prompt_put_history_item("Short");

    MmResult result = prompt_save_history(history_file.c_str());

    EXPECT_EQ(kOk, result);

    // Verify file contents
    FILE* f = fopen(history_file.c_str(), "r");
    ASSERT_NE(nullptr, f);

    char line[STRINGSIZE];
    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ(long_line.c_str(), line);

    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("Short", line);

    fclose(f);
}

// Test saving history overwrites existing file
TEST_F(PromptSaveHistoryTest, SaveOverwritesExistingFile) {
    std::string history_file = test_dir + "/overwrite_history.txt";

    // Create file with initial content
    FILE* f = fopen(history_file.c_str(), "w");
    ASSERT_NE(nullptr, f);
    fprintf(f, "OLD CONTENT\n");
    fclose(f);

    memset(prompt_history, 0, sizeof(prompt_history));
    prompt_put_history_item("NEW CONTENT");

    MmResult result = prompt_save_history(history_file.c_str());

    EXPECT_EQ(kOk, result);

    // Verify file has new content only
    f = fopen(history_file.c_str(), "r");
    ASSERT_NE(nullptr, f);

    char line[256];
    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("NEW CONTENT", line);

    EXPECT_EQ(nullptr, fgets(line, sizeof(line), f));

    fclose(f);
}

// Test saving history with relative path
TEST_F(PromptSaveHistoryTest, SaveWithRelativePath) {
    // Change to test directory and use relative path
    char old_cwd[PATH_MAX];
    ASSERT_EQ(kOk, file_getcwd(old_cwd, sizeof(old_cwd)));
    ASSERT_EQ(kOk, file_chdir(test_dir.c_str()));

    memset(prompt_history, 0, sizeof(prompt_history));
    prompt_put_history_item("RELATIVE TEST");

    MmResult result = prompt_save_history("./relative_save.txt");

    ASSERT_EQ(kOk, file_chdir(old_cwd));

    EXPECT_EQ(kOk, result);

    // Verify file was created
    std::string history_file = test_dir + "/relative_save.txt";
    FILE* f = fopen(history_file.c_str(), "r");
    ASSERT_NE(nullptr, f);

    char line[256];
    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("RELATIVE TEST", line);

    fclose(f);
}

// Test round-trip: save then restore
TEST_F(PromptSaveHistoryTest, RoundTripSaveAndRestore) {
    std::string history_file = test_dir + "/roundtrip_history.txt";

    memset(prompt_history, 0, sizeof(prompt_history));
    prompt_put_history_item("First");
    prompt_put_history_item("Second");
    prompt_put_history_item("Third");

    MmResult result = prompt_save_history(history_file.c_str());
    EXPECT_EQ(kOk, result);

    // Clear history and restore
    memset(prompt_history, 0, sizeof(prompt_history));
    result = prompt_restore_history(history_file.c_str());
    EXPECT_EQ(kOk, result);

    // Verify restored history matches
    EXPECT_EQ(3, prompt_get_history_count());
    EXPECT_STREQ("Third", prompt_get_history_item(0));
    EXPECT_STREQ("Second", prompt_get_history_item(1));
    EXPECT_STREQ("First", prompt_get_history_item(2));
}

// Test saving to non-existent directory fails
TEST_F(PromptSaveHistoryTest, SaveToNonExistentDirectory) {
    std::string history_file = test_dir + "/nonexistent/history.txt";

    memset(prompt_history, 0, sizeof(prompt_history));
    prompt_put_history_item("Test");

    MmResult result = prompt_save_history(history_file.c_str());

    EXPECT_EQ(kFileNotFound, result);
}

// Test saving preserves order (oldest first in file)
TEST_F(PromptSaveHistoryTest, SavePreservesOrder) {
    std::string history_file = test_dir + "/order_history.txt";

    memset(prompt_history, 0, sizeof(prompt_history));
    // Add in chronological order (oldest to newest)
    prompt_put_history_item("Third");   // Will be most recent
    prompt_put_history_item("Second");
    prompt_put_history_item("First");   // Will be oldest

    MmResult result = prompt_save_history(history_file.c_str());

    EXPECT_EQ(kOk, result);

    // File should have oldest first
    FILE* f = fopen(history_file.c_str(), "r");
    ASSERT_NE(nullptr, f);

    char line[256];

    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("Third", line);

    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("Second", line);

    EXPECT_NE(nullptr, fgets(line, sizeof(line), f));
    line[strcspn(line, "\n")] = '\0';
    EXPECT_STREQ("First", line);

    fclose(f);
}
