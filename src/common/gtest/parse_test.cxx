/*
 * Copyright (c) 2021-2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "test_helper.h"
#include "../cstring.h"
#include "../error.h"
#include "../graphics.h"
#include "../memory.h"
#include "../parse.h"
#include "../utility.h"
#include "../../core/Commands.h"
#include "../../core/MMBasic.h"
#include "../../core/vartbl.h"
#include "../../core/gtest/command_stubs.h"
#include "../../core/gtest/function_stubs.h"
#include "../../core/gtest/operation_stubs.h"

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
char **FontTable;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Options mmb_options;
ErrorState mmb_normal_error_state;
uint8_t mmb_exit_code = 0;
int MMgetchar(void) { return 0; }

// Defined in "commands/cmd_read.c"
void cmd_read_clear_cache()  { }

// Defined in "common/console.c"
int console_kbhit(void) { return 0; }
char console_putc(char c) { return c; }
void console_puts(const char *s) { }
void console_set_title(const char *title) { }
size_t console_write(const char *buf, size_t sz) { return 0; }

// Defined in "common/fonttbl.c"
void font_clear_user_defined(void) { }

// Defined in "common/graphics.c"
MmSurface graphics_surfaces[GRAPHICS_MAX_SURFACES];
MmResult graphics_term(void) { return kOk; }

// Defined in "common/interrupt.c"
bool interrupt_check() { return true; }
void interrupt_clear() { }
void interrupt_disable_serial_rx(int fnbr) {}
void interrupt_enable_serial_rx(int fnbr, int64_t count, const char *interrupt_addr) {}

// Defined in "core/Commands.c"
char DimUsed;
int doindex;
struct s_dostack dostack[MAXDOLOOPS];
const char *errorstack[MAXGOSUB];
int forindex;
struct s_forstack forstack[MAXFORLOOPS + 1];
int gosubindex;
const char *gosubstack[MAXGOSUB];
int TraceBuffIndex;
const char *TraceBuff[TRACE_BUFF_SIZE];
int TraceOn;
void CheckAbort(void) { }
void ListNewLine(int *ListCnt, int all) { }

}

class ParseTest : public ::testing::Test {

protected:

    void SetUp() override {
        vartbl_init_called = false;
        errno = 0;
        strcpy(error_msg, "");
        InitBasic();
        clear_prog_memory();
    }

    void TearDown() override {
        ClearTempMemory();
        memset(graphics_surfaces, 0x0, sizeof(graphics_surfaces));
    }
};

TEST_F(ParseTest, ParseMatchesLongStringPattern) {
    EXPECT_TRUE(parse_matches_longstring_pattern("foo%()"));
    EXPECT_TRUE(parse_matches_longstring_pattern("foo()"));
    EXPECT_TRUE(parse_matches_longstring_pattern(" foo  (   ) "));
    EXPECT_FALSE(parse_matches_longstring_pattern("foo%"));
    EXPECT_FALSE(parse_matches_longstring_pattern("foo$()"));
}

TEST_F(ParseTest, ParseName) {
    char name[MAXVARLEN + 1];

    std::string text = " foo ";
    const char *p = text.c_str();
    EXPECT_EQ(kOk, parse_name(&p, name));
    EXPECT_EQ(text.c_str() + 4, p);
    EXPECT_STREQ("FOO", name);

    text = "_32_chars_long_67890123456789012";
    p = text.c_str();
    EXPECT_EQ(kOk, parse_name(&p, name));
    EXPECT_EQ(text.c_str() + 32, p);
    EXPECT_STREQ("_32_CHARS_LONG_67890123456789012", name);
}

TEST_F(ParseTest, ParseName_GivenInvalidName) {
    char name[MAXVARLEN + 1];

    std::string text = "";
    const char *p = text.c_str();
    EXPECT_EQ(kInvalidName, parse_name(&p, name));

    text = " *";
    p = text.c_str();
    EXPECT_EQ(kInvalidName, parse_name(&p, name));
}

TEST_F(ParseTest, ParseName_GivenNameTooLong) {
    std::string text = " _33_chars_long_678901234567890123 ";
    const char *p = text.c_str();
    char name[MAXVARLEN + 1];

    EXPECT_EQ(kNameTooLong, parse_name(&p, name));
    EXPECT_EQ(text.c_str() + 33, p);
    EXPECT_STREQ("_33_CHARS_LONG_67890123456789012", name);
}

TEST_F(ParseTest, ParseTransformInputBuffer_GivenStarCommand) {
    char input[INPBUF_SIZE];
    char expected[INPBUF_SIZE];

    // Given star with a file.
    strcpy(input, "*foo");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo\"", input);

    // Given leading and trailing whitespace.
    strcpy(input, " *foo ");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo\"", input);

    // Given arguments.
    strcpy(input, "*foo bar wom bat");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo\", \"bar wom bat\"", input);

    // Given arguments and trailing whitespace.
    strcpy(input, "*foo bar wom bat    ");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo\", \"bar wom bat\"", input);

    // Given just star.
    strcpy(input, "*");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN", input);

    // Given just star with leading and trailing whitespace.
    strcpy(input, " * ");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN", input);

    // Given file to run is already quoted.
    strcpy(input, "*\"foo bar\" wom bat");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo bar\", \"wom bat\"", input);

    // Given quoted file and no space before first argument.
    strcpy(input, "*\"foo bar\"wom bat");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo bar\", \"wom bat\"", input);

    // Given file has leading but no trailing quote
    // - note that the resulting command will be invalid.
    strcpy(input, "*\"foo bar wom bat");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo bar wom bat", input);

    // Given single argument containing quotes.
    strcpy(input, "*foo --outfile=\"wom bat\"");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo\", \"--outfile=\" + Chr$(34) + \"wom bat\" + Chr$(34)", input);

    // Given whole argument quoted.
    strcpy(input, "*foo \"wom bat\"");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo\", Chr$(34) + \"wom bat\" + Chr$(34)", input);

    // Given two quoted arguments.
    strcpy(input, "*foo \"wom\" \"bat\"");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo\", Chr$(34) + \"wom\" + Chr$(34) + \" \" + Chr$(34) + \"bat\" + Chr$(34)", input);

    // Given maximum length input.
    input[0] = '\0';
    cstring_cat(input, "*foo \"", INPBUF_SIZE);
    for (int i = 0; i < INPBUF_SIZE - 36; ++i) cstring_cat(input, "A", INPBUF_SIZE);
    cstring_cat(input, "\"", INPBUF_SIZE);

    expected[0] = '\0';
    cstring_cat(expected, "RUN \"foo\", Chr$(34) + \"", INPBUF_SIZE);
    for (int i = 0; i < INPBUF_SIZE - 36; ++i) cstring_cat(expected, "A", INPBUF_SIZE);
    cstring_cat(expected, "\" + Chr$(34)", INPBUF_SIZE);

    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_EQ(INPBUF_SIZE - 1, strlen(input));
    EXPECT_STREQ(expected, input);

    // Given expanded command is too long.
    input[0] = '\0';
    cstring_cat(input, "*foo \"", INPBUF_SIZE);
    for (int i = 0; i < INPBUF_SIZE - 35; ++i) cstring_cat(input, "A", INPBUF_SIZE);
    cstring_cat(input, "\"", INPBUF_SIZE);
    strcpy(expected, input);

    EXPECT_EQ(kStringTooLong, parse_transform_input_buffer(input));
    EXPECT_STREQ(expected, input);
}

TEST_F(ParseTest, ParseTransformInputBuffer_GivenBangCommand) {
    char input[INPBUF_SIZE];
    char expected[INPBUF_SIZE];

    // Given bang on its own.
    strcpy(input, "!");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("SYSTEM", input);

    // Given bang surrounded by whitespace.
    strcpy(input, "  !  ");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("SYSTEM", input);

    // Given bang with a command.
    strcpy(input, "!foo");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("SYSTEM \"foo\"", input);

    // Given bang with an unquoted command and assorted whitespace.
    strcpy(input, "  !  foo  ");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("SYSTEM \"foo\"", input);

    // Given bang with a quoted command.
    strcpy(input, "!foo \"bar\" \"wombat");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("SYSTEM \"foo \" + Chr$(34) + \"bar\" + Chr$(34) + \" \" + Chr$(34) + \"wombat\"", input);
    strcpy(input, "!\"foo\"");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("SYSTEM Chr$(34) + \"foo\" + Chr$(34)", input);

    // Given bang with a command of maximum length.
    // input:    !<502 * 'A'>
    // expected: SYSTEM "<502 * 'A'>"
    input[0] = '\0';
    cstring_cat(input, "!", INPBUF_SIZE);
    for (int i = 0; i < INPBUF_SIZE - 10; ++i) cstring_cat(input, "A", INPBUF_SIZE);

    expected[0] = '\0';
    cstring_cat(expected, "SYSTEM \"", INPBUF_SIZE);
    cstring_cat(expected, input + 1, INPBUF_SIZE);
    cstring_cat(expected, "\"", INPBUF_SIZE);

    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_EQ(INPBUF_SIZE - 1, strlen(input));
    EXPECT_STREQ(expected, input);

    // Given bang with a command that is too long.
    // input:    !<503 * 'A'>
    // expected: kStringTooLong
    input[0] = '\0';
    cstring_cat(input, "!", INPBUF_SIZE);
    for (int i = 0; i < INPBUF_SIZE - 9; ++i) cstring_cat(input, "A", INPBUF_SIZE);
    strcpy(expected, input);

    EXPECT_EQ(kStringTooLong, parse_transform_input_buffer(input));
    EXPECT_STREQ(expected, input);

    // Given bang with a command whose Chr$(34) expansion makes it of maximum length.
    // input:    !"<491 * 'A'>
    // expected: SYSTEM Chr$(34) + "<491 * 'A'>"
    input[0] = '\0';
    cstring_cat(input, "!\"", INPBUF_SIZE);
    for (int i = 0; i < INPBUF_SIZE - 21; ++i) cstring_cat(input, "A", INPBUF_SIZE);

    expected[0] = '\0';
    cstring_cat(expected, "SYSTEM Chr$(34) + \"", INPBUF_SIZE);
    cstring_cat(expected, input + 2, INPBUF_SIZE);
    cstring_cat(expected, "\"", INPBUF_SIZE);

    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_EQ(INPBUF_SIZE - 1, strlen(input));
    EXPECT_STREQ(expected, input);

    // Given bang with a command whose Chr$(34) expansion makes it too long.
    // input:    !"<492 * 'A'>
    // expected: kStringTooLong
    input[0] = '\0';
    cstring_cat(input, "!\"", INPBUF_SIZE);
    for (int i = 0; i < INPBUF_SIZE - 20; ++i) cstring_cat(input, "A", INPBUF_SIZE);
    strcpy(expected, input);

    EXPECT_EQ(kStringTooLong, parse_transform_input_buffer(input));
    EXPECT_STREQ(expected, input);
}

TEST_F(ParseTest, ParseTransformInputBuffer_GivenBangCdCommand) {
    char input[INPBUF_SIZE];

    // Given no directory.
    strcpy(input, "!cd");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("CHDIR \"~\"", input);

    // Given no directory and assorted whitespace.
    strcpy(input, "  ! cd    ");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("CHDIR \"~\"", input);

    // Given unquoted directory.
    strcpy(input, "!cd foo");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("CHDIR \"foo\"", input);

    // Given unquoted directory with assorted whitespace.
    strcpy(input, "  ! cd   foo  ");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("CHDIR \"foo\"", input);

    // Given quoted directory.
    strcpy(input, "!cd \"foo\"");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("CHDIR \"foo\"", input);

    // Given quoted directory with spaces.
    strcpy(input, "!cd \"foo bar\"");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("CHDIR \"foo bar\"", input);

    // Given command with semi-colon ;
    strcpy(input, "!cd ; pwd");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("SYSTEM \"cd ; pwd\"", input);

    // Given command with &&
    strcpy(input, "!cd && pwd");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("SYSTEM \"cd && pwd\"", input);

    // Given command with ||
    strcpy(input, "!cd || pwd");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("SYSTEM \"cd || pwd\"", input);

    // Given command with a cd suffix.
    strcpy(input, "!cdsuffix");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("SYSTEM \"cdsuffix\"", input);
}

TEST_F(ParseTest, ParseTransformInputBuffer_GivenNormalCommand) {
    char input[INPBUF_SIZE];
    strcpy(input, "  PRINT \"Hello World\"  ");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("  PRINT \"Hello World\"  ", input);
}

TEST_F(ParseTest, CheckString) {
    char input[INPBUF_SIZE];
    strcpy(input, "PRINT \"Hello World\"");

    EXPECT_EQ(input + 6, parse_check_string(input, "PRINT"));
    EXPECT_EQ(input + 6, parse_check_string(input, "Print"));
    EXPECT_EQ(NULL, parse_check_string(input, "Printer"));
    EXPECT_EQ(NULL, parse_check_string(input, "DIM"));
}

TEST_F(ParseTest, CheckString_TerminatedBySpace) {
    char input[INPBUF_SIZE];
    strcpy(input, " foo bar");

    const char *actual = parse_check_string(input, "foo");
    EXPECT_EQ(input + 5, actual); // Expect 'b' of "bar".
}

TEST_F(ParseTest, CheckString_TerminatedByComma) {
    char input[INPBUF_SIZE];
    strcpy(input, " foo,bar");

    const char *actual = parse_check_string(input, "foo");
    EXPECT_EQ(input + 4, actual); // Expect ',' position.
}

TEST_F(ParseTest, CheckString_TerminatedBySingleQuote) {
    char input[INPBUF_SIZE];
    strcpy(input, " foo'bar");

    const char *actual = parse_check_string(input, "foo");
    EXPECT_EQ(input + 4, actual); // Expect single-quote position.
}

TEST_F(ParseTest, CheckString_TerminatedByOpenBracket)  {
    char input[INPBUF_SIZE];
    strcpy(input, " foo(bar");

    const char *actual = parse_check_string(input, "foo");
    EXPECT_EQ(input + 4, actual); // Expect '(' position.
}

TEST_F(ParseTest, CheckString_TerminatedByEquals)  {
    char input[INPBUF_SIZE];
    strcpy(input, " foo=bar");

    const char *actual = parse_check_string(input, "foo");
    EXPECT_EQ(input + 4, actual); // Expect '=' position.
}

TEST_F(ParseTest, CheckString_IsCaseInsensitive) {
    char input[INPBUF_SIZE];
    strcpy(input, " foo bar");

    const char *actual = parse_check_string(input, "fOO");
    EXPECT_EQ(input + 5, actual); // Expect 'b' of "bar".
}

TEST_F(ParseTest, CheckString_GivenNotFound) {
    char input[INPBUF_SIZE];
    strcpy(input, " foo bar");

    const char *actual = parse_check_string(input, "bar");
    EXPECT_EQ(NULL, actual); // Not found.
}

TEST_F(ParseTest, CheckString_IgnoresLeadingSpaces) {
    char input[INPBUF_SIZE];
    strcpy(input, "     foo bar");

    const char *actual = parse_check_string(input, "foo");
    EXPECT_EQ(input + 9, actual); // Expect 'b' of "bar".
}

TEST_F(ParseTest, CheckString_SkipsTrailingSpaces) {
    char input[INPBUF_SIZE];
    strcpy(input, " foo     bar");

    const char *actual = parse_check_string(input, "foo");
    EXPECT_EQ(input + 9, actual); // Expect 'b' of "bar".
}

TEST_F(ParseTest, CheckString_DoesNotMakePartialMatches) {
    char input[INPBUF_SIZE];
    strcpy(input, " foo bar");

    const char *actual = parse_check_string(input, "fo");
    EXPECT_EQ(NULL, actual); // Not found.

    actual = parse_check_string(input, "football");
    EXPECT_EQ(NULL, actual); // Not found.
}

TEST_F(ParseTest, ParsePage_GivenValidExistingPageId_AndNonPicoMite) {
    graphics_surfaces[0].type = kGraphicsBuffer;
    graphics_surfaces[1].type = kGraphicsBuffer;

    {
        clear_prog_memory();
        tokenise_and_append("PAGE WRITE 0");

        const char *p = ProgMemory + 9;
        MmSurfaceId page_id = -1;
        EXPECT_EQ(kOk, parse_page(p, &page_id));
        EXPECT_EQ(0, page_id);
    }

    {
        clear_prog_memory();
        tokenise_and_append("PAGE WRITE 1");

        const char *p = ProgMemory + 9;
        MmSurfaceId page_id = -1;
        EXPECT_EQ(kOk, parse_page(p, &page_id));
        EXPECT_EQ(1, page_id);
    }
}

TEST_F(ParseTest, ParsePage_GivenValidExistingPageId_AndPicoMite) {
    mmb_options.simulate = kSimulatePicoMiteVga;
    graphics_surfaces[GRAPHICS_SURFACE_N].type = kGraphicsBuffer;
    graphics_surfaces[GRAPHICS_SURFACE_F].type = kGraphicsBuffer;
    graphics_surfaces[GRAPHICS_SURFACE_L].type = kGraphicsBuffer;

    {
        clear_prog_memory();
        tokenise_and_append("PAGE WRITE N");

        const char *p = ProgMemory + 9;
        MmSurfaceId page_id = -1;
        EXPECT_EQ(kOk, parse_page(p, &page_id));
        EXPECT_EQ(1, page_id);
    }

    {
        clear_prog_memory();
        tokenise_and_append("PAGE WRITE F");

        const char *p = ProgMemory + 9;
        MmSurfaceId page_id = -1;
        EXPECT_EQ(kOk, parse_page(p, &page_id));
        EXPECT_EQ(2, page_id);
    }

    {
        clear_prog_memory();
        tokenise_and_append("PAGE WRITE L");

        const char *p = ProgMemory + 9;
        MmSurfaceId page_id = -1;
        EXPECT_EQ(kOk, parse_page(p, &page_id));
        EXPECT_EQ(3, page_id);
    }
}

TEST_F(ParseTest, ParsePage_GivenValidExistingPageIdAsString_AndPicoMite) {
    mmb_options.simulate = kSimulatePicoMiteVga;
    graphics_surfaces[GRAPHICS_SURFACE_N].type = kGraphicsBuffer;
    graphics_surfaces[GRAPHICS_SURFACE_F].type = kGraphicsBuffer;
    graphics_surfaces[GRAPHICS_SURFACE_L].type = kGraphicsBuffer;

    {
        clear_prog_memory();
        tokenise_and_append("PAGE WRITE \"N\"");

        const char *p = ProgMemory + 9;
        MmSurfaceId page_id = -1;
        EXPECT_EQ(kOk, parse_page(p, &page_id));
        EXPECT_EQ(1, page_id);
    }

    {
        clear_prog_memory();
        tokenise_and_append("PAGE WRITE \"F\"");

        const char *p = ProgMemory + 9;
        MmSurfaceId page_id = -1;
        EXPECT_EQ(kOk, parse_page(p, &page_id));
        EXPECT_EQ(2, page_id);
    }

    {
        clear_prog_memory();
        tokenise_and_append("PAGE WRITE \"L\"");

        const char *p = ProgMemory + 9;
        MmSurfaceId page_id = -1;
        EXPECT_EQ(kOk, parse_page(p, &page_id));
        EXPECT_EQ(3, page_id);
    }
}

TEST_F(ParseTest, ParsePage_GivenUnknownStringPageId_AndPicomite) {
    mmb_options.simulate = kSimulatePicoMiteVga;
    tokenise_and_append("PAGE WRITE \"A\"");

    const char *p = ProgMemory + 9;
    MmSurfaceId page_id = -1;
    EXPECT_EQ(kSyntax, parse_page(p, &page_id));
    EXPECT_EQ(-1, page_id);
}

TEST_F(ParseTest, ParsePage_GivenUnknownNonStringPageId_AndPicomite) {
    GTEST_SKIP() << "Segfaults due to longjmp() error handling";
    mmb_options.simulate = kSimulatePicoMiteVga;
    tokenise_and_append("PAGE WRITE 1");
 //    printf("### %s\n", mmresult_to_string(258));

    const char *p = ProgMemory + 9;
    MmSurfaceId page_id = -1;
    EXPECT_EQ(kSyntax, parse_page(p, &page_id));
    EXPECT_EQ(-1, page_id);
}

TEST_F(ParseTest, ParsePage_GivenValidNonExistingPageId_AndPicomite) {
    mmb_options.simulate = kSimulatePicoMiteVga;
    tokenise_and_append("PAGE WRITE \"N\"");

    const char *p = ProgMemory + 9;
    MmSurfaceId page_id = -1;
    EXPECT_EQ(kGraphicsInvalidSurface, parse_page(p, &page_id));
    EXPECT_EQ(1, page_id);
}

TEST_F(ParseTest, ParsePage_GivenValidNonExistingPageId_AndNonPicomite) {
    tokenise_and_append("PAGE WRITE 1");

    const char *p = ProgMemory + 9;
    MmSurfaceId page_id = -1;
    EXPECT_EQ(kGraphicsInvalidSurface, parse_page(p, &page_id));
    EXPECT_EQ(1, page_id);
}

TEST_F(ParseTest, ParsePage_GivenInvalidPageId_AndNonPicomite) {
    tokenise_and_append("PAGE WRITE \"A\"");

    const char *p = ProgMemory + 9;
    MmSurfaceId page_id = -1;
    EXPECT_EQ(kGraphicsInvalidSurface, parse_page(p, &page_id));
    EXPECT_EQ(0, page_id); // TODO: Should be -1
    EXPECT_STREQ("Expected a number", error_msg);
}

TEST_F(ParseTest, ParseReadPage_GivenValidNonExistingPageId_AndNonPicomite) {
    tokenise_and_append("PAGE WRITE 1");

    const char *p = ProgMemory + 9;
    MmSurfaceId page_id = -1;
    EXPECT_EQ(kGraphicsInvalidReadSurface, parse_read_page(p, &page_id));
    EXPECT_EQ(1, page_id);
}

TEST_F(ParseTest, ParseWritePage_GivenValidNonExistingPageId_AndNonPicomite) {
    tokenise_and_append("PAGE WRITE 1");

    const char *p = ProgMemory + 9;
    MmSurfaceId page_id = -1;
    EXPECT_EQ(kGraphicsInvalidWriteSurface, parse_write_page(p, &page_id));
    EXPECT_EQ(1, page_id);
}
