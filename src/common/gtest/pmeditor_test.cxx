/*
 * Copyright (c) 2025-2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdlib>

extern "C" {

#include "../options.h"
#include "../pmeditor.h"
#include "../pmeditor_private.h"
#include "../../core/gtest/command_stubs.h"
#include "../../core/gtest/function_stubs.h"
#include "../../core/gtest/operation_stubs.h"

// Defined in "common/file.c"
bool file_exists_regular(const char *filename) { return false; }

// Defined in "common/options.c"
Options mmb_options;

// Defined in "common/prompt.c"
MmResult prompt_getc(int *ch) {
    *ch = -1;
    return kOk;
}

// Defined in "core/MMBasic.c"
const char *nextstmt = NULL;
char inpbuf[INPBUF_SIZE] = { 0 };
char tknbuf[TKNBUF_SIZE] = { 0 };
int LocalIndex = 0;
MMINTEGER getinteger(const char *p) { return 0; }
MmResult PrepareProgram(bool abort_on_error) { return kOk; }
MmResult ClearRuntime(void) { return kOk; }

}

class PmEditorFindLineTest : public ::testing::Test {

protected:
    PmEditor test_editor;
    PmEditor *self = &test_editor;

    void SetUp() override {
        // Clear the buffer before each test
        memset(self->buf, 0, EDIT_BUFFER_SIZE);
    }

    void SetBuffer(const char* content) {
        strncpy(self->buf, content, EDIT_BUFFER_SIZE - 1);
        self->buf[EDIT_BUFFER_SIZE - 1] = '\0';
    }
};

// Test finding line 0 (first line) in empty buffer
TEST_F(PmEditorFindLineTest, FindLine0EmptyBuffer) {
    SetBuffer("");
    int inmulti = -1;  // Initialize to invalid value to ensure it's set

    char *result = pmeditor_find_line(self, 0, &inmulti);

    EXPECT_EQ(result, self->buf);
    EXPECT_FALSE(inmulti);
}

// Test finding line 0 in single line buffer
TEST_F(PmEditorFindLineTest, FindLine0SingleLine) {
    SetBuffer("Hello World");
    int inmulti = -1;

    char *result = pmeditor_find_line(self, 0, &inmulti);

    EXPECT_EQ(result, self->buf);
    EXPECT_FALSE(inmulti);
}

// Test finding line 1 in single line buffer (should return end)
TEST_F(PmEditorFindLineTest, FindLine1SingleLine) {
    SetBuffer("Hello World");
    int inmulti = -1;

    char *result = pmeditor_find_line(self, 1, &inmulti);

    EXPECT_EQ(result, self->buf + strlen("Hello World"));
    EXPECT_FALSE(inmulti);
}

// Test finding various lines in multi-line buffer
TEST_F(PmEditorFindLineTest, FindLinesMultiLine) {
    SetBuffer("Line 0\nLine 1\nLine 2\nLine 3");
    int inmulti;

    // Test line 0
    char *result0 = pmeditor_find_line(self, 0, &inmulti);
    EXPECT_EQ(result0, self->buf);
    EXPECT_FALSE(inmulti);

    // Test line 1
    char *result1 = pmeditor_find_line(self, 1, &inmulti);
    EXPECT_EQ(result1, self->buf + strlen("Line 0\n"));
    EXPECT_FALSE(inmulti);

    // Test line 2
    char *result2 = pmeditor_find_line(self, 2, &inmulti);
    EXPECT_EQ(result2, self->buf + strlen("Line 0\nLine 1\n"));
    EXPECT_FALSE(inmulti);

    // Test line 3
    char *result3 = pmeditor_find_line(self, 3, &inmulti);
    EXPECT_EQ(result3, self->buf + strlen("Line 0\nLine 1\nLine 2\n"));
    EXPECT_FALSE(inmulti);
}

// Test finding line beyond end of buffer
TEST_F(PmEditorFindLineTest, FindLineBeyondEnd) {
    SetBuffer("Line 0\nLine 1");
    int inmulti = -1;

    char *result = pmeditor_find_line(self, 5, &inmulti);

    // Should return end of buffer
    EXPECT_EQ(result, self->buf + strlen("Line 0\nLine 1"));
    EXPECT_FALSE(inmulti);
}

// Test multiline comment detection at start of file
TEST_F(PmEditorFindLineTest, MultilineCommentAtStart) {
    SetBuffer("/* comment */\nLine 1\nLine 2");
    int inmulti = -1;

    char *result = pmeditor_find_line(self, 0, &inmulti);

    EXPECT_EQ(result, self->buf);
    EXPECT_TRUE(inmulti);
}

// Test multiline comment detection with leading spaces
TEST_F(PmEditorFindLineTest, MultilineCommentWithSpaces) {
    SetBuffer("   /* comment */\nLine 1\nLine 2");
    int inmulti = -1;

    char *result = pmeditor_find_line(self, 0, &inmulti);

    EXPECT_EQ(result, self->buf);
    EXPECT_TRUE(inmulti);
}

// Test multiline comment detection with tabs
TEST_F(PmEditorFindLineTest, MultilineCommentWithTabs) {
    SetBuffer("\t\t/* comment */\nLine 1\nLine 2");
    int inmulti = -1;

    char *result = pmeditor_find_line(self, 0, &inmulti);

    EXPECT_EQ(result, self->buf);
    EXPECT_FALSE(inmulti);
}

