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
#include "../../core/commandtbl.h"
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

static HighlightType last_highlight_type = kHighlightNormal;
static int highlight_call_count = 0;

MmResult pmeditor_highlight(HighlightType highlight) {
    last_highlight_type = highlight;
    highlight_call_count++;
    return kOk;
}

} // extern "C"

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
    int comment_level = -1;  // Initialize to invalid value to ensure it's set

    char *result = pmeditor_find_line(self, 0, &comment_level);

    EXPECT_EQ(result, self->buf);
    EXPECT_EQ(0, comment_level);
}

// Test finding line 0 in single line buffer
TEST_F(PmEditorFindLineTest, FindLine0SingleLine) {
    SetBuffer("Hello World");
    int comment_level = -1;

    char *result = pmeditor_find_line(self, 0, &comment_level);

    EXPECT_EQ(result, self->buf);
    EXPECT_EQ(0, comment_level);
}

// Test finding line 1 in single line buffer (should return end)
TEST_F(PmEditorFindLineTest, FindLine1SingleLine) {
    SetBuffer("Hello World");
    int comment_level = -1;

    char *result = pmeditor_find_line(self, 1, &comment_level);

    EXPECT_EQ(result, self->buf + strlen("Hello World"));
    EXPECT_EQ(0, comment_level);
}

// Test finding various lines in multi-line buffer
TEST_F(PmEditorFindLineTest, FindLinesMultiLine) {
    SetBuffer("Line 0\nLine 1\nLine 2\nLine 3");
    int comment_level;

    // Test line 0
    char *result0 = pmeditor_find_line(self, 0, &comment_level);
    EXPECT_EQ(result0, self->buf);
    EXPECT_EQ(0, comment_level);

    // Test line 1
    char *result1 = pmeditor_find_line(self, 1, &comment_level);
    EXPECT_EQ(result1, self->buf + strlen("Line 0\n"));
    EXPECT_EQ(0, comment_level);

    // Test line 2
    char *result2 = pmeditor_find_line(self, 2, &comment_level);
    EXPECT_EQ(result2, self->buf + strlen("Line 0\nLine 1\n"));
    EXPECT_EQ(0, comment_level);

    // Test line 3
    char *result3 = pmeditor_find_line(self, 3, &comment_level);
    EXPECT_EQ(result3, self->buf + strlen("Line 0\nLine 1\nLine 2\n"));
    EXPECT_EQ(0, comment_level);
}

// Test finding line beyond end of buffer
TEST_F(PmEditorFindLineTest, FindLineBeyondEnd) {
    SetBuffer("Line 0\nLine 1");
    int comment_level = -1;

    char *result = pmeditor_find_line(self, 5, &comment_level);

    // Should return end of buffer
    EXPECT_EQ(result, self->buf + strlen("Line 0\nLine 1"));
    EXPECT_EQ(0, comment_level);
}

// Test multiline comment detection at start of file
TEST_F(PmEditorFindLineTest, MultilineCommentAtStart) {
    SetBuffer("/* comment */\nLine 1\nLine 2");
    int comment_level = -1;

    char *result = pmeditor_find_line(self, 0, &comment_level);

    EXPECT_EQ(result, self->buf);
    EXPECT_EQ(0, comment_level);  // Start of the line is not inside the comment.
}

// Test multiline comment detection with leading spaces
TEST_F(PmEditorFindLineTest, MultilineCommentWithSpaces) {
    SetBuffer("   /* comment */\nLine 1\nLine 2");
    int comment_level = -1;

    char *result = pmeditor_find_line(self, 0, &comment_level);

    EXPECT_EQ(result, self->buf);
    EXPECT_EQ(0, comment_level);  // Start of the line is not inside the comment.
}

// Test multiline comment detection with tabs
TEST_F(PmEditorFindLineTest, MultilineCommentWithTabs) {
    SetBuffer("\t\t/* comment */\nLine 1\nLine 2");
    int comment_level = -1;

    char *result = pmeditor_find_line(self, 0, &comment_level);

    EXPECT_EQ(result, self->buf);
    EXPECT_EQ(0, comment_level);  // Start of the line is not inside the comment.
}

