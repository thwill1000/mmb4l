/*
 * Copyright (c) 2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../../common/features.h"
#include "../../common/memory.h"
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
Features mmb_features;
Options mmb_options;
ErrorState mmb_normal_error_state;

void CheckAbort(void) { }

// Defined in "commands/cmd_read.c"
void cmd_read_clear_cache()  { }

// Defined in "commands/cmd_run.c"
extern char cmd_run_args[STRINGSIZE];

// Defined in "commands/cmd_run.c"
MmResult cmd_run_parse_args(const char *p, char *filename, char *run_args);

// Defined in "common/gpio.c"
MmResult gpio_term() { return kOk; }
MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num) { return kOk; }

// Defined in "common/program.c"
char CurrentFile[STRINGSIZE];
MmResult program_load_file(char *filename) { return kError; }

// Defined in "common/mmgetchar.c"
int MMgetchar(void) { return -1; }

// Defined in "core/Commands.c"
char DimUsed;
int doindex;
struct s_dostack dostack[MAXDOLOOPS];
const char *errorstack[MAXGOSUB];
int forindex;
struct s_forstack forstack[MAXFORLOOPS + 1];
int gosubindex;
const char *gosubstack[MAXGOSUB];

} // extern "C"

class CmdDoTest : public ::testing::Test {

protected:

    void SetUp() override {
        vartbl_init_called = false;
        ASSERT_EQ(kOk, memory_init());
        ASSERT_EQ(kOk, InitBasic());
        error_msg[0] = '\0';
        ClearProgMemory();
    }

    void TearDown() override {
        ASSERT_EQ(kOk, memory_term());
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
    ASSERT_EQ(kOk, PrepareProgram(true));
    cmdtoken = cmdDO;
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