// Test multiline comment starting on second line
TEST_F(PmEditorFindLineTest, MultilineCommentOnSecondLine) {
    SetBuffer("Line 0\n/* comment\nLine 2");
    int inmulti = -1;

    // Line 0 should not be in multiline comment
    char *result0 = pmeditor_find_line(self, 0, &inmulti);
    EXPECT_EQ(result0, self->buf);
    EXPECT_FALSE(inmulti);

    // Line 1 should be in multiline comment
    char *result1 = pmeditor_find_line(self, 1, &inmulti);
    EXPECT_EQ(result1, self->buf + strlen("Line 0\n"));
    EXPECT_TRUE(inmulti);
}

// Test multiline comment end detection
TEST_F(PmEditorFindLineTest, MultilineCommentEnd) {
    SetBuffer("Line 0\n*/\nLine 2");
    int inmulti = -1;

    // Line 1 should have inmulti = 2 (comment ending)
    char *result1 = pmeditor_find_line(self, 1, &inmulti);
    EXPECT_EQ(result1, self->buf + strlen("Line 0\n"));
    EXPECT_EQ(inmulti, 2);

    // Line 2 should not be in comment (inmulti should be false after line with */)
    char *result2 = pmeditor_find_line(self, 2, &inmulti);
    EXPECT_EQ(result2, self->buf + strlen("Line 0\n*/\n"));
    EXPECT_FALSE(inmulti);
}

// Test multiline comment end with spaces
TEST_F(PmEditorFindLineTest, MultilineCommentEndWithSpaces) {
    SetBuffer("Line 0\n  */\nLine 2");
    int inmulti = -1;

    char *result1 = pmeditor_find_line(self, 1, &inmulti);
    EXPECT_EQ(result1, self->buf + strlen("Line 0\n"));
    EXPECT_EQ(inmulti, 2);
}

// Test complex multiline comment scenario
TEST_F(PmEditorFindLineTest, ComplexMultilineComment) {
    SetBuffer("/* start comment\nstill in comment\n*/\nLine 3\n/* new comment\nLine 5");
    int inmulti;

    // Line 0: starts with comment
    pmeditor_find_line(self, 0, &inmulti);
    EXPECT_TRUE(inmulti);

    // Line 1: still in comment
    pmeditor_find_line(self, 1, &inmulti);
    EXPECT_TRUE(inmulti);

    // Line 2: ends comment
    pmeditor_find_line(self, 2, &inmulti);
    EXPECT_EQ(inmulti, 2);

    // Line 3: not in comment
    pmeditor_find_line(self, 3, &inmulti);
    EXPECT_FALSE(inmulti);

    // Line 4: starts new comment
    pmeditor_find_line(self, 4, &inmulti);
    EXPECT_TRUE(inmulti);

    // Line 5: still in comment
    pmeditor_find_line(self, 5, &inmulti);
    EXPECT_TRUE(inmulti);
}

// Test edge case: buffer with only newlines
TEST_F(PmEditorFindLineTest, OnlyNewlines) {
    SetBuffer("\n\n\n");
    int inmulti;

    char *result0 = pmeditor_find_line(self, 0, &inmulti);
    EXPECT_EQ(result0, self->buf);
    EXPECT_FALSE(inmulti);

    char *result1 = pmeditor_find_line(self, 1, &inmulti);
    EXPECT_EQ(result1, self->buf + 1);
    EXPECT_FALSE(inmulti);

    char *result2 = pmeditor_find_line(self, 2, &inmulti);
    EXPECT_EQ(result2, self->buf + 2);
    EXPECT_FALSE(inmulti);

    char *result3 = pmeditor_find_line(self, 3, &inmulti);
    EXPECT_EQ(result3, self->buf + 3);
    EXPECT_FALSE(inmulti);
}

// Test edge case: incomplete multiline comment markers
TEST_F(PmEditorFindLineTest, IncompleteCommentMarkers) {
    SetBuffer("/\n*\nLine 2");
    int inmulti = -1;

    // Should not detect as multiline comment
    char *result = pmeditor_find_line(self, 0, &inmulti);
    EXPECT_EQ(result, self->buf);
    EXPECT_FALSE(inmulti);
}

// Test edge case: comment markers not at start of content
TEST_F(PmEditorFindLineTest, CommentMarkersNotAtStart) {
    SetBuffer("code /* comment\nLine 1");
    int inmulti = -1;

    // Should not detect as multiline comment since /* is not at start of line content
    char *result = pmeditor_find_line(self, 0, &inmulti);
    EXPECT_EQ(result, self->buf);
    EXPECT_FALSE(inmulti);
}

// Test performance with large line numbers
TEST_F(PmEditorFindLineTest, LargeLineNumber) {
    // Create a buffer with many lines
    std::string content;
    for (int i = 0; i < 100; ++i) {
        content += "Line " + std::to_string(i) + "\n";
    }
    SetBuffer(content.c_str());

    int inmulti = -1;
    char *result = pmeditor_find_line(self, 50, &inmulti);

    // Calculate expected position
    size_t expected_pos = 0;
    for (int i = 0; i < 50; ++i) {
        std::string line = "Line " + std::to_string(i) + "\n";
        expected_pos += line.length();
    }

    EXPECT_EQ(result, self->buf + expected_pos);
    EXPECT_FALSE(inmulti);
}

// Test null pointer safety for inmulti parameter
TEST_F(PmEditorFindLineTest, NullInmultiParameter) {
    SetBuffer("Line 0\nLine 1");

    // This should not crash, though the original function doesn't handle null inmulti
    // In a real implementation, you might want to add null checks
    char *result = pmeditor_find_line(self, 1, nullptr);
    EXPECT_EQ(result, nullptr);
}
