/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../../Hardware_Includes.h"
#include "../MMBasic.h"
#include "../Commands.h"
#include "../FunTable.h"
#include "../mmbasic_core_xtra.h"

char error_msg[256];

void CheckAbort(void) { }

void cmd_read_clear_cache()  { }

/** Write a NULL terminated stirng to the console. */
void console_puts(const char *s) {
}

void error_init(ErrorState *error_state) { }

void error_throw_ex(MmResult error, const char *msg, ...) {
    strcpy(error_msg, msg);
}

void error_throw_legacy(const char *msg, ...) {
    strcpy(error_msg, msg);
}

void file_close_all(void) { }

bool interrupt_check(void) {
    return false;
}

void interrupt_clear(void) { }

// Declared in MMBasic.c
extern struct s_funtbl funtbl[MAXSUBFUN];

// Declared in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
char **FontTable;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Options mmb_options;
ErrorState mmb_normal_error_state;

// Declared in "Commands.c"
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

} // extern "C"

class MmBasicCoreTest : public ::testing::Test {

protected:

    void SetUp() override {
        error_msg[0] = '\0';
    }

    void TearDown() override {
    }

};

TEST_F(MmBasicCoreTest, FunctionTableHash) {
    char program[256];
    char name[MAXVARLEN + 1];
    HASH_TYPE hash;

    sprintf(program, "foo");
    int actual = mmb_function_table_hash(program, name, &hash);

    EXPECT_EQ(0, actual);
    EXPECT_STREQ("FOO", name);
    EXPECT_EQ(503, hash);

    sprintf(program, "bar");
    actual = mmb_function_table_hash(program, name, &hash);

    EXPECT_EQ(0, actual);
    EXPECT_STREQ("BAR", name);
    EXPECT_EQ(122, hash);
}

TEST_F(MmBasicCoreTest, FunctionTableHash_GivenMaximumLengthName) {
    char program[256];
    char name[MAXVARLEN + 1];
    HASH_TYPE hash;

    sprintf(program, "_32_character_name_9012345678901");
    int actual = mmb_function_table_hash(program, name, &hash);

    EXPECT_EQ(0, actual);
    EXPECT_STREQ("_32_CHARACTER_NAME_9012345678901", name);
    EXPECT_EQ(479, hash);
}

TEST_F(MmBasicCoreTest, FunctionTableHash_GivenNameTooLong) {
    char program[256];
    char name[MAXVARLEN + 1];
    HASH_TYPE hash;

    sprintf(program, "_33_character_name_90123456789012");
    int actual = mmb_function_table_hash(program, name, &hash);

    EXPECT_EQ(-1, actual);
    EXPECT_STREQ("_33_CHARACTER_NAME_9012345678901", name);
    EXPECT_EQ(24, hash);
}

TEST_F(MmBasicCoreTest, FunctionTablePrepare) {
    char program[256];
    sprintf(program,
            "# foo\n"
            "#\n"
            "# bar\n"
            "#\n");
    subfun[0] = program;
    subfun[1] = program + 8;
    subfun[2] = NULL;

    mmb_function_table_prepare(true);

    EXPECT_EQ(2, mmb_function_table_size());
    EXPECT_STREQ("", funtbl[0].name);
    EXPECT_EQ(0, funtbl[0].index);
    EXPECT_STREQ("BAR", funtbl[122].name);
    EXPECT_EQ(1, funtbl[122].index);
    EXPECT_STREQ("FOO", funtbl[503].name);
    EXPECT_EQ(0, funtbl[503].index);
}

TEST_F(MmBasicCoreTest, FunctionTablePrepare_GivenNameTooLong) {
    char program[256];
    sprintf(program,
            "# name_32_characters_xxxxxxxxxxxxx\n"
            "#\n"
            "# name_33_characters_zzzzzzzzzzzzzz\n"
            "#\n");
    subfun[0] = program;
    subfun[1] = program + 37;
    subfun[2] = NULL;

    mmb_function_table_prepare(true);

    EXPECT_EQ(2, mmb_function_table_size());
    EXPECT_STREQ("", funtbl[0].name);
    EXPECT_EQ(0, funtbl[0].index);
    EXPECT_STREQ("NAME_32_CHARACTERS_XXXXXXXXXXXXX", funtbl[308].name);
    EXPECT_EQ(0, funtbl[308].index);
    EXPECT_STREQ("NAME_33_CHARACTERS_ZZZZZZZZZZZZZ", funtbl[431].name);
    EXPECT_EQ(1, funtbl[431].index);
    EXPECT_STREQ("SUB/FUNCTION name too long", error_msg);
}

TEST_F(MmBasicCoreTest, FunctionTablePrepare_GivenDuplicateName) {
    char program[256];
    sprintf(program,
            "# foo\n"
            "#\n"
            "# fOo\n"
            "#\n");
    subfun[0] = program;
    subfun[1] = program + 8;
    subfun[2] = NULL;

    mmb_function_table_prepare(true);

    EXPECT_EQ(1, mmb_function_table_size());
    EXPECT_STREQ("", funtbl[0].name);
    EXPECT_EQ(0, funtbl[0].index);
    EXPECT_STREQ("FOO", funtbl[503].name);
    EXPECT_EQ(0, funtbl[503].index);
    EXPECT_STREQ("Duplicate SUB/FUNCTION declaration", error_msg);
}

TEST_F(MmBasicCoreTest, FunctionTableFind) {
    char program[256];
    sprintf(program,
            "# foo\n"
            "#\n"
            "# bar\n"
            "#\n");
    subfun[0] = program;
    subfun[1] = program + 8;
    subfun[2] = NULL;

    mmb_function_table_prepare(true);

    EXPECT_EQ(0,  mmb_function_table_find("Foo("));
    EXPECT_EQ(1,  mmb_function_table_find("BAr "));
    EXPECT_EQ(-1, mmb_function_table_find("WOMBAT("));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(-1, mmb_function_table_find("name_33_characters_zzzzzzzzzzzzzz"));
    EXPECT_STREQ("SUB/FUNCTION name too long", error_msg);
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
void cmd_run() { }
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
void op_add() { }
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