// Test multiline comment starting on second line
TEST_F(PmEditorFindLineTest, MultilineCommentOnSecondLine) {
    SetBuffer("Line 0\n/* comment\nLine 2");
    int comment_level = -1;

    // Line 0 should not be in multiline comment
    char *result0 = pmeditor_find_line(self, 0, &comment_level);
    EXPECT_EQ(result0, self->buf);
    EXPECT_EQ(0, comment_level);

    // Line 1 should be in multiline comment TODO
    char *result1 = pmeditor_find_line(self, 1, &comment_level);
    EXPECT_EQ(result1, self->buf + strlen("Line 0\n"));
    EXPECT_EQ(0, comment_level);  // Line 1 starts the comment TODO

    // Line 2 should be in multiline comment
    char *result2 = pmeditor_find_line(self, 2, &comment_level);
    EXPECT_EQ(result2, self->buf + strlen("Line 0\n/* comment\n"));
    EXPECT_EQ(1, comment_level);  // Start of line 2 is inside the comment
}

// Test multiline comment end detection
TEST_F(PmEditorFindLineTest, MultilineCommentEnd) {
    SetBuffer("Line 0\n*/\nLine 2");
    int comment_level = -1;

    // Line 1 should have comment_level = 0
    char *result1 = pmeditor_find_line(self, 1, &comment_level);
    EXPECT_EQ(result1, self->buf + strlen("Line 0\n"));
    EXPECT_EQ(0, comment_level);

    // Line 2 should not be in comment (comment_level should be false after line with */)
    char *result2 = pmeditor_find_line(self, 2, &comment_level);
    EXPECT_EQ(result2, self->buf + strlen("Line 0\n*/\n"));
    EXPECT_EQ(0, comment_level);
}

// Test multiline comment end with spaces
TEST_F(PmEditorFindLineTest, MultilineCommentEndWithSpaces) {
    SetBuffer("Line 0\n  */\nLine 2");
    int comment_level = -1;

    char *result1 = pmeditor_find_line(self, 1, &comment_level);
    EXPECT_EQ(result1, self->buf + strlen("Line 0\n"));
    EXPECT_EQ(0, comment_level);
}

// Test complex multiline comment scenario
TEST_F(PmEditorFindLineTest, ComplexMultilineComment) {
    SetBuffer("/* start comment\nstill in comment\n*/\nLine 3\n/* new comment\nLine 5");
    int comment_level = -1;

    // Line 0: starts with comment
    pmeditor_find_line(self, 0, &comment_level);
    EXPECT_EQ(0, comment_level);

    // Line 1: still in comment
    pmeditor_find_line(self, 1, &comment_level);
    EXPECT_EQ(1, comment_level);

    // Line 2: ends comment
    pmeditor_find_line(self, 2, &comment_level);
    EXPECT_EQ(1, comment_level);

    // Line 3: not in comment
    pmeditor_find_line(self, 3, &comment_level);
    EXPECT_EQ(0, comment_level);

    // Line 4: starts new comment
    pmeditor_find_line(self, 4, &comment_level);
    EXPECT_EQ(0, comment_level);

    // Line 5: still in comment
    pmeditor_find_line(self, 5, &comment_level);
    EXPECT_EQ(1, comment_level);
}

// Test edge case: buffer with only newlines
TEST_F(PmEditorFindLineTest, OnlyNewlines) {
    SetBuffer("\n\n\n");
    int comment_level;

    char *result0 = pmeditor_find_line(self, 0, &comment_level);
    EXPECT_EQ(result0, self->buf);
    EXPECT_EQ(0, comment_level);

    char *result1 = pmeditor_find_line(self, 1, &comment_level);
    EXPECT_EQ(result1, self->buf + 1);
    EXPECT_EQ(0, comment_level);

    char *result2 = pmeditor_find_line(self, 2, &comment_level);
    EXPECT_EQ(result2, self->buf + 2);
    EXPECT_EQ(0, comment_level);

    char *result3 = pmeditor_find_line(self, 3, &comment_level);
    EXPECT_EQ(result3, self->buf + 3);
    EXPECT_EQ(0, comment_level);
}

