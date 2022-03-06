#include <gtest/gtest.h>

extern "C" {

#include "../parse.h"
#include "../utility.h"
#include "../../Configuration.h"

long long int getinteger(char *p) { return 0; }
int getint(char *p, int min, int max) { return 0; }

}

class ParseTest : public ::testing::Test {

protected:

    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(ParseTest, ParseTransformInputBuffer_GivenStarCommand) {
    char input[STRINGSIZE];
    char expected[STRINGSIZE];

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
    EXPECT_STREQ("RUN \"foo\", bar wom bat", input);

    // Given arguments and trailing whitespace.
    strcpy(input, "*foo bar wom bat    ");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo\", bar wom bat", input);

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
    EXPECT_STREQ("RUN \"foo bar\", wom bat", input);

    // Given quoted file and no space before first argument.
    strcpy(input, "*\"foo bar\"wom bat");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo bar\", wom bat", input);

    // Give file has leading but no trailing quote
    // - note that the resulting command will be invalid.
    strcpy(input, "*\"foo bar wom bat");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("RUN \"foo bar wom bat", input);

    // Given maximum length input.
    memset(input, 'A', STRINGSIZE);
    input[0] = '*';
    memset(input + 1, 'A', 249);
    input[250] = '\0';
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_EQ(STRINGSIZE - 1, strlen(input));
    memset(expected, 'A', STRINGSIZE);
    memcpy(expected, "RUN \"", 5);
    memset(expected + 5, 'A', 249);
    expected[254] = '"';
    expected[255] = '\0';
    EXPECT_STREQ(expected, input);

    // Given expanded command is too long.
    memset(input, 'A', STRINGSIZE);
    input[0] = '*';
    memset(input + 1, 'A', 250);
    input[251] = '\0';
    strcpy(expected, input);
    EXPECT_EQ(kStringTooLong, parse_transform_input_buffer(input));
    EXPECT_STREQ(expected, input);
}

TEST_F(ParseTest, ParseTransformInputBuffer_GivenBangCommand) {
    char input[STRINGSIZE];
    char expected[STRINGSIZE];

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
    // input:    !<246 * 'A'>
    // expected: SYSTEM "<246 * 'A'>"
    input[0] = '!';
    memset(input + 1, 'A', 246);
    input[247] = '\0';
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_EQ(STRINGSIZE - 1, strlen(input));
    memcpy(expected, "SYSTEM \"", 8);
    memset(expected + 8, 'A', 246);
    expected[254] = '"';
    expected[255] = '\0';
    EXPECT_STREQ(expected, input);

    // Given bang with a command that is too long.
    // input:    !<247 * 'A'>
    // expected: kStringTooLong
    input[0] = '!';
    memset(input + 1, 'A', 247);
    input[248] = '\0';
    strcpy(expected, input);
    EXPECT_EQ(kStringTooLong, parse_transform_input_buffer(input));
    EXPECT_STREQ(expected, input);

    // Given bang with a command whose Chr$(34) expansion makes it of maximum length.
    // input:    !"<235 * 'A'>
    // expected: SYSTEM Chr$(34) + "<235 * 'A'>"
    input[0] = '!';
    input[1] = '"';
    memset(input + 2, 'A', 235);
    input[237] = '\0'; // !"<235 x 'A'>
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_EQ(STRINGSIZE - 1, strlen(input));
    memcpy(expected, "SYSTEM Chr$(34) + \"", 19);
    memset(expected + 19, 'A', 235);
    expected[254] = '"';
    expected[255] = '\0';
    EXPECT_STREQ(expected, input);

    // Given bang with a command whose Chr$(34) expansion makes it too long.
    // input:    !"<236 * 'A'>
    // expected: kStringTooLong
    input[0] = '!';
    input[1] = '"';
    memset(input + 2, 'A', 236);
    input[238] = '\0';
}

TEST_F(ParseTest, ParseTransformInputBuffer_GivenBangCdCommand) {
    char input[STRINGSIZE];
    char expected[STRINGSIZE];

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
    char input[STRINGSIZE];
    strcpy(input, "  PRINT \"Hello World\"  ");
    EXPECT_EQ(kOk, parse_transform_input_buffer(input));
    EXPECT_STREQ("  PRINT \"Hello World\"  ", input);
}
