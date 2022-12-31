/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../../Hardware_Includes.h"
#include "../../core/Commands.h"
#include "../../core/MMBasic.h"
#include "../../core/vartbl.h"

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

extern "C" {

void cmd_autosave() { }
void cmd_call() { }
void cmd_case() { }
void cmd_cfunction() { }
void cmd_chdir() { }
void cmd_clear() { }
void cmd_close() { }
void cmd_cls() { }
void cmd_console() { }
void cmd_const() { }
void cmd_continue() { }
void cmd_copy() { }
void cmd_cursor() { }
void cmd_dim() { }
void cmd_do() { }
void cmd_dummy() { }
void cmd_edit() { }
void cmd_else() { }
void cmd_end() { }
void cmd_endfun() { }
void cmd_erase() { }
void cmd_error() { }
void cmd_execute() { }
void cmd_exit() { }
void cmd_exitfor() { }
void cmd_files() { }
void cmd_for() { }
void cmd_gosub() { }
void cmd_goto() { }
void cmd_if() { }
void cmd_inc() { }
void cmd_input() { }
void cmd_ireturn() { }
void cmd_kill() { }
void cmd_let() { }
void cmd_lineinput() { }
void cmd_list() { }
void cmd_load() { }
void cmd_loop() { }
void cmd_longstring() { }
void cmd_math() { }
void cmd_memory() { }
void cmd_mid() { }
void cmd_mkdir() { }
void cmd_new() { }
void cmd_next() { }
void cmd_null() { }
void cmd_on() { }
void cmd_open() { }
void cmd_option() { }
void cmd_pause() { }
void cmd_poke() { }
void cmd_print() { }
void cmd_quit() { }
void cmd_randomize() { }
void cmd_read() { }
void cmd_rename() { }
void cmd_restore() { }
void cmd_return() { }
void cmd_rmdir() { }
void cmd_seek() { }
void cmd_select() { }
void cmd_settick() { }
void cmd_settitle() { }
void cmd_sort() { }
void cmd_subfun() { }
void cmd_system() { }
void cmd_timer() { }
void cmd_trace() { }
void cmd_troff() { }
void cmd_tron() { }
void cmd_xmodem() { }
void fun_abs() { }
void fun_acos() { }
void fun_asc() { }
void fun_asin() { }
void fun_at() { }
void fun_atn() { }
void fun_bin() { }
void fun_bound() { }
void fun_call() { }
void fun_choice() { }
void fun_chr() { }
void fun_cint() { }
void fun_cos() { }
void fun_cwd() { }
void fun_date() { }
void fun_datetime() { }
void fun_day() { }
void fun_deg() { }
void fun_dir() { }
void fun_eof() { }
void fun_epoch() { }
void fun_errmsg() { }
void fun_errno() { }
void fun_eval() { }
void fun_exp() { }
void fun_field() { }
void fun_fix() { }
void fun_format() { }
void fun_hex() { }
void fun_hres() { }
void fun_inkey() { }
void fun_inputstr() { }
void fun_instr() { }
void fun_int() { }
void fun_json() { }
void fun_lcase() { }
void fun_lcompare() { }
void fun_left() { }
void fun_len() { }
void fun_lgetbyte() { }
void fun_lgetstr() { }
void fun_linstr() { }
void fun_llen() { }
void fun_loc() { }
void fun_lof() { }
void fun_log() { }
void fun_math() { }
void fun_max() { }
void fun_mid() { }
void fun_min() { }
void fun_mmcmdline() { }
void fun_mmdevice() { }
void fun_mminfo() { }
void fun_oct() { }
void fun_peek() { }
void fun_pi() { }
void fun_pos() { }
void fun_rad() { }
void fun_rgb() { }
void fun_right() { }
void fun_rnd() { }
void fun_sgn() { }
void fun_sin() { }
void fun_space() { }
void fun_sqr() { }
void fun_str() { }
void fun_string() { }
void fun_tab() { }
void fun_tan() { }
void fun_time() { }
void fun_timer() { }
void fun_ucase() { }
void fun_val() { }
void fun_version() { }
void fun_vres() { }
void op_and() { }
void op_div() { }
void op_divint() { }
void op_equal() { }
void op_exp() { }
void op_gt() { }
void op_gte() { }
void op_inv() { }
void op_invalid() { }
void op_lt() { }
void op_lte() { }
void op_mod() { }
void op_mul() { }
void op_ne() { }
void op_not() { }
void op_or() { }
void op_shiftleft() { }
void op_shiftright() { }
void op_subtract() { }
void op_xor() { }

} // extern "C"