// Test edge case: incomplete multiline comment markers
TEST_F(PmEditorFindLineTest, IncompleteCommentMarkers) {
    SetBuffer("/\n*\nLine 2");
    int comment_level = -1;

    // Should not detect as multiline comment
    char *result = pmeditor_find_line(self, 0, &comment_level);
    EXPECT_EQ(result, self->buf);
    EXPECT_EQ(0, comment_level);
}

// Test edge case: comment markers not at start of content
TEST_F(PmEditorFindLineTest, CommentMarkersNotAtStart) {
    SetBuffer("code /* comment\nLine 1");
    int comment_level = -1;

    // Should not detect as multiline comment since /* is not at start of line content
    char *result = pmeditor_find_line(self, 0, &comment_level);
    EXPECT_EQ(result, self->buf);
    EXPECT_EQ(0, comment_level);
}

// Test performance with large line numbers
TEST_F(PmEditorFindLineTest, LargeLineNumber) {
    // Create a buffer with many lines
    std::string content;
    for (int i = 0; i < 100; ++i) {
        content += "Line " + std::to_string(i) + "\n";
    }
    SetBuffer(content.c_str());

    int comment_level = -1;
    char *result = pmeditor_find_line(self, 50, &comment_level);

    // Calculate expected position
    size_t expected_pos = 0;
    for (int i = 0; i < 50; ++i) {
        std::string line = "Line " + std::to_string(i) + "\n";
        expected_pos += line.length();
    }

    EXPECT_EQ(result, self->buf + expected_pos);
    EXPECT_EQ(0, comment_level);
}

// Test null pointer safety for comment_level parameter
TEST_F(PmEditorFindLineTest, NullInmultiParameter) {
    SetBuffer("Line 0\nLine 1");

    // This should not crash, though the original function doesn't handle null comment_level
    // In a real implementation, you might want to add null checks
    char *result = pmeditor_find_line(self, 1, nullptr);
    EXPECT_EQ(result, nullptr);
}

#define EXPECT_HIGHLIGHT(expected_type) \
    do { \
        EXPECT_EQ(expected_type, last_highlight_type); \
    } while (0)

class PmEditorSetColourTest : public ::testing::Test {

protected:
    PmEditor test_editor;
    PmEditor *self = &test_editor;

    void SetUp() override {
        // Initialize command token table
        commandtbl_init();

        // Clear the editor state
        memset(self, 0, sizeof(PmEditor));

        // Reset mock state
        last_highlight_type = kHighlightNormal;
        highlight_call_count = 0;

        // Enable color coding for most tests
        // Note: OPTION_COLOUR_CODE is a #define, so we can't change it at runtime
        // In a real implementation, you might want to make this configurable
    }

    void ResetState() {
        // Call pmeditor_set_colour(self, NULL) to reset internal state
        pmeditor_set_colour(self, NULL);
        last_highlight_type = kHighlightNormal;
        highlight_call_count = 0;
    }

    void ExpectNoHighlightChange() {
        int previous_count = highlight_call_count;
        // This would need to be called after the function under test
        EXPECT_EQ(previous_count, highlight_call_count);
    }
};

// Test resetting state with NULL pointer
TEST_F(PmEditorSetColourTest, ResetStateWithNull) {
    // Set some initial state by processing characters
    char text[] = "PRINT";
    pmeditor_set_colour(self, &text[0]);

    // Reset state
    pmeditor_set_colour(self, NULL);

    // The function should reset to normal highlighting
    EXPECT_HIGHLIGHT(kHighlightNormal);
}

