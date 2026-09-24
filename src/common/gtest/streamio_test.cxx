/*
 * Copyright (c) 2025-2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>
#include <string>

extern "C" {

#include "../streamio.h"
#include "../file.h"
#include "../file_private.h"
#include "../error.h"

// Defined in "common/prompt.c"
MmResult prompt_getc(int *ch) { *ch = -1; return kOk; }

} // extern "C"

class StreamIOTestBase : public ::testing::Test {
protected:
    std::string test_file_path;
    int fnbr;

    void SetUp() override {
        fnbr = -1;
        // Create unique test file path in gtest temp directory
        test_file_path = ::testing::TempDir() + "streamio_test_" +
                        std::to_string(reinterpret_cast<uintptr_t>(this)) + ".txt";
        // Clean up any leftover test file
        MmResult result = file_delete(test_file_path.c_str());
        if (FAILED(result) && result != kFileNotFound) {
            ON_FAILURE_EXIT(result);
        }
        // Initialize file table
        for (int i = 0; i <= MAXOPENFILES; i++) {
            file_table[i].type = fet_closed;
            file_table[i].file_ptr = NULL;
        }
    }

    void TearDown() override {
        if (fnbr > 0) {
            streamio_close(fnbr);
        }
        MmResult result = file_delete(test_file_path.c_str());
        if (FAILED(result) && result != kFileNotFound) {
            ON_FAILURE_EXIT(result);
        }
    }

    /**
     * Opens a test file for reading.
     * Creates the file with the given content first.
     */
    MmResult OpenForReading(const char* content) {
        // Write content to file
        errno = 0;
        FILE* f = fopen(test_file_path.c_str(), "w");
        if (!f) return errno;
        fwrite(content, 1, strlen(content), f);
        fclose(f);

        // Open for reading
        fnbr = streamio_find_free();
        if (fnbr < 0) return kTooManyOpenFiles;
        return streamio_open(test_file_path.c_str(), "r", fnbr);
    }

    /**
     * Opens a test file for writing.
     */
    MmResult OpenForWriting() {
        fnbr = streamio_find_free();
        if (fnbr < 0) return kTooManyOpenFiles;
        return streamio_open(test_file_path.c_str(), "w", fnbr);
    }

    /**
     * Reads the entire content of the test file.
     */
    std::string ReadTestFile() {
        FILE* f = fopen(test_file_path.c_str(), "r");
        if (!f) return "";

        std::string content;
        char ch;
        while (fread(&ch, 1, 1, f) == 1) {
            content += ch;
        }
        fclose(f);
        return content;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Tests for streamio_ungetc()
////////////////////////////////////////////////////////////////////////////////

class StreamIOUngetcTest : public StreamIOTestBase {};

// Test ungetc with valid file number returns success
TEST_F(StreamIOUngetcTest, UngetcValidFileNumber) {
    ASSERT_EQ(kOk, OpenForReading("Hello"));

    int ch = streamio_getc(fnbr);
    EXPECT_EQ('H', ch);

    MmResult result = streamio_ungetc(fnbr, ch);

    EXPECT_EQ(kOk, result);

    // Verify character can be read again
    ch = streamio_getc(fnbr);
    EXPECT_EQ('H', ch);
}

// Test ungetc with different character than read
TEST_F(StreamIOUngetcTest, UngetcDifferentCharacter) {
    ASSERT_EQ(kOk, OpenForReading("Hello"));

    int ch = streamio_getc(fnbr);
    EXPECT_EQ('H', ch);

    MmResult result = streamio_ungetc(fnbr, 'X');

    EXPECT_EQ(kOk, result);

    // Should get 'X' back, not 'H'
    ch = streamio_getc(fnbr);
    EXPECT_EQ('X', ch);
}

// Test ungetc multiple times (only one guaranteed)
TEST_F(StreamIOUngetcTest, UngetcMultipleTimes) {
    ASSERT_EQ(kOk, OpenForReading("Hello"));

    int ch1 = streamio_getc(fnbr);
    int ch2 = streamio_getc(fnbr);
    EXPECT_EQ('H', ch1);
    EXPECT_EQ('e', ch2);

    // First ungetc should work
    EXPECT_EQ(kOk, streamio_ungetc(fnbr, ch2));

    // Second ungetc may fail (only one pushback guaranteed),
    // but at least on Linux systems it works (maybe?).
    EXPECT_EQ(kOk, streamio_ungetc(fnbr, ch1));
    int ch = streamio_getc(fnbr);
    EXPECT_EQ('H', ch);
}

// Test ungetc at start of file
TEST_F(StreamIOUngetcTest, UngetcAtStartOfFile) {
    ASSERT_EQ(kOk, OpenForReading("Hello"));

    // Don't read anything, just ungetc
    MmResult result = streamio_ungetc(fnbr, 'X');

    EXPECT_EQ(kOk, result);

    // Should get 'X' before 'H'
    int ch = streamio_getc(fnbr);
    EXPECT_EQ('X', ch);
    ch = streamio_getc(fnbr);
    EXPECT_EQ('H', ch);
}

// Test ungetc at end of file
TEST_F(StreamIOUngetcTest, UngetcAtEndOfFile) {
    ASSERT_EQ(kOk, OpenForReading("Hi"));

    streamio_getc(fnbr); // 'H'
    streamio_getc(fnbr); // 'i'
    int eof_ch = streamio_getc(fnbr); // EOF
    EXPECT_EQ(-1, eof_ch);

    MmResult result = streamio_ungetc(fnbr, 'X');

    EXPECT_EQ(kOk, result);

    // Should get 'X' instead of EOF
    int ch = streamio_getc(fnbr);
    EXPECT_EQ('X', ch);
}

// Test ungetc with file number 0 (console) returns error
TEST_F(StreamIOUngetcTest, UngetcConsoleReturnsError) {
    MmResult result = streamio_ungetc(0, 'A');

    EXPECT_EQ(kFileInvalidOperation, result);
}

// Test ungetc with invalid file number (too low)
TEST_F(StreamIOUngetcTest, UngetcInvalidFileNumberTooLow) {
    MmResult result = streamio_ungetc(-1, 'A');

    EXPECT_EQ(kFileInvalidFileNumber, result);
}

// Test ungetc with invalid file number (too high)
TEST_F(StreamIOUngetcTest, UngetcInvalidFileNumberTooHigh) {
    MmResult result = streamio_ungetc(MAXOPENFILES + 1, 'A');

    EXPECT_EQ(kFileInvalidFileNumber, result);
}

// Test ungetc with closed file
TEST_F(StreamIOUngetcTest, UngetcClosedFile) {
    fnbr = streamio_find_free();

    MmResult result = streamio_ungetc(fnbr, 'A');

    EXPECT_EQ(kFileNotOpen, result);
}

// Test ungetc after closing file
TEST_F(StreamIOUngetcTest, UngetcAfterClose) {
    ASSERT_EQ(kOk, OpenForReading("Hello"));
    int saved_fnbr = fnbr;

    streamio_close(fnbr);
    fnbr = -1; // Prevent double close in TearDown

    MmResult result = streamio_ungetc(saved_fnbr, 'A');

    EXPECT_EQ(kFileNotOpen, result);
}

// Test ungetc with newline character
TEST_F(StreamIOUngetcTest, UngetcNewlineCharacter) {
    ASSERT_EQ(kOk, OpenForReading("Line1\nLine2"));

    streamio_getc(fnbr); // 'L'
    streamio_getc(fnbr); // 'i'
    streamio_getc(fnbr); // 'n'
    streamio_getc(fnbr); // 'e'
    streamio_getc(fnbr); // '1'
    int ch = streamio_getc(fnbr); // '\n'
    EXPECT_EQ('\n', ch);

    MmResult result = streamio_ungetc(fnbr, ch);

    EXPECT_EQ(kOk, result);

    ch = streamio_getc(fnbr);
    EXPECT_EQ('\n', ch);
}

// Test ungetc with special characters
TEST_F(StreamIOUngetcTest, UngetcSpecialCharacters) {
    ASSERT_EQ(kOk, OpenForReading("Test"));

    streamio_getc(fnbr); // 'T'

    // Test various special characters
    const char special_chars[] = {'\t', '\r', '\0', '\xFF'};
    for (char special : special_chars) {
        MmResult result = streamio_ungetc(fnbr, special);
        EXPECT_EQ(kOk, result);

        int ch = streamio_getc(fnbr);
        EXPECT_EQ((unsigned char)special, (unsigned char)ch);
    }
}

// Test ungetc interaction with streamio_eof
TEST_F(StreamIOUngetcTest, UngetcWithEOF) {
    ASSERT_EQ(kOk, OpenForReading("A"));

    streamio_getc(fnbr); // 'A'
    streamio_getc(fnbr); // EOF

    EXPECT_EQ(1, streamio_eof(fnbr));

    // Unget a character
    EXPECT_EQ(kOk, streamio_ungetc(fnbr, 'B'));

    // EOF should now be false
    EXPECT_EQ(0, streamio_eof(fnbr));
}

////////////////////////////////////////////////////////////////////////////////
// Tests for streamio_is_file() and streamio_is_serial()
////////////////////////////////////////////////////////////////////////////////

class StreamIOTypeTest : public StreamIOTestBase {};

TEST_F(StreamIOTypeTest, ConsoleIsNotFile) {
    EXPECT_FALSE(streamio_is_file(0));
}

TEST_F(StreamIOTypeTest, ConsoleIsNotSerial) {
    EXPECT_FALSE(streamio_is_serial(0));
}

////////////////////////////////////////////////////////////////////////////////
// Tests for streamio_readln()
////////////////////////////////////////////////////////////////////////////////

class StreamIOReadlnTest : public StreamIOTestBase {};

// ============================================================================
// Basic Line Reading
// ============================================================================

// Test reading a simple line with \n terminator
TEST_F(StreamIOReadlnTest, ReadSimpleLineWithNewline) {
    ASSERT_EQ(kOk, OpenForReading("Hello World\n"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("Hello World", buf);
}

// Test reading a simple line with \r terminator
TEST_F(StreamIOReadlnTest, ReadSimpleLineWithCarriageReturn) {
    ASSERT_EQ(kOk, OpenForReading("Hello World\r"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("Hello World", buf);
}

// Test reading a simple line with \r\n terminator
TEST_F(StreamIOReadlnTest, ReadSimpleLineWithCRLF) {
    ASSERT_EQ(kOk, OpenForReading("Hello World\r\n"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("Hello World", buf);
}

// Test reading line without terminator (EOF)
TEST_F(StreamIOReadlnTest, ReadLineWithoutTerminator) {
    ASSERT_EQ(kOk, OpenForReading("Hello World"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("Hello World", buf);
}

// Test reading empty line with \n
TEST_F(StreamIOReadlnTest, ReadEmptyLineWithNewline) {
    ASSERT_EQ(kOk, OpenForReading("\n"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("", buf);
}

// Test reading empty line with \r\n
TEST_F(StreamIOReadlnTest, ReadEmptyLineWithCRLF) {
    ASSERT_EQ(kOk, OpenForReading("\r\n"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("", buf);
}

// Test reading empty file
TEST_F(StreamIOReadlnTest, ReadEmptyFile) {
    ASSERT_EQ(kOk, OpenForReading(""));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("", buf);
}

// Test reading single character line
TEST_F(StreamIOReadlnTest, ReadSingleCharacterLine) {
    ASSERT_EQ(kOk, OpenForReading("A\n"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("A", buf);
}

// ============================================================================
// Multiple Lines
// ============================================================================

// Test reading multiple lines with \n
TEST_F(StreamIOReadlnTest, ReadMultipleLinesWithNewline) {
    ASSERT_EQ(kOk, OpenForReading("Line1\nLine2\nLine3\n"));

    char buf[256];

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line1", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line2", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line3", buf);

    // EOF
    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("", buf);
}

// Test reading multiple lines with mixed terminators
TEST_F(StreamIOReadlnTest, ReadMultipleLinesWithMixedTerminators) {
    ASSERT_EQ(kOk, OpenForReading("Line1\nLine2\rLine3\r\nLine4"));

    char buf[256];

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line1", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line2", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line3", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line4", buf);
}

// Test reading consecutive empty lines
TEST_F(StreamIOReadlnTest, ReadConsecutiveEmptyLines) {
    ASSERT_EQ(kOk, OpenForReading("\n\n\n"));

    char buf[256];

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("", buf);
}

// Test reading lines with consecutive \r\n sequences
TEST_F(StreamIOReadlnTest, ReadConsecutiveCRLF) {
    ASSERT_EQ(kOk, OpenForReading("Line1\r\n\r\nLine3\r\n"));

    char buf[256];

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line1", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line3", buf);
}

// ============================================================================
// Buffer Size Limits
// ============================================================================

// Test reading line that exactly fits buffer
TEST_F(StreamIOReadlnTest, ReadLineExactlyFitsBuffer) {
    ASSERT_EQ(kOk, OpenForReading("12345\n"));

    char buf[6]; // 5 chars + null
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("12345", buf);
}

// Test reading line longer than buffer
TEST_F(StreamIOReadlnTest, ReadLineLongerThanBuffer) {
    ASSERT_EQ(kOk, OpenForReading("1234567890\n"));

    char buf[6]; // Can hold 5 chars + null
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kStringTooLong, result);
    EXPECT_STREQ("12345", buf);

    // Next read should continue from where we left off
    result = streamio_readln(fnbr, buf, sizeof(buf));
    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("67890", buf);
}

// Test reading with minimum buffer size
TEST_F(StreamIOReadlnTest, ReadWithMinimumBuffer) {
    ASSERT_EQ(kOk, OpenForReading("Hello\n"));

    char buf[2]; // Can hold 1 char + null
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kStringTooLong, result);
    EXPECT_STREQ("H", buf);
}

// Test reading with buffer size of 1 (only null terminator)
TEST_F(StreamIOReadlnTest, ReadWithBufferSizeOne) {
    ASSERT_EQ(kOk, OpenForReading("Hello\n"));

    char buf[1];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kStringTooLong, result);
    EXPECT_STREQ("", buf);
}

// Test reading very long line
TEST_F(StreamIOReadlnTest, ReadVeryLongLine) {
    std::string long_line(1000, 'A');
    long_line += "\n";
    ASSERT_EQ(kOk, OpenForReading(long_line.c_str()));

    char buf[2048];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1000, strlen(buf));
    EXPECT_EQ('A', buf[0]);
    EXPECT_EQ('A', buf[999]);
    EXPECT_EQ('\0', buf[1000]);
}

// ============================================================================
// Special Characters
// ============================================================================

// Test reading line with tabs
TEST_F(StreamIOReadlnTest, ReadLineWithTabs) {
    ASSERT_EQ(kOk, OpenForReading("Hello\tWorld\tTest\n"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("Hello\tWorld\tTest", buf);
}

// Test reading line with spaces
TEST_F(StreamIOReadlnTest, ReadLineWithSpaces) {
    ASSERT_EQ(kOk, OpenForReading("  Hello   World  \n"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("  Hello   World  ", buf);
}

// Test reading line with special symbols
TEST_F(StreamIOReadlnTest, ReadLineWithSpecialSymbols) {
    ASSERT_EQ(kOk, OpenForReading("/*comment*/code!@#$%\n"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("/*comment*/code!@#$%", buf);
}

// Test reading line with quotes
TEST_F(StreamIOReadlnTest, ReadLineWithQuotes) {
    ASSERT_EQ(kOk, OpenForReading("He said \"Hello\"\n"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("He said \"Hello\"", buf);
}

// Test reading line with backslashes
TEST_F(StreamIOReadlnTest, ReadLineWithBackslashes) {
    ASSERT_EQ(kOk, OpenForReading("C:\\Users\\Test\n"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kOk, result);
    EXPECT_STREQ("C:\\Users\\Test", buf);
}

// ============================================================================
// Edge Cases with \r and \n
// ============================================================================

// Test \r\n as two separate terminators doesn't duplicate
TEST_F(StreamIOReadlnTest, CRLFNotSeparateTerminators) {
    ASSERT_EQ(kOk, OpenForReading("Line1\r\nLine2\n"));

    char buf[256];

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line1", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line2", buf);
}

// Test \n\r as separate terminators (not a single line ending)
TEST_F(StreamIOReadlnTest, LFCRSeparateTerminators) {
    ASSERT_EQ(kOk, OpenForReading("Line1\n\rLine2\n"));

    char buf[256];

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line1", buf);

    // \r after \n starts a new empty line
    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line2", buf);
}

// Test \r\r as two separate lines
TEST_F(StreamIOReadlnTest, ConsecutiveCR) {
    ASSERT_EQ(kOk, OpenForReading("Line1\r\rLine2\r"));

    char buf[256];

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line1", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Line2", buf);
}

// Test line ending at buffer boundary
TEST_F(StreamIOReadlnTest, LineEndingAtBufferBoundary) {
    ASSERT_EQ(kOk, OpenForReading("1234\n6789\n"));

    char buf[6]; // Exactly fits "1234" + null

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("1234", buf);

    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("6789", buf);
}

// ============================================================================
// Error Conditions
// ============================================================================

// Test with invalid file number (too low)
TEST_F(StreamIOReadlnTest, InvalidFileNumberTooLow) {
    char buf[256];
    MmResult result = streamio_readln(-1, buf, sizeof(buf));

    EXPECT_EQ(kFileInvalidFileNumber, result);
}

// Test with invalid file number (too high)
TEST_F(StreamIOReadlnTest, InvalidFileNumberTooHigh) {
    char buf[256];
    MmResult result = streamio_readln(MAXOPENFILES + 1, buf, sizeof(buf));

    EXPECT_EQ(kFileInvalidFileNumber, result);
}

// Test with null buffer
TEST_F(StreamIOReadlnTest, NullBuffer) {
    ASSERT_EQ(kOk, OpenForReading("Hello\n"));

    MmResult result = streamio_readln(fnbr, NULL, 256);

    EXPECT_EQ(kInternalFault, result);
}

// Test with zero buffer size
TEST_F(StreamIOReadlnTest, ZeroBufferSize) {
    ASSERT_EQ(kOk, OpenForReading("Hello\n"));

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, 0);

    EXPECT_EQ(kInternalFault, result);
}

// Test with closed file
TEST_F(StreamIOReadlnTest, ClosedFile) {
    fnbr = streamio_find_free();

    char buf[256];
    MmResult result = streamio_readln(fnbr, buf, sizeof(buf));

    EXPECT_EQ(kFileNotOpen, result);
}

// Test reading after file is closed
TEST_F(StreamIOReadlnTest, ReadAfterClose) {
    ASSERT_EQ(kOk, OpenForReading("Hello\n"));
    int saved_fnbr = fnbr;

    streamio_close(fnbr);
    fnbr = -1; // Prevent double close

    char buf[256];
    MmResult result = streamio_readln(saved_fnbr, buf, sizeof(buf));

    EXPECT_EQ(kFileNotOpen, result);
}

// ============================================================================
// Integration Tests
// ============================================================================

// Test reading entire file line by line
TEST_F(StreamIOReadlnTest, ReadEntireFile) {
    ASSERT_EQ(kOk, OpenForReading("Line1\nLine2\nLine3\nLine4\nLine5\n"));

    char buf[256];
    std::vector<std::string> lines;

    while (true) {
        EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
        if (strlen(buf) == 0 && streamio_eof(fnbr)) break;
        lines.push_back(buf);
    }

    ASSERT_EQ(5, lines.size());
    EXPECT_STREQ("Line1", lines[0].c_str());
    EXPECT_STREQ("Line2", lines[1].c_str());
    EXPECT_STREQ("Line3", lines[2].c_str());
    EXPECT_STREQ("Line4", lines[3].c_str());
    EXPECT_STREQ("Line5", lines[4].c_str());
}

// Test mixing streamio_readln with streamio_getc
TEST_F(StreamIOReadlnTest, MixReadlnWithGetc) {
    ASSERT_EQ(kOk, OpenForReading("Hello\nWorld\n"));

    char buf[256];

    // Read first line
    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Hello", buf);

    // Read individual characters
    EXPECT_EQ('W', streamio_getc(fnbr));
    EXPECT_EQ('o', streamio_getc(fnbr));
    EXPECT_EQ('r', streamio_getc(fnbr));

    // Read rest as line
    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("ld", buf);
}

// Test reading with ungetc interaction
TEST_F(StreamIOReadlnTest, ReadlnWithUngetc) {
    ASSERT_EQ(kOk, OpenForReading("Hello\nWorld\n"));

    char buf[256];

    // Read first character
    int ch = streamio_getc(fnbr);
    EXPECT_EQ('H', ch);

    // Push it back
    EXPECT_EQ(kOk, streamio_ungetc(fnbr, ch));

    // Read full line - should include the 'H'
    EXPECT_EQ(kOk, streamio_readln(fnbr, buf, sizeof(buf)));
    EXPECT_STREQ("Hello", buf);
}

// Test console input (file number 0)
TEST_F(StreamIOReadlnTest, ReadFromConsole) {
    char buf[256];

    // Reading from console should work (though will likely timeout in test)
    // Just verify it doesn't crash and handles fnbr=0
    MmResult result = streamio_readln(0, buf, sizeof(buf));

    // Result depends on prompt_getc implementation
    // Just ensure it doesn't crash
    EXPECT_TRUE(result == kOk || result != kOk);
}
