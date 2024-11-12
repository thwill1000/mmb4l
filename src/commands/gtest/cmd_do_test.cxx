/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../../Hardware_Includes.h"
#include "../../common/utility.h"
#include "../../common/gtest/test_helper.h"
#include "../../common/gtest/stubs/error_stubs.h"
#include "../../core/Commands.h"
#include "../../core/MMBasic.h"
#include "../../core/commandtbl.h"
#include "../../core/vartbl.h"
#define DO_NOT_STUB_CMD_DO
#include "../../core/gtest/command_stubs.h"
#include "../../core/gtest/function_stubs.h"
#include "../../core/gtest/operation_stubs.h"

void cmd_do(void);

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
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

// Defined in "common/file.c"
void file_close_all(void) { }

// Defined in "common/gpio.c"
void gpio_term() { }
MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num) { return kOk; }

// Defined in "common/path.c"
MmResult path_munge(const char *original_path, char *new_path, size_t sz) { return kOk; }

// Defined in "common/program.c"
char CurrentFile[STRINGSIZE];
MmResult program_load_file(char *filename) { return kError; }

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

} // extern "C"

class CmdDoTest : public ::testing::Test {

protected:

    void SetUp() override {
        vartbl_init_called = false;
        InitBasic();
        ClearRuntime();
        error_msg[0] = '\0';
        ClearProgMemory();
    }

    void TearDown() override {
    }

    void ClearProgMemory() {
        clear_prog_memory();
    }

    void TokeniseAndAppend(const char* untokenised) {
        tokenise_and_append(untokenised);
        EXPECT_STREQ("", error_msg);
    }
};

TEST_F(CmdDoTest, GivenOneLineDoLoop) {
    TokeniseAndAppend("Do : Print : Loop");
    PrepareProgram(1);
    cmdtoken = GetCommandValue("Do");
    nextstmt = cmdline = ProgMemory + 1 + sizeof(CommandToken);
    skipspace(cmdline);
    skipelement(nextstmt);

    cmd_do();

    EXPECT_EQ(1, doindex);
    EXPECT_EQ(NULL, dostack[0].evalptr);
    EXPECT_EQ(ProgMemory + 3, dostack[0].doptr);
    EXPECT_EQ(0, dostack[0].level);
    EXPECT_EQ(ProgMemory + 9, dostack[0].loopptr);
}