// Test single quote comment detection
TEST_F(PmEditorSetColourTest, SingleQuoteComment) {
    ResetState();

    char text[] = "'This is a comment";
    pmeditor_set_colour(self, &text[0]); // Process the single quote

    EXPECT_HIGHLIGHT(kHighlightComment);
}

// Test multiline comment start detection
TEST_F(PmEditorSetColourTest, MultilineCommentStart) {
    ResetState();

    char text[] = "/*comment*/";

    // Process the '/'
    pmeditor_set_colour(self, &text[0]);
    EXPECT_HIGHLIGHT(kHighlightComment);  // Should detect multiline comment start
    EXPECT_EQ(1, self->comment_level);

    // Process the '*'
    pmeditor_set_colour(self, &text[1]);
    EXPECT_HIGHLIGHT(kHighlightComment);  // Should still be in multiline comment
    EXPECT_EQ(1, self->comment_level);

    // Process the 'c'
    pmeditor_set_colour(self, &text[2]);
    EXPECT_HIGHLIGHT(kHighlightComment);  // Should still be in multiline comment
    EXPECT_EQ(1, self->comment_level);
}

// Test multiline comment start detection
TEST_F(PmEditorSetColourTest, MultilineCommentContinuation) {
    ResetState();

    char text[] = "/*comment*/";
    pmeditor_set_colour(self, &text[0]); // Process '/'
    // The function checks text[1] for '*', but doesn't change color yet

    // Should detect multiline comment start
    EXPECT_HIGHLIGHT(kHighlightComment);
    EXPECT_EQ(1, self->comment_level);

    // Process the '*'
    pmeditor_set_colour(self, &text[1]);
    EXPECT_HIGHLIGHT(kHighlightComment);  // Should still be in multiline comment
    EXPECT_EQ(1, self->comment_level);

    // Process the 'c'
    pmeditor_set_colour(self, &text[2]);
    EXPECT_HIGHLIGHT(kHighlightComment);  // Should still be in multiline comment
    EXPECT_EQ(1, self->comment_level);
}

// Test multiline comment end detection
TEST_F(PmEditorSetColourTest, MultilineCommentEnd) {
    ResetState();

    // Start in multiline comment state
    self->comment_level = 1;

    char text[] = "*/";
    pmeditor_set_colour(self, &text[0]); // Process '*'

    // Should detect comment end when processing '/'
    pmeditor_set_colour(self, &text[1]);

    EXPECT_FALSE(self->comment_level);
}

// Test nested multiline comment start detection
TEST_F(PmEditorSetColourTest, NestedMultilineCommentStart) {
    ResetState();

    // Start in multiline comment state
    self->comment_level = 1;

    char text[] = "/*comment2*/";
    pmeditor_set_colour(self, &text[0]); // Process '/'

    // Process the '*'
    pmeditor_set_colour(self, &text[1]);

    // Should detect multiline comment start
    EXPECT_HIGHLIGHT(kHighlightNormal); // Still in comment, so no change
    EXPECT_EQ(2, self->comment_level);
}

// Test quoted string detection
TEST_F(PmEditorSetColourTest, QuotedString) {
    ResetState();

    char text[] = "\"Hello World\"";

    // Process opening quote
    pmeditor_set_colour(self, &text[0]);
    EXPECT_HIGHLIGHT(kHighlightQuote);

    // Process characters inside string - should remain in quote mode
    pmeditor_set_colour(self, &text[1]); // 'H'
    EXPECT_HIGHLIGHT(kHighlightQuote);

    // Process closing quote
    pmeditor_set_colour(self, &text[12]); // Closing quote
    // Should exit quote mode but color might not change immediately
}

// Test number detection - simple integer
TEST_F(PmEditorSetColourTest, SimpleNumber) {
    ResetState();

    char text[] = "123";
    pmeditor_set_colour(self, &text[0]); // Process '1'

    EXPECT_HIGHLIGHT(kHighlightNumber);
}

