/*
 * Copyright (c) 2022-2023 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../../Hardware_Includes.h"
#include "../../core/Commands.h"
#include "../../core/MMBasic.h"
#include "../../core/vartbl.h"
#define DO_NOT_STUB_CMD_RUN
#include "../../core/gtest/command_stubs.h"
#define DO_NOT_STUB_FUN_MMCMDLINE
#include "../../core/gtest/function_stubs.h"
#define DO_NOT_STUB_OP_ADD
#include "../../core/gtest/operation_stubs.h"

char error_msg[256];

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
char **FontTable;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Options mmb_options;
ErrorState mmb_normal_error_state;
int WatchdogSet;
int IgnorePIN;

void CheckAbort(void) { }

// Defined in "commands/cmd_read.c"
void cmd_read_clear_cache()  { }

// Defined in "commands/cmd_run.c"
extern char cmd_run_args[STRINGSIZE];

// Defined in "commands/cmd_run.c"
MmResult cmd_run_parse_args(const char *p, char *filename, char *run_args);

// Defined in "common/console.c"
void console_puts(const char *s) { }

// Defined in "common/error.c"
void error_init(ErrorState *error_state) { }

void error_throw(MmResult error) {
    error_throw_ex(error, mmresult_to_string(error));
}

void error_throw_ex(MmResult error, const char *msg, ...) {
    strcpy(error_msg, msg);
}

void error_throw_legacy(const char *msg, ...) {
    strcpy(error_msg, msg);
}

// Defined in "common/file.c"
void file_close_all(void) { }

// Defined in "common/interrupt.c"
bool interrupt_check(void) { return false; }
void interrupt_clear(void) { }

// Defined in "common/program.c"
char CurrentFile[STRINGSIZE];
int program_load_file(char *filename) { return -1; }

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

// Defined in "core/Operators.c"
void op_add(void) {
    if (targ & T_NBR) {
        fret = farg1 + farg2;
    } else if(targ & T_INT) {
        iret = iarg1 + iarg2;
    } else {
        if(*sarg1 + *sarg2 > MAXSTRLEN) error("String too long");
        sret = (char *) GetTempStrMemory();
        Mstrcpy(sret, sarg1);
        Mstrcat(sret, sarg2);
    }
}

} // extern "C"

class CmdRunTest : public ::testing::Test {

protected:

    char m_filename[STRINGSIZE];
    char m_run_args[STRINGSIZE];

    void SetUp() override {
        *m_filename = '\0';
        *m_run_args = '\0';
        vartbl_init_called = false;
        InitBasic();
        ClearRuntime();
    }

    void TearDown() override {
    }

};

TEST_F(CmdRunTest, ParseArgs_GivenEmptyString) {
    strcpy(inpbuf, "RUN");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("", m_filename);
    EXPECT_STREQ("", m_run_args);
}

TEST_F(CmdRunTest, ParseArgs_GivenJustAComma) {
    strcpy(inpbuf, "RUN ,");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("", m_filename);
    EXPECT_STREQ("", m_run_args);
}

TEST_F(CmdRunTest, ParseArgs_GivenFilename) {
    strcpy(inpbuf, "RUN \"foo\"");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_STREQ("", m_run_args);
}

TEST_F(CmdRunTest, ParseArgs_GivenFilenameWithTrailingComma) {
    strcpy(inpbuf, "RUN \"foo\",");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_STREQ("", m_run_args);
}

TEST_F(CmdRunTest, ParseArgs_GivenCmdArgs) {
    strcpy(inpbuf, "RUN , \"foo\"");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("", m_filename);
    EXPECT_STREQ("foo", m_run_args);
}

TEST_F(CmdRunTest, ParseArgs_GivenFilenameAndCmdArgs) {
    strcpy(inpbuf, "RUN \"foo\", \"bar\"");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_STREQ("bar", m_run_args);
}

TEST_F(CmdRunTest, ParseArgs_GivenStringExpressions) {
    strcpy(inpbuf, "RUN \"foo\" + \"bar\", \"wom\" + \"bat\"");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foobar", m_filename);
    EXPECT_STREQ("wombat", m_run_args);
}

TEST_F(CmdRunTest, ParseArgs_GivenNewCmdArgsDependOnExistingMmCmdLine) {
    strcpy(cmd_run_args, "wom");
    strcpy(inpbuf, "RUN \"foobar\", Mm.CmdLine$ + \"bat\"");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, cmd_run_args));
    EXPECT_STREQ("foobar", m_filename);
    EXPECT_STREQ("wombat", cmd_run_args);
}

TEST_F(CmdRunTest, ParseArgs_GivenLegacyArgs) {
    // Legacy compatibility code should be invoked when args contain hyphen.
    strcpy(inpbuf, "RUN \"foo\", -wom bat  ");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_STREQ("-wom bat", m_run_args);

    // Legacy compatibility code should be invoked when filename contains
    // "menu/menu.bas" and args contains "MENU_".
    strcpy(inpbuf, "RUN \"my/menu/menu.bas\", MENU_ITEM  ");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("my/menu/menu.bas", m_filename);
    EXPECT_STREQ("menu_item", m_run_args);

    // Legacy compatibility code should not insert spaces between consecutive
    // hyphen tokens.
    strcpy(inpbuf, "RUN \"foo\", --wom bat  ");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_STREQ("--wom bat", m_run_args);

    // Legacy compatibility code should not insert space between alphanumeric
    // and '=' token.
    strcpy(inpbuf, "RUN \"foo\", -wom=bat  ");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_STREQ("-wom=bat", m_run_args);

    // Legacy compatibility code should not insert space between alphanumeric
    // and '-' token.
    strcpy(inpbuf, "RUN \"foo\", wom-bat=2");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_STREQ("wom-bat=2", m_run_args);

    // Legacy compatibility code should convert to lower-case.
    strcpy(inpbuf, "RUN \"foo\", -WOM BAT  ");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_STREQ("-wom bat", m_run_args);

    // Legacy compatibility code should insert spaces between consecutive
    // tokens.
    strcpy(inpbuf, "RUN \"foo\", /-=-/=-  ");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_STREQ("/ - = - / = -", m_run_args);

    // Legacy compatibility code should compresses consecutive spaces.
    strcpy(inpbuf, "RUN \"foo\", wom  -bat  ");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_STREQ("wom -bat", m_run_args);

    // Legacy compatibility code should not mangle quoted sections.
    strcpy(inpbuf, "RUN \"foo\", -wom \"/-=-/=- -WOM BAT  \" bat  ");
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_STREQ("-wom \"/-=-/=- -WOM BAT  \" bat", m_run_args);
}

TEST_F(CmdRunTest, ParseArgs_DoesNotOverrunBuffer) {
    memset(inpbuf, '+', 255);
    memcpy(inpbuf, "RUN \"foo\", -bar", 15);
    inpbuf[255] = '\0';
    tokenise(1);
    EXPECT_EQ(kOk, cmd_run_parse_args(tknbuf + 1, m_filename, m_run_args));
    EXPECT_STREQ("foo", m_filename);
    EXPECT_EQ(255, strlen(m_run_args));
    EXPECT_STREQ(
            "-bar + + + + + + + + + + + + + + + + + + + + + + + + + + + "
            "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + "
            "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + "
            "+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + "
            "+ + + + + + + + ",
            m_run_args);
}
