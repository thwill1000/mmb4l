/*
 * Copyright (c) 2021-2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../cmdline.h"
#include "../cstring.h"
#include "../features.h"
#include "../memory.h"
#include "../parse.h"
#include "../options.h"

// Defined in "main.c"
Features mmb_features;
Options mmb_options;

// Defined in "common/gpio.c"
MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num) { return kOk; }

// Defined in "common/graphics.c"
MmSurface graphics_surfaces[GRAPHICS_MAX_SURFACES] = {};

// Defined in "common/mmgetchar.c"
int MMgetchar(void) { return -1; }

// Defined in "core/commandtbl.c"
CommandToken cmdFUN = 0x0;
CommandToken cmdSUB = 0x0;

// Defined in "core/MMBasic.c"
int LocalIndex = 0;
int VarIndex;
char *getCstring(const char *p) { return NULL; }
void *findvar(const char *p, int action)  { return NULL; }
const char *skipexpression(const char *p) { return NULL; }
MMINTEGER getinteger(char *p) { return 0; }
MMINTEGER getint(char *p, MMINTEGER min, MMINTEGER max) { return 0; }

// Defined in "core/tokentbl.c"
char tokenAS = 0x0;

}

class CmdLineTest : public ::testing::Test {

protected:

    void SetUp() override {
        ASSERT_EQ(kOk, memory_init());
    }

    void TearDown() override {
        ASSERT_EQ(kOk, memory_term());
    }
};

TEST_F(CmdLineTest, Parse_GivenNoAdditionalArguments) {
    int argc = 1;
    const char *argv[10];
    argv[0] = "mmbasic";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST_F(CmdLineTest, Parse_GivenHelpFlag) {
    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "-h";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.help);
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);

    args.show_prompt = 0;
    argv[1] = "--help";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.help);
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST_F(CmdLineTest, Parse_GivenInteractiveFlag) {
    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "-i";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);

    args.show_prompt = 0;
    argv[1] = "--interactive";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST_F(CmdLineTest, Parse_GivenVersionFlag) {
    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "-v";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(1, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);

    args.version = 0;
    argv[1] = "--version";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(1, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST_F(CmdLineTest, Parse_GivenInteractiveAndVersionFlags) {
    int argc = 3;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "--interactive";
    argv[2] = "-v";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(1, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST_F(CmdLineTest, Parse_GivenLogFlag) {
    int argc = 3;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "-l";
    argv[2] = "Debug";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_STREQ("Debug", args.log);
    EXPECT_EQ(1, args.show_prompt);

    argv[1] = "--log";
    argv[2] = "warning";
    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_STREQ("warning", args.log);

    argc = 2;
    argv[1] = "-l=Info";
    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_STREQ("Info", args.log);

    argv[1] = "--log=Error";
    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_STREQ("Error", args.log);
}

TEST_F(CmdLineTest, Parse_GivenLogFlagFatal_Fails) {
    int argc = 3;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "--log";
    argv[2] = "Fatal";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kInvalidValue, cmdline_parse(argc, argv, &args));
}

TEST_F(CmdLineTest, Parse_GivenLogFlagInvalidValue_Fails) {
    int argc = 3;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "--log";
    argv[2] = "Trace";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kInvalidValue, cmdline_parse(argc, argv, &args));

    argc = 2;
    argv[1] = "-l=verbose";
    EXPECT_EQ(kInvalidValue, cmdline_parse(argc, argv, &args));
}

TEST_F(CmdLineTest, Parse_GivenLogFlagMissingValue_Fails) {
    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "-l";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kInvalidCommandLine, cmdline_parse(argc, argv, &args));

    argv[1] = "--log";
    EXPECT_EQ(kInvalidCommandLine, cmdline_parse(argc, argv, &args));
}

TEST_F(CmdLineTest, Parse_GivenProgramArgument) {
    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "myprogram.bas";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(0, args.show_prompt);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("RUN \"myprogram.bas\"", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST_F(CmdLineTest, Parse_GivenProgramArgumentWithFlags) {
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
    EXPECT_EQ(0, args.show_prompt);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("RUN \"myprogram.bas\", \"--foo -i -v --interactive --version \" + Chr$(34) + \"wom bat\" + Chr$(34)", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST_F(CmdLineTest, Parse_GivenDirectoryFlag) {
    int argc = 3;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "-d";
    argv[2] = "some/directory";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("some/directory", args.directory);

    argv[1] = "--directory";
    argv[2] = "foo/bar/wom bat";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("foo/bar/wom bat", args.directory);

    argc = 2;
    argv[1] = "-d=some/directory";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("some/directory", args.directory);

    argv[1] = "--directory=\"foo/bar/wom bat\"";

    EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.show_prompt);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("foo/bar/wom bat", args.directory);
}

TEST_F(CmdLineTest, Parse_GivenUnknownFlag) {
    int argc = 2;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "--unknown";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(kInvalidCommandLine, cmdline_parse(argc, argv, &args));
}

TEST_F(CmdLineTest, Parse_GivenCommandLineMaxLength) {
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

TEST_F(CmdLineTest, Parse_GivenCommandLineTooLong) {
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

static int count_args(const char **argv, size_t size) {
    int count = 0;
    for (size_t i = 0; i < size; ++i) {
        if (!argv[i] || argv[i][0] == '\0') break;
        count++;
    }
    return count;
}

TEST_F(CmdLineTest, Parse_GivenSimulateKnownDevice_Succeeds) {
    typedef struct {
        const char *args[4];
        OptionsSimulate expected;
    } TestData;

    const TestData tests[] = {
        { { "mmbasic", "", "" }, kSimulateUnspecified },
        { { "mmbasic", "--simulate", "CMM2" }, kSimulateCmm2 },
        { { "mmbasic", "-s", "picomitevga" }, kSimulatePicomiteVga },
        { { "mmbasic", "--simulate=GameMite", "" }, kSimulateGamemite },
        { { "mmbasic", "-s=PicoCalc", "" }, kSimulatePicocalc },
        { { "mmbasic", "picomitevgausb", "" }, kSimulatePicomiteVgaUsb },
        { { "mmbasic", "PicoMiteHdmi", "" }, kSimulatePicomiteHdmi },
        { { NULL, NULL, NULL }, kSimulateUnspecified }
    };

    for (const TestData *t = tests; t->args[0]; ++t) {
        const char *argv[10] = { 0 };
        argv[0] = t->args[0];
        argv[1] = t->args[1];
        argv[2] = t->args[2];
        const int argc = count_args(argv, 10);
        CmdLineArgs args = { 0 };

        EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
        EXPECT_EQ(0, args.help);
        EXPECT_EQ(1, args.show_prompt);
        EXPECT_EQ(0, args.version);
        EXPECT_EQ(t->expected, args.simulate);
        EXPECT_STREQ("", args.run_cmd);
        EXPECT_STREQ("", args.directory);
    }
}

TEST_F(CmdLineTest, Parse_GivenSimulateKnownDevice_AndProgramArgument_Succeeds) {
    typedef struct {
        const char *args[4];
        OptionsSimulate expected;
    } TestData;

    const TestData tests[] = {
        { { "mmbasic", "", "" }, kSimulateUnspecified },
        { { "mmbasic", "--simulate", "CMM2" }, kSimulateCmm2 },
        { { "mmbasic", "-s", "picomitevga" }, kSimulatePicomiteVga },
        { { "mmbasic", "--simulate=GameMite", "" }, kSimulateGamemite },
        { { "mmbasic", "-s=PicoCalc", "" }, kSimulatePicocalc },
        { { "mmbasic", "picomitevgausb", "" }, kSimulatePicomiteVgaUsb },
        { { "mmbasic", "PicoMiteHdmi", "" }, kSimulatePicomiteHdmi },
        { { NULL, NULL, NULL }, kSimulateUnspecified }
    };

    for (const TestData *t = tests; t->args[0]; ++t) {
        const char *argv[10] = { 0 };
        argv[0] = t->args[0];
        argv[1] = t->args[1];
        argv[2] = t->args[2];
        const int argc = count_args(argv, 10) + 1;
        argv[argc - 1] = "myprogram.bas";
        CmdLineArgs args = { 0 };

        EXPECT_EQ(kOk, cmdline_parse(argc, argv, &args));
        EXPECT_EQ(0, args.help);
        EXPECT_EQ(0, args.show_prompt);
        EXPECT_EQ(0, args.version);
        EXPECT_EQ(t->expected, args.simulate);
        EXPECT_STREQ("RUN \"myprogram.bas\"", args.run_cmd);
        EXPECT_STREQ("", args.directory);
    }
}

TEST_F(CmdLineTest, Parse_GivenSimulateUnknownDevice_Fails) {
    typedef struct {
        const char *args[3];
        MmResult expected;
    } TestData;

    const TestData tests[] = {
        { { "mmbasic", "--simulate", "foo" }, kUnknownDevice },
        { { "mmbasic", "-s", "" }, kInvalidCommandLine },
        { { "mmbasic", "--simulate=bar", "" }, kUnknownDevice },
        { { "mmbasic", "-s=", "" }, kUnknownDevice },
    };

    for (size_t i = 0; i < sizeof(tests) / sizeof(TestData); ++i) {
        const char *argv[10] = {};
        argv[0] = tests[i].args[0];
        argv[1] = tests[i].args[1];
        argv[2] = tests[i].args[2];
        const int argc = count_args(argv, 10);
        CmdLineArgs args = { 0 };

        EXPECT_EQ(tests[i].expected, cmdline_parse(argc, argv, &args));
        EXPECT_EQ(kSimulateUnspecified, args.simulate);
    }
}