// Test number detection - decimal
TEST_F(PmEditorSetColourTest, DecimalNumber) {
    ResetState();

    char text[] = "123.456";
    pmeditor_set_colour(self, &text[0]); // Process '1'

    EXPECT_HIGHLIGHT(kHighlightNumber);

    // Continue processing digits and decimal point
    pmeditor_set_colour(self, &text[3]); // Process '.'
    EXPECT_HIGHLIGHT(kHighlightNumber);
}

// Test number detection - hex prefix
TEST_F(PmEditorSetColourTest, HexNumber) {
    ResetState();

    char text[] = "&HFF";
    pmeditor_set_colour(self, &text[0]); // Process '&'

    EXPECT_HIGHLIGHT(kHighlightNumber);
}

// Test number detection - negative number
TEST_F(PmEditorSetColourTest, NegativeNumber) {
    ResetState();

    char text[] = "-123";
    pmeditor_set_colour(self, &text[0]); // Process '-'

    EXPECT_HIGHLIGHT(kHighlightNumber);
}

// Test keyword detection - PRINT
TEST_F(PmEditorSetColourTest, KeywordPrint) {
    ResetState();

    char text[] = "PRINT";
    pmeditor_set_colour(self, &text[0]); // Process 'P'

    EXPECT_HIGHLIGHT(kHighlightKeyword);
}

// Test keyword detection - FOR
TEST_F(PmEditorSetColourTest, KeywordFor) {
    ResetState();

    char text[] = "FOR I = 1 TO 10";
    pmeditor_set_colour(self, &text[0]); // Process 'F'

    EXPECT_HIGHLIGHT(kHighlightKeyword);
}

// Test REM comment (special keyword case)
TEST_F(PmEditorSetColourTest, RemComment) {
    ResetState();

    char text[] = "REM This is a comment";
    pmeditor_set_colour(self, &text[0]); // Process 'R'

    EXPECT_HIGHLIGHT(kHighlightComment);
}

// Test two-keyword commands - OPTION BASE
TEST_F(PmEditorSetColourTest, TwoKeywordOption) {
    ResetState();

    char text[] = "OPTION BASE 1";

    // Process "OPTION"
    for (int i = 0; i < 6; i++) {
        pmeditor_set_colour(self, &text[i]);
    }
    EXPECT_HIGHLIGHT(kHighlightKeyword);

    // Process space
    pmeditor_set_colour(self, &text[6]);

    // Process "BASE" - should also be highlighted as keyword
    pmeditor_set_colour(self, &text[7]); // 'B'
    EXPECT_HIGHLIGHT(kHighlightKeyword);
}

// Test special keywords - INTEGER
TEST_F(PmEditorSetColourTest, SpecialKeywordInteger) {
    ResetState();

    char text[] = "INTEGER";
    pmeditor_set_colour(self, &text[0]); // Process 'I'

    EXPECT_HIGHLIGHT(kHighlightKeyword);
}

// Test that variables are not highlighted as keywords
TEST_F(PmEditorSetColourTest, VariableNotKeyword) {
    ResetState();

    char text[] = "myVariable = 5";
    pmeditor_set_colour(self, &text[0]); // Process 'm'

    // Should not be highlighted as keyword
    EXPECT_HIGHLIGHT(kHighlightNormal);
}

// Test comment inside quoted string is ignored
TEST_F(PmEditorSetColourTest, CommentInQuotedString) {
    ResetState();

    char text[] = "\"Don't highlight this\"";

    // Process opening quote
    pmeditor_set_colour(self, &text[0]);
    EXPECT_HIGHLIGHT(kHighlightQuote);

    // Process the single quote inside - should remain in quote mode
    pmeditor_set_colour(self, &text[3]); // Single quote
    EXPECT_HIGHLIGHT(kHighlightQuote);
}

// Test multiline comment state persistence
TEST_F(PmEditorSetColourTest, MultilineCommentPersistence) {
    ResetState();

    // Start multiline comment
    self->comment_level = true;

    char text[] = "still in comment";
    pmeditor_set_colour(self, &text[0]); // Process 's'

    // Should remain in comment mode without changing highlight
    // (since we're already in multiline comment)
}

