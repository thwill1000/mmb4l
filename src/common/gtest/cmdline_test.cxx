/*
 * Copyright (c) 2021-2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../cmdline.h"
#include "../cstring.h"
#include "../parse.h"
#include "../options.h"

int LocalIndex = 0;

void error_throw(MmResult error) { }
void error_throw_ex(MmResult error, char *msg, ...) { }
long long int getinteger(char *p) { return 0; }
int getint(char *p, int min, int max) { return 0; }

// Defined in "main.c"
Options mmb_options;

// Defined in "common/gpio.c"
MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num) { return kOk; }

// Defined in "core/commandtbl.c"
CommandToken cmdFUN = 0x0;
CommandToken cmdSUB = 0x0;

// Defined in "core/MMBasic.c"
char *getCstring(const char *p) { return NULL; }
const char *skipexpression(const char *p) { return NULL; }

// Defined in "core/tokentbl.c"
char tokenAS = 0x0;

}

TEST(CmdLineTest, Parse_GivenNoAdditionalArguments) {
    int argc = 1;
    const char *argv[10];
    argv[0] = "mmbasic";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST(CmdLineTest, Parse_GivenHelpFlag) {
    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "-h";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);

    args.interactive = 0;
    argv[1] = "--help";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST(CmdLineTest, Parse_GivenInteractiveFlag) {
    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "-i";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);

    args.interactive = 0;
    argv[1] = "--interactive";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST(CmdLineTest, Parse_GivenVersionFlag) {
    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "-v";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(1, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);

    args.version = 0;
    argv[1] = "--version";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(1, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST(CmdLineTest, Parse_GivenInteractiveAndVersionFlags) {
    int argc = 3;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "--interactive";
    argv[2] = "-v";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(1, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST(CmdLineTest, Parse_GivenProgramArgument) {
    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "myprogram.bas";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(0, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("RUN \"myprogram.bas\"", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST(CmdLineTest, Parse_GivenProgramArgumentWithFlags) {
    int argc = 8;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "myprogram.bas";
    argv[2] = "--foo";
    argv[3] = "-i";
    argv[4] = "-v";
    argv[5] = "--interactive";
    argv[6] = "--version";
    argv[7] = "\"wom bat\"";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(0, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("RUN \"myprogram.bas\", \"--foo -i -v --interactive --version \" + Chr$(34) + \"wom bat\" + Chr$(34)", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST(CmdLineTest, Parse_GivenDirectoryFlag) {
    int argc = 3;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "-d";
    argv[2] = "some/directory";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("some/directory", args.directory);

    argv[1] = "--directory";
    argv[2] = "foo/bar/wom bat";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("foo/bar/wom bat", args.directory);

    argc = 2;
    argv[1] = "-d=some/directory";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("some/directory", args.directory);

    argv[1] = "--directory=\"foo/bar/wom bat\"";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("foo/bar/wom bat", args.directory);
}

TEST(CmdLineTest, Parse_GivenUnknownFlag) {
    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "--unknown";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kInvalidCommandLine, cmdline_parse(argc, argv, &args));
}

TEST(CmdLineTest, Parse_GivenCommandLineMaxLength) {
    char input[INPBUF_SIZE] = { 0 };
    cstring_cat(input, "\"", INPBUF_SIZE);
    for (int i = 0; i < INPBUF_SIZE - 7; ++i) cstring_cat(input, "A", INPBUF_SIZE);
    cstring_cat(input, "\"", INPBUF_SIZE);

    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = input;
    CmdLineArgs args = { 0 };

    char expected[INPBUF_SIZE] = { 0 };
    cstring_cat(expected, "RUN ", INPBUF_SIZE);
    cstring_cat(expected, input, INPBUF_SIZE);

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_STREQ(expected, args.run_cmd);
}

TEST(CmdLineTest, Parse_GivenCommandLineTooLong) {
    char input[INPBUF_SIZE] = { 0 };
    cstring_cat(input, "\"", INPBUF_SIZE);
    for (int i = 0; i < INPBUF_SIZE - 6; ++i) cstring_cat(input, "A", INPBUF_SIZE);
    cstring_cat(input, "\"", INPBUF_SIZE);

    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = input;
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kStringTooLong, cmdline_parse(argc, argv, &args));
}
