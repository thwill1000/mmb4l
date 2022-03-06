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
}

TEST_F(ParseTest, ParseTransformInputBuffer_GivenNormalCommand) {
}