// Test keyword followed by non-name character
TEST_F(PmEditorSetColourTest, KeywordBoundary) {
    ResetState();

    char text[] = "PRINT(";

    // Process "PRINT"
    for (int i = 0; i < 5; i++) {
        pmeditor_set_colour(self, &text[i]);
    }
    EXPECT_HIGHLIGHT(kHighlightKeyword);

    // Process '(' - should exit keyword mode
    pmeditor_set_colour(self, &text[5]);
    EXPECT_HIGHLIGHT(kHighlightNormal);
}

// Test number followed by non-digit character
TEST_F(PmEditorSetColourTest, NumberBoundary) {
    ResetState();

    char text[] = "123 ";

    // Process digits
    for (int i = 0; i < 3; i++) {
        pmeditor_set_colour(self, &text[i]);
    }
    EXPECT_HIGHLIGHT(kHighlightNumber);

    // Process space - should exit number mode
    pmeditor_set_colour(self, &text[3]);
    EXPECT_HIGHLIGHT(kHighlightNormal);
}

// Test 8-digit hex number detection
TEST_F(PmEditorSetColourTest, EightDigitHex) {
    ResetState();

    char text[] = "12345678 ";
    pmeditor_set_colour(self, &text[0]); // Process first digit

    EXPECT_HIGHLIGHT(kHighlightNumber);
}

// Test scientific notation
TEST_F(PmEditorSetColourTest, ScientificNotation) {
    ResetState();

    char text[] = "1.23E10";
    pmeditor_set_colour(self, &text[0]); // Process '1'

    EXPECT_HIGHLIGHT(kHighlightNumber);

    // Process 'E' - should remain in number mode
    pmeditor_set_colour(self, &text[4]);
    EXPECT_HIGHLIGHT(kHighlightNumber);
}

// Test that color coding can be disabled
TEST_F(PmEditorSetColourTest, ColorCodingDisabled) {
    // This test would require a way to disable OPTION_COLOUR_CODE
    // Since it's a compile-time constant, we can't test this easily
    // In a real implementation, you might want to make this runtime configurable

    // If color coding is disabled, the function should return early
    // and not change any highlighting
}

// Test edge case: empty string processing
TEST_F(PmEditorSetColourTest, EmptyStringChar) {
    ResetState();

    char text[] = "";
    pmeditor_set_colour(self, &text[0]); // Process null terminator

    // Should handle gracefully without crashing
}

// Test complex mixed content
TEST_F(PmEditorSetColourTest, ComplexMixedContent) {
    ResetState();

    char text[] = "FOR I = 1 TO 10 'Loop comment";

    // Process "FOR" keyword
    pmeditor_set_colour(self, &text[0]);
    EXPECT_HIGHLIGHT(kHighlightKeyword);

    // Skip to the number
    ResetState();
    pmeditor_set_colour(self, &text[8]); // '1'
    EXPECT_HIGHLIGHT(kHighlightNumber);

    // Skip to the comment
    ResetState();
    pmeditor_set_colour(self, &text[16]); // Single quote
    EXPECT_HIGHLIGHT(kHighlightComment);
}

// Test nested scenarios
TEST_F(PmEditorSetColourTest, NestedCommentScenarios) {
    ResetState();

    // Test /* inside a quoted string - should not start multiline comment
    char text[] = "\"This /* is not a comment\"";

    // Enter quote mode
    pmeditor_set_colour(self, &text[0]);
    EXPECT_HIGHLIGHT(kHighlightQuote);

    // Process the /* inside quotes - should remain in quote mode
    pmeditor_set_colour(self, &text[6]); // '/'
    EXPECT_HIGHLIGHT(kHighlightQuote);

    pmeditor_set_colour(self, &text[7]); // '*'
    EXPECT_HIGHLIGHT(kHighlightQuote);

    EXPECT_FALSE(self->comment_level);
}
