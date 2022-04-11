#include <gtest/gtest.h>

extern "C" {

#include "../cmdline.h"

}

TEST(CmdLineTest, Parse_GivenNoAdditionalArguments) {
    int argc = 1;
    const char *argv[10];
    argv[0] = "mmbasic";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
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

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);

    args.interactive = 0;
    argv[1] = "--help";

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
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

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);

    args.interactive = 0;
    argv[1] = "--interactive";

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
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

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(1, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("", args.directory);

    args.version = 0;
    argv[1] = "--version";

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
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

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
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

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
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

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(0, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("RUN \"myprogram.bas\", --foo -i -v --interactive --version \"wom bat\"", args.run_cmd);
    EXPECT_STREQ("", args.directory);
}

TEST(CmdLineTest, Parse_GivenDirectoryFlag) {
    int argc = 3;
    const char *argv[10];
    argv[0] = "mmbasic";
    argv[1] = "-d";
    argv[2] = "some/directory";
    CmdLineArgs args = { 0 };

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(0, args.help);
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("some/directory", args.directory);

    argv[1] = "--directory";
    argv[2] = "foo/bar/wom bat";

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("foo/bar/wom bat", args.directory);

    argc = 2;
    argv[1] = "-d=some/directory";

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("some/directory", args.directory);

    argv[1] = "--directory=\"foo/bar/wom bat\"";

    EXPECT_EQ(0, cmdline_parse(argc, argv, &args));
    EXPECT_EQ(1, args.interactive);
    EXPECT_EQ(0, args.version);
    EXPECT_STREQ("", args.run_cmd);
    EXPECT_STREQ("foo/bar/wom bat", args.directory);
}
