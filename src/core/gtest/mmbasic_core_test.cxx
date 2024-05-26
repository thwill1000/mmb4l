/*
 * Copyright (c) 2022-2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../../Hardware_Includes.h"
#include "../Commands.h"
#include "../commandtbl.h"
#include "../funtbl.h"
#include "../tokentbl.h"
#include "../vartbl.h"
#include "../MMBasic.h"
#include "../../common/graphics.h"
#include "../../common/parse.h"
#include "../../common/program.h"
#include "../../common/gtest/test_helper.h"
#include "command_stubs.h"
#include "function_stubs.h"
#include "operation_stubs.h"

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Options mmb_options;
ErrorState mmb_normal_error_state;
uint8_t mmb_exit_code = 0;
int MMgetchar(void) { return 0; }
void MMgetline(int filenbr, char *p) { }

// Defined in "commands/cmd_read.c"
void cmd_read_clear_cache()  { }

// Defined in "common/console.c"
int console_kbhit(void) { return 0; }
char console_putc(char c) { return c; }
void console_puts(const char *s) { }
void console_set_title(const char *title) { }
size_t console_write(const char *buf, size_t sz) { return 0; }

// Defined in "common/gpio.c"
void gpio_term() { }
MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num) { return kOk; }

// Defined in "common/graphics.c"
MmSurface graphics_surfaces[GRAPHICS_MAX_SURFACES];
MmResult graphics_term(void) { return kOk; }
MmResult graphics_surface_destroy(MmSurface *surface) { return kOk; }

// Defined in "common/interrupt.c"
bool interrupt_check() { return true; }
void interrupt_clear() { }
void interrupt_disable_serial_rx(int fnbr) { }
void interrupt_enable_serial_rx(int fnbr, int64_t count, const char *interrupt_addr) { }

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
void CheckAbort(void) { }
void ListNewLine(int *ListCnt, int all) { }

} // extern "C"

#define MAX_LENGTH_NAME  "_32_character_name_9012345678901"
#define TOO_LONG_NAME    "_33_character_name_90123456789012"

class MmBasicCoreTest : public ::testing::Test {

protected:

    void SetUp() override {
        vartbl_init_called = false;
        InitBasic();
        ClearRuntime();
        funtbl_clear(); // TODO: remove this
        error_msg[0] = '\0';
        m_program[0] = '\0';
        VarIndex = 999;
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

    char m_program[256];
};

TEST_F(MmBasicCoreTest, FindVar_GivenNoExplicitType) {
    sprintf(m_program, "foo = 1");
    void *actual = findvar(m_program, V_FIND);

    EXPECT_EQ(&vartbl[0].val, actual);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, vartbl[0].level);
    EXPECT_STREQ("FOO", vartbl[0].name);
    EXPECT_EQ(T_NBR, vartbl[0].type);
    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenTypeSuffix) {
    // Make an INTEGER variable.
    sprintf(m_program, "my_int%% = 1");
    void *actual = findvar(m_program, V_FIND);

    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("MY_INT", vartbl[0].name);
    EXPECT_EQ(0, vartbl[0].level);
    EXPECT_EQ(T_INT, vartbl[0].type);
    EXPECT_EQ(&vartbl[0].val, actual);
    EXPECT_STREQ("", error_msg);

    // Make a FLOAT variable.
    sprintf(m_program, "my_float! = 1");
    actual = findvar(m_program, T_NBR);

    EXPECT_EQ(1, VarIndex);
    EXPECT_STREQ("MY_FLOAT", vartbl[1].name);
    EXPECT_EQ(0, vartbl[1].level);
    EXPECT_EQ(T_NBR, vartbl[1].type);
    EXPECT_EQ(&vartbl[1].val, actual);
    EXPECT_STREQ("", error_msg);

    // Make a STRING variable.
    sprintf(m_program, "my_string$ = \"wombat\"");
    actual = findvar(m_program, T_STR);

    EXPECT_EQ(2, VarIndex);
    EXPECT_STREQ("MY_STRING", vartbl[2].name);
    EXPECT_EQ(0, vartbl[2].level);
    EXPECT_EQ(T_STR, vartbl[2].type);
    EXPECT_EQ(vartbl[2].val.s, actual);
    EXPECT_STREQ("", error_msg);

    EXPECT_EQ(3, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenImpliedType) {
    // Make an INTEGER variable.
    sprintf(m_program, "my_int = 1");
    void *actual = findvar(m_program, T_IMPLIED | T_INT);

    EXPECT_EQ(&vartbl[0].val, actual);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, vartbl[0].level);
    EXPECT_STREQ("MY_INT", vartbl[0].name);
    EXPECT_EQ(T_IMPLIED | T_INT, vartbl[0].type);

    // Make a FLOAT variable.
    sprintf(m_program, "my_float = 1");
    actual = findvar(m_program, T_IMPLIED | T_NBR);

    EXPECT_EQ(&vartbl[1].val, actual);
    EXPECT_EQ(1, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, vartbl[1].level);
    EXPECT_STREQ("MY_FLOAT", vartbl[1].name);
    EXPECT_EQ(T_IMPLIED | T_NBR, vartbl[1].type);

    // Make a STRING variable.
    sprintf(m_program, "my_string = \"wombat\"");
    actual = findvar(m_program, T_IMPLIED | T_STR);

    EXPECT_EQ(vartbl[2].val.s, actual);
    EXPECT_EQ(2, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, vartbl[2].level);
    EXPECT_STREQ("MY_STRING", vartbl[2].name);
    EXPECT_EQ(T_IMPLIED | T_STR, vartbl[2].type);

    EXPECT_EQ(3, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenMaxNameLen) {
    sprintf(m_program, "_32_characters_long9012345678901 = 1");
    void *actual = findvar(m_program, V_FIND);

    EXPECT_EQ(&vartbl[0].val, actual);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, vartbl[0].level);
    EXPECT_EQ(T_NBR, vartbl[0].type);

    // The name is not null terminated when it is the maximum allowed length (32 chars).
    EXPECT_EQ(0, memcmp("_32_CHARACTERS_LONG9012345678901", vartbl[0].name, 32));
    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenNameTooLong) {
    sprintf(m_program, "_33_characters_long90123456789012 = 1");
    (void) findvar(m_program, V_FIND);

    EXPECT_STREQ("Variable name too long", error_msg);
    EXPECT_EQ(0, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenExists) {
    // First findvar() creates new variables.
    sprintf(m_program, "foo = 1");
    EXPECT_EQ(&vartbl[0].val, findvar(m_program, V_FIND));
    sprintf(m_program, "bar = 1");
    EXPECT_EQ(&vartbl[1].val, findvar(m_program, V_FIND));

    // Second findvar() finds them.
    sprintf(m_program, "foo = 2");
    EXPECT_EQ(&vartbl[0].val, findvar(m_program, V_FIND));
    sprintf(m_program, "bar = 1");
    EXPECT_EQ(&vartbl[1].val, findvar(m_program, V_FIND));

    EXPECT_EQ(2, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenThrowErrorIfNotExist) {
    sprintf(m_program, "foo = 1");
    (void) findvar(m_program, V_NOFIND_ERR);

    EXPECT_STREQ("Cannot find $", error_msg);
    EXPECT_EQ(0, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenReturnNullIfNotExist) {
    sprintf(m_program, "foo = 1");
    void *actual = findvar(m_program, V_NOFIND_NULL);

    EXPECT_EQ(NULL, actual);
    EXPECT_EQ(999, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenPreviouslyDeclaredWithDifferentType) {
    sprintf(m_program, "foo%% = 1");
    (void) findvar(m_program, V_FIND);

    error_msg[0] = '\0';
    sprintf(m_program, "foo!");
    (void) findvar(m_program, V_FIND);
    EXPECT_STREQ("$ already declared", error_msg);

    error_msg[0] = '\0';
    sprintf(m_program, "foo$");
    (void) findvar(m_program, V_FIND);
    EXPECT_STREQ("$ already declared", error_msg);

    error_msg[0] = '\0';
    sprintf(m_program, "foo");
    (void) findvar(m_program, V_FIND);
    EXPECT_STREQ("$ already declared", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenInvalidName) {
    sprintf(m_program, "1foo = 1");
    (void) findvar(m_program, V_FIND);

    EXPECT_STREQ("Invalid variable name", error_msg);
    EXPECT_EQ(0, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenConflictingType) {
    // Make an INTEGER variable.
    error_msg[0] = '\0';
    sprintf(m_program, "my_int! = 1");
    (void) findvar(m_program, T_IMPLIED | T_INT);

    EXPECT_STREQ("Conflicting variable type", error_msg);

    // Make a FLOAT variable.
    error_msg[0] = '\0';
    sprintf(m_program, "my_float$ = 1");
    (void) findvar(m_program, T_IMPLIED | T_NBR);

    EXPECT_STREQ("Conflicting variable type", error_msg);

    // Make a STRING variable.
    error_msg[0] = '\0';
    sprintf(m_program, "my_string%% = \"wombat\"");
    (void) findvar(m_program, T_IMPLIED | T_STR);

    EXPECT_STREQ("Conflicting variable type", error_msg);

    EXPECT_EQ(0, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimExistingVariable) {
    // Implicitly create "foo".
    sprintf(m_program, "foo = 1");
    (void) findvar(m_program, V_FIND);

    // DIM foo
    sprintf(m_program, "foo = 2");
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("$ already declared", error_msg);
    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenImplicitGlobalInSubroutine) {
    LocalIndex = 3;

    sprintf(m_program, "foo = 1");
    void *actual = findvar(m_program, V_FIND);

    EXPECT_EQ(&vartbl[0].val, actual);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, vartbl[0].level);
    EXPECT_STREQ("FOO", vartbl[0].name);
    EXPECT_EQ(T_NBR, vartbl[0].type);
    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimGlobalInSubroutine) {
    LocalIndex = 3;

    sprintf(m_program, "foo = 1");
    void *actual = findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(&vartbl[0].val, actual);
    EXPECT_EQ(0, VarIndex);
    EXPECT_EQ(0, vartbl[0].level);
    EXPECT_STREQ("FOO", vartbl[0].name);
    EXPECT_EQ(T_NBR, vartbl[0].type);
    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenExplicitCreationOfLocal) {
    LocalIndex = 3;

    sprintf(m_program, "foo = 1");
    void *actual = findvar(m_program, V_LOCAL);

    EXPECT_EQ(&vartbl[0].val, actual);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(3, vartbl[0].level);
    EXPECT_STREQ("FOO", vartbl[0].name);
    EXPECT_EQ(T_NBR, vartbl[0].type);
    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenLocal_GivenLocalExists) {
    LocalIndex = 3;
    sprintf(m_program, "foo = 1");
    (void) findvar(m_program, V_LOCAL);

    sprintf(m_program, "foo = 1");
    (void) findvar(m_program, V_LOCAL);

    EXPECT_STREQ("$ already declared", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenGlobalAndLocalOfSameName) {
    sprintf(m_program, "foo = 1");
    void *global_var = findvar(m_program, V_DIM_VAR); // Explicit creation of global.
    LocalIndex = 2;
    void *local_var = findvar(m_program, V_LOCAL); // Explicit creation of local.
    EXPECT_EQ(2, varcnt);

    // Global scope.
    error_msg[0] = '\0';
    LocalIndex = 0;
    void *actual = findvar(m_program, V_FIND);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(global_var, actual);
    EXPECT_EQ(0, VarIndex);

    // Level 1 scope.
    error_msg[0] = '\0';
    LocalIndex = 1;
    actual = findvar(m_program, V_FIND);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(global_var, actual);
    EXPECT_EQ(0, VarIndex);

    // Level 2 scope.
    error_msg[0] = '\0';
    LocalIndex = 2;
    actual = findvar(m_program, V_FIND);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(local_var, actual);
    EXPECT_EQ(1, VarIndex);

    // Level 3 scope.
    error_msg[0] = '\0';
    LocalIndex = 3;
    actual = findvar(m_program, V_FIND);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(global_var, actual);
    EXPECT_EQ(0, VarIndex);
}

TEST_F(MmBasicCoreTest, FindVar_GivenFindingUndeclaredVariable_GivenExplicitOn) {
    mmb_options.explicit_type = true;

    sprintf(m_program, "foo = 1");
    (void) findvar(m_program, V_FIND);

    EXPECT_STREQ("$ is not declared", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenCreationOfGlobal_GivenFunctionWithSameName) {
    TokeniseAndAppend("Function foo()");
    TokeniseAndAppend("End Function");
    TokeniseAndAppend("Function bar()");
    TokeniseAndAppend("End Function");
    TokeniseAndAppend("foo = 1");
    PrepareProgram(1);

    error_msg[0] = '\0';
    (void) findvar(ProgMemory + 29, V_DIM_VAR);
    EXPECT_STREQ("A function/subroutine has the same name: $", error_msg);

    // With V_FUNCT ... though actually this never happens in production with V_DIM_VAR.
    error_msg[0] = '\0';
    (void) findvar(ProgMemory + 29, V_DIM_VAR | V_FUNCT);
    EXPECT_STREQ("", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenCreationOfLocal_GivenFunctionWithSameName) {
    TokeniseAndAppend("Function foo()");
    TokeniseAndAppend("End Function");
    TokeniseAndAppend("Function bar()");
    TokeniseAndAppend("End Function");
    TokeniseAndAppend("foo = 1");
    PrepareProgram(1);

    error_msg[0] = '\0';
    LocalIndex = 3;
    (void) findvar(ProgMemory + 29, V_LOCAL);
    EXPECT_STREQ("A function/subroutine has the same name: $", error_msg);

    // With V_FUNCT.
    error_msg[0] = '\0';
    (void) findvar(ProgMemory + 29, V_LOCAL | V_FUNCT);
    EXPECT_STREQ("", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_CreatesGlobal_GivenLabelWithSameName) {
    TokeniseAndAppend("foo:");
    TokeniseAndAppend("  Print \"foo\"");
    TokeniseAndAppend("foo% = 1");
    PrepareProgram(1);

    error_msg[0] = '\0';
    void *actual = findvar(ProgMemory + 21, V_DIM_VAR);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("FOO", vartbl[0].name);
    EXPECT_EQ(0, vartbl[0].level);
    EXPECT_EQ(T_INT, vartbl[0].type);
    EXPECT_EQ(&vartbl[0].val, actual);
}

TEST_F(MmBasicCoreTest, FindVar_CreatesLocal_GivenLabelWithSameName) {
    TokeniseAndAppend("foo:");
    TokeniseAndAppend("  Print \"foo\"");
    TokeniseAndAppend("foo% = 1");
    PrepareProgram(1);

    error_msg[0] = '\0';
    LocalIndex = 3;
    void *actual = findvar(ProgMemory + 21, V_LOCAL);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("FOO", vartbl[0].name);
    EXPECT_EQ(3, vartbl[0].level);
    EXPECT_EQ(T_INT, vartbl[0].type);
    EXPECT_EQ(&vartbl[0].val, actual);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDim_ReusesEmptySlot) {
    sprintf(m_program, "foo = ...");
    (void) findvar(m_program, V_DIM_VAR);
    EXPECT_EQ(0, VarIndex);
    sprintf(m_program, "bar = ...");
    (void) findvar(m_program, V_DIM_VAR);
    EXPECT_EQ(1, VarIndex);

    EXPECT_STREQ("FOO", vartbl[0].name);
    EXPECT_STREQ("BAR", vartbl[1].name);

    vartbl_delete(0);

    sprintf(m_program, "wombat = ...");
    void *actual = findvar(m_program, V_DIM_VAR);

    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("WOMBAT", vartbl[0].name);
    EXPECT_EQ(0, vartbl[0].dims[0]);
    EXPECT_EQ(T_NBR, vartbl[0].type);
    EXPECT_EQ(&vartbl[0].val, actual);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenExistingInteger) {
    sprintf(m_program, "foo%% = ...");
    void *int_array = findvar(m_program, V_DIM_VAR);

    sprintf(m_program, "bar = ...");
    (void) findvar(m_program, V_DIM_VAR);

    sprintf(m_program, "foo%% = ...");
    void *actual = findvar(m_program, V_FIND);

    EXPECT_EQ(int_array, actual);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, vartbl[0].dims[0]);
    EXPECT_STREQ("FOO", vartbl[0].name);
    EXPECT_EQ(T_INT, vartbl[0].type);
    EXPECT_EQ(2, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimFloatArray) {
    sprintf(m_program, "my_array!(2) = ...");
    void *created = findvar(m_program, V_DIM_VAR);

    EXPECT_EQ(vartbl[0].val.fa, created);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, vartbl[0].dims[0]);
    EXPECT_EQ(0, vartbl[0].dims[1]);
    EXPECT_STREQ("MY_ARRAY", vartbl[0].name);
    EXPECT_EQ(T_NBR, vartbl[0].type);

    sprintf(m_program, "my_array(1) = ...");
    void *found = findvar(m_program, V_FIND);

    EXPECT_EQ(created, (void *) ((uintptr_t) found - 8)); // Account for offset of element.
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, vartbl[0].dims[0]);
    EXPECT_EQ(0, vartbl[0].dims[1]);
    EXPECT_STREQ("MY_ARRAY", vartbl[0].name);
    EXPECT_EQ(T_NBR, vartbl[0].type);

    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimIntegerArray) {
    sprintf(m_program, "my_array%%(2,4) = ...");
    void *created = findvar(m_program, V_DIM_VAR);

    EXPECT_EQ(vartbl[0].val.fa, created);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, vartbl[0].dims[0]);
    EXPECT_EQ(4, vartbl[0].dims[1]);
    EXPECT_EQ(0, vartbl[0].dims[2]);
    EXPECT_STREQ("MY_ARRAY", vartbl[0].name);
    EXPECT_EQ(T_INT, vartbl[0].type);

    sprintf(m_program, "my_array%%(1,1) = ...");
    void *found = findvar(m_program, V_FIND);

    EXPECT_EQ(created, (void *) ((uintptr_t) found - 8 * 4)); // Account for offset of element.
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, vartbl[0].dims[0]);
    EXPECT_EQ(4, vartbl[0].dims[1]);
    EXPECT_EQ(0, vartbl[0].dims[2]);
    EXPECT_STREQ("MY_ARRAY", vartbl[0].name);
    EXPECT_EQ(T_INT, vartbl[0].type);

    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimStringArray) {
    sprintf(m_program, "my_array$(2,4,6)");
    void *created = findvar(m_program, V_DIM_VAR);

    EXPECT_EQ(vartbl[0].val.fa, created);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, vartbl[0].dims[0]);
    EXPECT_EQ(4, vartbl[0].dims[1]);
    EXPECT_EQ(6, vartbl[0].dims[2]);
    EXPECT_EQ(0, vartbl[0].dims[3]);
    EXPECT_STREQ("MY_ARRAY", vartbl[0].name);
    EXPECT_EQ(255, vartbl[0].size);
    EXPECT_EQ(T_STR, vartbl[0].type);

    sprintf(m_program, "my_array$(1,1,1)");
    void *found = findvar(m_program, V_FIND);

    EXPECT_EQ(created, (void *) ((uintptr_t) found - 256 * 19)); // Account for offset of element.
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, vartbl[0].dims[0]);
    EXPECT_EQ(4, vartbl[0].dims[1]);
    EXPECT_EQ(6, vartbl[0].dims[2]);
    EXPECT_EQ(0, vartbl[0].dims[3]);
    EXPECT_STREQ("MY_ARRAY", vartbl[0].name);
    EXPECT_EQ(255, vartbl[0].size);
    EXPECT_EQ(T_STR, vartbl[0].type);

    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimArrayWithNoType) {
    mmb_options.default_type = T_NOTYPE;

    sprintf(m_program, "my_array = ...");
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("Variable type not specified", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimEmptyArray) {
    sprintf(m_program, "foo() = ...");
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("Dimensions", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimArrayWithTooManyDimensions) {
    sprintf(m_program, "foo(1,1,1,1,1,1,1,1,1) = ...");
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("Dimensions", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimArrayWithDimensionEqualToOptionBase) {
    mmb_options.base = 0;
    sprintf(m_program, "foo(0) = ...");
    error_msg[0] = '\0';
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("Dimensions", error_msg);

    mmb_options.base = 0;
    sprintf(m_program, "foo2D(2, 0) = ...");
    error_msg[0] = '\0';
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("Dimensions", error_msg);

    mmb_options.base = 1;
    sprintf(m_program, "bar(1) = ...");
    error_msg[0] = '\0';
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("Dimensions", error_msg);

    mmb_options.base = 1;
    sprintf(m_program, "bar2D(2, 1) = ...");
    error_msg[0] = '\0';
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("Dimensions", error_msg);
}

extern "C" void op_subtract();

TEST_F(MmBasicCoreTest, FindVar_GivenFindWithDimensionLessThanOptionBase) {
    error_msg[0] = '\0';
    sprintf(m_program, "foo(2,5) = ...");
    (void) findvar(m_program, V_DIM_VAR);

    // We can't just put a literal negative value for one of the bounds,
    // we need to insert the token for the subtract operator.
    char SUBTRACT_TOKEN = 0;
    for (int ii = 0; tokentbl[ii].fptr; ++ii) {
        if (tokentbl[ii].fptr == op_subtract) {
            SUBTRACT_TOKEN = ii + C_BASETOKEN;
        }
    }

    error_msg[0] = '\0';
    mmb_options.base = 0;
    sprintf(m_program, "foo(1,%c1) = ...", SUBTRACT_TOKEN);
    (void) findvar(m_program, V_FIND);
    EXPECT_STREQ("Dimensions", error_msg);

    error_msg[0] = '\0';
    mmb_options.base = 1;
    sprintf(m_program, "foo(1,0) = ...");
    (void) findvar(m_program, V_FIND);
    EXPECT_STREQ("Dimensions", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenFindNonExistentEmptyArray) {
    sprintf(m_program, "foo() = ...");
    (void) findvar(m_program, V_FIND);

    EXPECT_STREQ("Dimensions", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenFindNonExistentEmptyArray_GivenEmptyOk) {
    sprintf(m_program, "foo() = ...");
    void *actual = findvar(m_program, V_DIM_VAR | V_EMPTY_OK);

    EXPECT_EQ(vartbl[0].val.fa, actual);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(-1, vartbl[0].dims[0]);
    EXPECT_EQ(0, vartbl[0].dims[1]);
    EXPECT_EQ(0, vartbl[0].dims[2]);
    EXPECT_EQ(0, vartbl[0].level);
    EXPECT_STREQ("FOO", vartbl[0].name);
    EXPECT_EQ(T_NBR, vartbl[0].type);
    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenFindExistingEmptyArray) {
    sprintf(m_program, "foo(2,4) = ...");
    (void) findvar(m_program, V_DIM_VAR);

    sprintf(m_program, "foo() = ...");
    void *actual = findvar(m_program, V_EMPTY_OK);

    EXPECT_EQ(vartbl[0].val.fa, actual);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, vartbl[0].dims[0]);
    EXPECT_EQ(4, vartbl[0].dims[1]);
    EXPECT_EQ(0, vartbl[0].dims[2]);
    EXPECT_EQ(0, vartbl[0].level);
    EXPECT_STREQ("FOO", vartbl[0].name);
    EXPECT_EQ(T_NBR, vartbl[0].type);
    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenRedimArray) {
    sprintf(m_program, "foo%%(2,4) = ...");
    (void) findvar(m_program, V_DIM_VAR);

    sprintf(m_program, "foo%%(1,2) = ...");
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("$ already declared", error_msg);

    // Code contains this error but there is no code path to get to it that
    // does not hit "$ already declared" first.
    // EXPECT_STREQ("Cannot re dimension array", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenArrayDimensionOutOfBounds) {
    sprintf(m_program, "foo%%(2,4) = ...");
    (void) findvar(m_program, V_DIM_VAR);

    sprintf(m_program, "foo%%(3,4) = ...");
    (void) findvar(m_program, V_FIND);

    EXPECT_STREQ("Index out of bounds", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenArrayDimensionMismatch) {
    sprintf(m_program, "foo%%(2,4) = ...");
    (void) findvar(m_program, V_DIM_VAR);

    sprintf(m_program, "foo%%(1) = ...");
    (void) findvar(m_program, V_FIND);

    EXPECT_STREQ("Array dimensions", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimStringArrayWithLength) {
    sprintf(m_program, "my_array$(5) Length 32");
    void *created = findvar(m_program, V_DIM_VAR);

    EXPECT_EQ(vartbl[0].val.fa, created);
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(5, vartbl[0].dims[0]);
    EXPECT_EQ(0, vartbl[0].dims[1]);
    EXPECT_STREQ("MY_ARRAY", vartbl[0].name);
    EXPECT_EQ(32, vartbl[0].size);
    EXPECT_EQ(T_STR, vartbl[0].type);

    sprintf(m_program, "my_array$(2)");
    void *found = findvar(m_program, V_FIND);

    EXPECT_EQ(created, (void *) ((uintptr_t) found - (32 + 1) * 2)); // Account for offset of element.
    EXPECT_EQ(0, VarIndex);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(5, vartbl[0].dims[0]);
    EXPECT_EQ(0, vartbl[0].dims[1]);
    EXPECT_STREQ("MY_ARRAY", vartbl[0].name);
    EXPECT_EQ(32, vartbl[0].size);
    EXPECT_EQ(T_STR, vartbl[0].type);

    EXPECT_EQ(1, varcnt);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimStringArrayWithLengthTooLong) {
    sprintf(m_program, "my_array$(5) Length 256");
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("% is invalid (valid is % to %)", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenDimStringArrayWithUnexpectedText) {
    sprintf(m_program, "my_array$(5) Aardvark");
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("Unexpected text: $", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenFindEmptyArray_GivenDeclaredScalar) {
    sprintf(m_program, "foo = 1");
    (void) findvar(m_program, V_DIM_VAR);

    error_msg[0] = '\0';
    sprintf(m_program, "foo()");
    (void) findvar(m_program, V_EMPTY_OK);

    EXPECT_STREQ("Array dimensions", error_msg);
}

TEST_F(MmBasicCoreTest, FindVar_GivenTooManyDeclarations) {
    int ii;
    for (ii = 1; ; ++ii) {
        error_msg[0] = '\0';
        sprintf(m_program, "var_%d", ii);
        (void) findvar(m_program, V_DIM_VAR);
        if (*error_msg) break;
    }

    // The 1025'th request should fail.
    EXPECT_STREQ("Too many variables", error_msg);
    EXPECT_EQ(1025, ii);
}

TEST_F(MmBasicCoreTest, FindVar_GivenArrayDimensionTooLarge) {
    sprintf(m_program, "my_array%%(32768)");
    (void) findvar(m_program, V_DIM_VAR);

    EXPECT_STREQ("Array bound exceeds maximum: %", error_msg);
}

TEST_F(MmBasicCoreTest, Tokenise_DimStatement) {
    sprintf(inpbuf, "Dim a = 1");

    tokenise(0);

    char expected[TKNBUF_SIZE];
    sprintf(
            expected,
            "%c%c%ca %c 1",
            T_NEWLINE,
            (GetCommandValue("Dim") & 0x7F) + C_BASETOKEN,
            (GetCommandValue("Dim") >> 7) + C_BASETOKEN,
            GetTokenValue("="));
    EXPECT_STREQ(expected, tknbuf);
}

TEST_F(MmBasicCoreTest, Tokenise_RunStatement) {
    sprintf(inpbuf, "Run \"foo\", --base=1");

    tokenise(0);

    char expected[TKNBUF_SIZE];
    sprintf(
            expected,
            "%c%c%c\"foo\", %c%cbase%c1",
            T_NEWLINE,
            (GetCommandValue("Run") & 0x7F) + C_BASETOKEN,
            (GetCommandValue("Run") >> 7) + C_BASETOKEN,
            GetTokenValue("-"),
            GetTokenValue("-"),
            GetTokenValue("="));
    EXPECT_STREQ(expected, tknbuf);
}

TEST_F(MmBasicCoreTest, PrepareProgram_And_FindSubFun) {
    TokeniseAndAppend("Sub foo()");
    TokeniseAndAppend("End Sub");
    TokeniseAndAppend("Function bar%%()");
    TokeniseAndAppend("End Function");
    TokeniseAndAppend("CSub wom()");
    TokeniseAndAppend("End CSub");
//    TokeniseAndAppend("CFunction bat!()");
//    TokeniseAndAppend("End CFunction");

    PrepareProgram(1);

    EXPECT_STREQ("", error_msg);

    int fun_idx = FindSubFun("foo", kSub);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, fun_idx);
    EXPECT_EQ(kSub, funtbl[fun_idx].type);
    EXPECT_EQ(ProgMemory + 1, funtbl[fun_idx].addr);

    fun_idx = FindSubFun("bar", kFunction);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(1, fun_idx);
    EXPECT_EQ(kFunction, funtbl[fun_idx].type);
    EXPECT_EQ(ProgMemory + 14, funtbl[fun_idx].addr);

    fun_idx = FindSubFun("wom", kSub);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, fun_idx);
    EXPECT_EQ(kSub, funtbl[fun_idx].type);
    EXPECT_EQ(ProgMemory + 29, funtbl[fun_idx].addr);
}

TEST_F(MmBasicCoreTest, PrepareProgram_GivenMaximumNumberOfFunctions) {
    char buf[MAXVARLEN + 1];
    for (int ii = 0; ii < MAXSUBFUN; ++ii) {
        sprintf(buf, "Function fun%d()", ii);
        TokeniseAndAppend(buf);
        TokeniseAndAppend("End Function");
    }

    PrepareProgram(1);

    EXPECT_STREQ("", error_msg);
    int fun_idx;
    for (int ii = 0; ii < MAXSUBFUN; ++ii) {
        sprintf(buf, "fun%d", ii);
        fun_idx = FindSubFun(buf, kFunction);
        EXPECT_STREQ("", error_msg);
        EXPECT_EQ(ii, fun_idx);
    }
}

TEST_F(MmBasicCoreTest, PrepareProgram_GivenTooManyFunctions) {
    char buf[MAXVARLEN + 1];
    for (int ii = 0; ii < MAXSUBFUN + 1; ++ii) {
        sprintf(buf, "Function fun%d()", ii);
        TokeniseAndAppend(buf);
        TokeniseAndAppend("End Function");
    }

    PrepareProgram(1);

    EXPECT_STREQ("Too many functions/labels/subroutines", error_msg);

    error_msg[0] = '\0';
    PrepareProgram(0); // Should not report error.

    EXPECT_STREQ("", error_msg);
}

TEST_F(MmBasicCoreTest, PrepareProgram_Errors_GivenInvalidFunctionName) {
    TokeniseAndAppend("Function .foo()");
    TokeniseAndAppend("End Function");

    PrepareProgram(1);

    EXPECT_STREQ("Invalid function name", error_msg);

    error_msg[0] = '\0';
    PrepareProgram(0); // Should not report error.

    EXPECT_STREQ("", error_msg);
}

TEST_F(MmBasicCoreTest, PrepareProgram_Errors_GivenInvalidSubName) {
    TokeniseAndAppend("Sub .foo()");
    TokeniseAndAppend("End Sub");

    PrepareProgram(1);

    EXPECT_STREQ("Invalid subroutine name", error_msg);

    error_msg[0] = '\0';
    PrepareProgram(0); // Should not report error.

    EXPECT_STREQ("", error_msg);
}

TEST_F(MmBasicCoreTest, PrepareProgram_Succeeds_GivenFunctionNameContainingPeriod) {
    TokeniseAndAppend("Function f.oo()");
    TokeniseAndAppend("End Function");
    TokeniseAndAppend("Sub b.ar()");
    TokeniseAndAppend("End Sub");

    PrepareProgram(1);

    EXPECT_STREQ("", error_msg);

    int fun_idx = FindSubFun("f.oo", kFunction);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, fun_idx);

    fun_idx = FindSubFun("b.ar", kSub);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(MmBasicCoreTest, PrepareProgram_Succeeds_GivenFunctionHasMaximumLengthName) {
    TokeniseAndAppend("Function " MAX_LENGTH_NAME "()");
    TokeniseAndAppend("End Function");

    PrepareProgram(1);

    EXPECT_STREQ("", error_msg);

    int fun_idx = FindSubFun(MAX_LENGTH_NAME, kFunction);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(MmBasicCoreTest, PrepareProgram_Succeeds_GivenLabelHasMaximumLength) {
    TokeniseAndAppend(MAX_LENGTH_NAME ":");
    TokeniseAndAppend("Print \"foo\"");

    PrepareProgram(1);

    EXPECT_STREQ("", error_msg);

    int fun_idx = FindSubFun(MAX_LENGTH_NAME, kLabel);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(MmBasicCoreTest, PrepareProgram_Succeeds_GivenSubHasMaximumLengthName) {
    TokeniseAndAppend("Sub " MAX_LENGTH_NAME "()");
    TokeniseAndAppend("End Sub");

    PrepareProgram(1);

    EXPECT_STREQ("", error_msg);

    int fun_idx = FindSubFun(MAX_LENGTH_NAME, kSub);
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(MmBasicCoreTest, PrepareProgram_Errors_GivenFunctionNameTooLong) {
    TokeniseAndAppend("Function " MAX_LENGTH_NAME "A()");
    TokeniseAndAppend("End Function");

    PrepareProgram(1);

    EXPECT_STREQ("Function name too long", error_msg);

    error_msg[0] = '\0';
    PrepareProgram(0);  // Should not report error.

    EXPECT_STREQ("", error_msg);
}

TEST_F(MmBasicCoreTest, PrepareProgram_Errors_GivenSubNameTooLong) {
    TokeniseAndAppend("Sub " MAX_LENGTH_NAME "A()");
    TokeniseAndAppend("End Sub");

    PrepareProgram(1);

    EXPECT_STREQ("Subroutine name too long", error_msg);

    error_msg[0] = '\0';
    PrepareProgram(0);  // Should not report error.

    EXPECT_STREQ("", error_msg);
}

TEST_F(MmBasicCoreTest, PrepareProgram_GivenSubWithSameNameAsFunction) {
    TokeniseAndAppend("Function foo()");
    TokeniseAndAppend("End Function");
    TokeniseAndAppend("Sub foo()");
    TokeniseAndAppend("End Sub");

    PrepareProgram(1);

    EXPECT_STREQ("Function/subroutine already declared", error_msg);

    error_msg[0] = '\0';
    PrepareProgram(0);  // Should not report error.

    EXPECT_STREQ("", error_msg);
}

TEST_F(MmBasicCoreTest, PrepareProgram_Errors_GivenDuplicateLabel) {
    TokeniseAndAppend("foo:");
    TokeniseAndAppend("Print \"foo\"");
    TokeniseAndAppend("foo:");
    TokeniseAndAppend("Print \"bar\"");

    PrepareProgram(1);

    EXPECT_STREQ("Duplicate label", error_msg);

    error_msg[0] = '\0';
    PrepareProgram(0);  // Should not report error.

    EXPECT_STREQ("", error_msg);
}

TEST_F(MmBasicCoreTest, PrepareProgram_GivenTwoFunctionsWithSameNameButDifferentTypeSuffix) {
    TokeniseAndAppend("Function foo!()");
    TokeniseAndAppend("End Function");
    TokeniseAndAppend("Function foo%()");
    TokeniseAndAppend("End Function");

    PrepareProgram(1);

    EXPECT_STREQ("Function/subroutine already declared", error_msg);

    error_msg[0] = '\0';
    PrepareProgram(0); // Should not report error.

    EXPECT_STREQ("", error_msg);
}

TEST_F(MmBasicCoreTest, PrepareProgram_GivenMixOfFunctionsLabelsAndSubs) {
    // Deliberately erratically spaced.
    TokeniseAndAppend("  zzz: Data \"zzz\"");
    TokeniseAndAppend(" aaa:   Sub  aaa(i As Integer, f As Float)");
    TokeniseAndAppend(" i = Int(f)");
    TokeniseAndAppend("End Sub");
    TokeniseAndAppend("Data \"aaa\", \"zzz\"");
    TokeniseAndAppend("    bbb: Function bbb(i As Integer, f As Float) As Integer");
    TokeniseAndAppend("bbb = i * Int(f)");
    TokeniseAndAppend(" End Function");
    TokeniseAndAppend("  Data \"bbb\"");

    PrepareProgram(1);
    
    EXPECT_STREQ("", error_msg);

    EXPECT_STREQ("ZZZ", funtbl[0].name);
    EXPECT_EQ(kLabel, funtbl[0].type);
    EXPECT_EQ(ProgMemory, funtbl[0].addr);

    EXPECT_STREQ("AAA", funtbl[1].name);
    EXPECT_EQ(kLabel, funtbl[1].type);
    EXPECT_EQ(ProgMemory + 17, funtbl[1].addr);

    EXPECT_STREQ("AAA", funtbl[2].name);
    EXPECT_EQ(kSub, funtbl[2].type);
    EXPECT_EQ(ProgMemory + 27, funtbl[2].addr);

    EXPECT_STREQ("BBB", funtbl[3].name);
    EXPECT_EQ(kLabel, funtbl[3].type);
    EXPECT_EQ(ProgMemory + 90, funtbl[3].addr);

    EXPECT_STREQ("BBB", funtbl[4].name);
    EXPECT_EQ(kFunction, funtbl[4].type);
    EXPECT_EQ(ProgMemory + 101, funtbl[4].addr);
}

TEST_F(MmBasicCoreTest, FindSubFun_Errors_GivenFunctionNameTooLong) {
    int fun_idx = FindSubFun(MAX_LENGTH_NAME "A", kFunction);

    EXPECT_STREQ("Function name too long", error_msg);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(MmBasicCoreTest, FindSubFun_Errors_GivenSubNameTooLong) {
    int fun_idx = FindSubFun(MAX_LENGTH_NAME "A", kSub);

    EXPECT_STREQ("Subroutine name too long", error_msg);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(MmBasicCoreTest, FindSubFun_Errors_GivenFunctionOrSubNameTooLong) {
    int fun_idx = FindSubFun(MAX_LENGTH_NAME "A", kFunction | kSub);

    EXPECT_STREQ("Function/subroutine name too long", error_msg);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(MmBasicCoreTest, FindSubFun_GivenTypeMismatch) {
    TokeniseAndAppend("Function foo%()");
    TokeniseAndAppend("End Function");
    PrepareProgram(1);

    int fun_idx = FindSubFun("foo!", kFunction);

    // The type suffix does not form part of the name and the type checking is
    // performed in the function calling code only after the function has
    // been found.
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(MmBasicCoreTest, FindSubFun_GivenLookingForSub_ButFoundFunction) {
    TokeniseAndAppend("Function foo%()");
    TokeniseAndAppend("End Function");
    PrepareProgram(1);

    int fun_idx = FindSubFun("foo", kSub);

    EXPECT_STREQ("Not a subroutine", error_msg);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(MmBasicCoreTest, FindSubFun_GivenLookingForFunction_ButFoundSub) {
    TokeniseAndAppend("Sub foo()");
    TokeniseAndAppend("End Sub");
    PrepareProgram(1);

    int fun_idx = FindSubFun("foo", kFunction);

    EXPECT_STREQ("Not a function", error_msg);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(MmBasicCoreTest, FindSubFun_GivenLookingForFunctionOrSub) {
    TokeniseAndAppend("Function foo()");
    TokeniseAndAppend("End Function");
    TokeniseAndAppend("Sub bar()");
    TokeniseAndAppend("End Sub");
    PrepareProgram(1);

    int fun_idx = FindSubFun("foo", kFunction | kSub);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(0, fun_idx);

    fun_idx = FindSubFun("bar", kFunction | kSub);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(MmBasicCoreTest, FindSubFun_GivenLookingForFunction_ButFindLabel) {
    TokeniseAndAppend("foo:");
    TokeniseAndAppend("  Print \"Hello\"");
    PrepareProgram(1);

    int fun_idx = FindSubFun("foo", kFunction);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(MmBasicCoreTest, FindSubFun_GivenLookingForSub_ButFindLabel) {
    TokeniseAndAppend("foo:");
    TokeniseAndAppend("  Print \"Hello\"");
    PrepareProgram(1);

    int fun_idx = FindSubFun("foo", kSub);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(MmBasicCoreTest, FindSubFun_GivenLookingForFunctionOrSub_ButFindLabel) {
    TokeniseAndAppend("foo:");
    TokeniseAndAppend("  Print \"Hello\"");
    PrepareProgram(1);

    int fun_idx = FindSubFun("foo", kFunction | kSub);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(MmBasicCoreTest, FindLabel_GivenLabelPresent) {
    TokeniseAndAppend("Print \"Hello\"");
    TokeniseAndAppend("Goto foo");
    TokeniseAndAppend("Print \"End1\"");
    TokeniseAndAppend("End");
    TokeniseAndAppend("foo:");
    TokeniseAndAppend("Print \"World\"");
    TokeniseAndAppend("Goto bar");
    TokeniseAndAppend("Print \"End2\"");
    TokeniseAndAppend("bar: Print \"Humbug\"");
    TokeniseAndAppend("Print \"End3\"");
    PrepareProgram(true);

    const char *addr = findlabel("foo");

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(T_NEWLINE, *addr);
    EXPECT_EQ(T_LABEL, *(addr + 1));
    EXPECT_EQ(3, *(addr + 2));
    EXPECT_EQ(0, memcmp(addr + 3, "foo", 3));

    addr = findlabel("bar");

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(T_NEWLINE, *addr);
    EXPECT_EQ(T_LABEL, *(addr + 1));
    EXPECT_EQ(3, *(addr + 2));
    EXPECT_EQ(0, memcmp(addr + 3, "bar", 3));
}

TEST_F(MmBasicCoreTest, FindLabel_GivenLabelNotPresent) {
    TokeniseAndAppend("Print \"Hello\"");
    TokeniseAndAppend("foobar:");
    TokeniseAndAppend("Print \"World\"");
    TokeniseAndAppend("End");
    PrepareProgram(true);

    const char *addr = findlabel("wombat");

    EXPECT_STREQ("Label not found", error_msg);
    EXPECT_EQ(NULL, addr);
}

TEST_F(MmBasicCoreTest, FindLabel_IsCaseInsensitive) {
    TokeniseAndAppend("Print \"Hello\"");
    TokeniseAndAppend("foobar:");
    TokeniseAndAppend("Print \"World\"");
    TokeniseAndAppend("End");
    PrepareProgram(true);

    const char *addr = findlabel("FOOBAR");

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(T_NEWLINE, *addr);
    EXPECT_EQ(T_LABEL, *(addr + 1));
    EXPECT_EQ(6, *(addr + 2));
    EXPECT_EQ(0, memcmp(addr + 3, "foobar", 6));
}

TEST_F(MmBasicCoreTest, FindLabel_RequiresCompleteMatch) {
    TokeniseAndAppend("Print \"Hello\"");
    TokeniseAndAppend("foobar:");
    TokeniseAndAppend("Print \"World\"");
    TokeniseAndAppend("End");
    PrepareProgram(true);

    const char *addr = findlabel("foo"); // Try to match on a prefix.

    EXPECT_STREQ("Label not found", error_msg);
    EXPECT_EQ(NULL, addr);

    error_msg[0] = '\0';
    addr = findlabel("bar"); // Try to match on a suffix.

    EXPECT_STREQ("Label not found", error_msg);
    EXPECT_EQ(NULL, addr);
}

TEST_F(MmBasicCoreTest, FindLabel_GivenLabelPreceededBySpaces) {
    TokeniseAndAppend("Print \"Hello\"");
    TokeniseAndAppend("  foobar:");
    TokeniseAndAppend("Print \"World\"");
    TokeniseAndAppend("End");
    PrepareProgram(true);

    const char *addr = findlabel("foobar");

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(T_NEWLINE, *addr);
    EXPECT_EQ(' ', *(addr + 1));
    EXPECT_EQ(' ', *(addr + 2));
    EXPECT_EQ(T_LABEL, *(addr + 3));
    EXPECT_EQ(6, *(addr + 4));
    EXPECT_EQ(0, memcmp(addr + 5, "foobar", 6));
}

TEST_F(MmBasicCoreTest, FindLabel_GivenLabelPreceededByLineNumber) {
    TokeniseAndAppend("Print \"Hello\"");
    TokeniseAndAppend("100 foobar:");
    TokeniseAndAppend("Print \"World\"");
    TokeniseAndAppend("End");
    PrepareProgram(true);

    const char *addr = findlabel("foobar");

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(T_NEWLINE, *addr);
    EXPECT_EQ(T_LINENBR, *(addr + 1));
    EXPECT_EQ(0x00, *(addr + 2)); // 16-bit line number ...
    EXPECT_EQ(0x64, *(addr + 3)); // ... 0x64 = 100
    EXPECT_EQ(' ', *(addr + 4));  // space
    EXPECT_EQ(T_LABEL, *(addr + 5));
    EXPECT_EQ(6, *(addr + 6));
    EXPECT_EQ(0, memcmp(addr + 7, "foobar", 6));
}

TEST_F(MmBasicCoreTest, FindLabel_GivenLabelHasMaximumLength) {
    TokeniseAndAppend("Print \"Hello\"");
    TokeniseAndAppend(MAX_LENGTH_NAME ":");
    TokeniseAndAppend("Print \"World\"");
    TokeniseAndAppend("End");
    PrepareProgram(true);

    const char *addr = findlabel(MAX_LENGTH_NAME);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(T_NEWLINE, *addr);
    EXPECT_EQ(T_LABEL, *(addr + 1));
    EXPECT_EQ(32, *(addr + 2));
    EXPECT_EQ(0, memcmp(addr + 3, MAX_LENGTH_NAME, 32));
}

TEST_F(MmBasicCoreTest, FindLabel_GivenLabelArgumentIsTooLong) {
    TokeniseAndAppend("Print \"Hello\"");
    TokeniseAndAppend("foo:");
    TokeniseAndAppend("Print \"World\"");
    TokeniseAndAppend("End");
    PrepareProgram(true);

    const char *addr = findlabel(TOO_LONG_NAME);

    EXPECT_STREQ("Label too long", error_msg);
    EXPECT_EQ(NULL, addr);
}

TEST_F(MmBasicCoreTest, FindLabel_GivenLabelInProgramIsTooLong) {
    TokeniseAndAppend("Print \"Hello\"");
    TokeniseAndAppend(TOO_LONG_NAME ":");
    TokeniseAndAppend("Print \"World\"");
    TokeniseAndAppend("End");

    PrepareProgram(true);

    // At the moment the presence of the invalid label is not enough to
    // cause an error.
    EXPECT_STREQ("", error_msg);

    const char *addr = findlabel(TOO_LONG_NAME);

    EXPECT_STREQ("Label too long", error_msg);
    EXPECT_EQ(NULL, addr);
}

TEST_F(MmBasicCoreTest, FindLabel_GivenLabelWithinMultiStatementLine) {
    TokeniseAndAppend("Print \"Hello\"");
    TokeniseAndAppend("Print \"abc\" : foobar: Print \"def\"");
    TokeniseAndAppend("Print \"World\"");
    TokeniseAndAppend("End");
    PrepareProgram(true);

    const char *addr = findlabel("foobar");

    // Because it is not at the 'start' of the line the colon following 'foobar'
    // is converted to a statement separator '\0' instead of being recognised as
    // terminating a label.
    EXPECT_STREQ("Label not found", error_msg);
    EXPECT_EQ(NULL, addr);
}

TEST_F(MmBasicCoreTest, FindLabel_Errors_GivenFoundFunction) {
    TokeniseAndAppend("Print \"Hello World\"");
    TokeniseAndAppend("Function foo()");
    TokeniseAndAppend("End Function");
    PrepareProgram(true);

    const char *addr = findlabel("foo");

    EXPECT_STREQ("Not a label", error_msg);
    EXPECT_EQ(NULL, addr);
}

TEST_F(MmBasicCoreTest, FindLabel_Errors_GivenFoundSub) {
    TokeniseAndAppend("Print \"Hello World\"");
    TokeniseAndAppend("Sub foo()");
    TokeniseAndAppend("End Sub");
    PrepareProgram(true);

    const char *addr = findlabel("foo");

    EXPECT_STREQ("Not a label", error_msg);
    EXPECT_EQ(NULL, addr);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenStringScalar) {
    TokeniseAndAppend("Print x$");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 5, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenIntegerScalar) {
    TokeniseAndAppend("Print xy%");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 6, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenFloatScalar) {
    TokeniseAndAppend("Print xyz!");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 7, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenUntypedScalar) {
    TokeniseAndAppend("Print xyz_");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 7, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenMaxLengthNamePlusExtension) {
    TokeniseAndAppend("Print " MAX_LENGTH_NAME "$");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 36, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenTooLongName) {
    TokeniseAndAppend("Print " TOO_LONG_NAME "$");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("Variable name too long", error_msg);
    EXPECT_EQ(0x0, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenTooLongName_AndNoErrorSet) {
    TokeniseAndAppend("Print " TOO_LONG_NAME "$");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 1);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 36, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenNotAName) {
    TokeniseAndAppend("Print 1");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 2;

    const char *actual = skipvar(p, 0);

    EXPECT_EQ(ProgMemory + 2, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenLeadingSpaces) {
    TokeniseAndAppend("Print    abc!");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 10, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenUndimensionedArray) {
    TokeniseAndAppend("Print abc()");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 8, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenUnbalancedBrackets) {
    TokeniseAndAppend("Print abc(");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("Expected closing bracket", error_msg);
    EXPECT_EQ(NULL, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenUnbalancedBrackets_AndNoErrorSet) {
    TokeniseAndAppend("Print abc(");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 1);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 7, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenSpaceBeforeBrackets) {
    TokeniseAndAppend("Print abc  ()");
    // Tokenising removes the spaces,
    // insert them back in for sake of test.
    memcpy(ProgMemory + 6, "  ()\0\0", 6);
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 10, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenStringArg) {
    TokeniseAndAppend("Print abc(\"def\")");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 13, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenBracketInQuotes) {
    TokeniseAndAppend("Print abc(\"(\")");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 11, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenInternalFunctionCallInBrackets) {
    TokeniseAndAppend("Print abc(Int(5))");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 11, actual);
}

TEST_F(MmBasicCoreTest, SkipVar_GivenUserFunctionCallInBrackets) {
    TokeniseAndAppend("Print abc(def(5))");
    TokeniseAndAppend("Print \"something more\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3;

    const char *actual = skipvar(p, 0);

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 14, actual);
}

TEST_F(MmBasicCoreTest, GetIntAddress_Succeeds_GivenSubroutine) {
    TokeniseAndAppend("foo");
    TokeniseAndAppend("Sub foo()");
    TokeniseAndAppend("End Sub");
    PrepareProgram(true);

    const char *actual = GetIntAddress(ProgMemory + 1); // Skip initial T_NEWLINE.

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 6, actual); // Point to the SUB token.
}

TEST_F(MmBasicCoreTest, GetIntAddress_Succeeds_GivenLabel) {
    TokeniseAndAppend("foo");
    TokeniseAndAppend("foo:");
    TokeniseAndAppend("  Print \"bar\"");
    PrepareProgram(true);

    const char *actual = GetIntAddress(ProgMemory + 1); // Skip initial T_NEWLINE.

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 5, actual); // Point to the T_NEWLINE on the label line.
}

TEST_F(MmBasicCoreTest, GetIntAddress_Errors_GivenFunction) {
    TokeniseAndAppend("foo");
    TokeniseAndAppend("Function foo()");
    TokeniseAndAppend("End Function");

    const char *actual = GetIntAddress(ProgMemory + 1); // Skip initial T_NEWLINE.

    EXPECT_STREQ("Label/subroutine not found", error_msg);
    EXPECT_EQ(NULL, actual);
}

TEST_F(MmBasicCoreTest, GetIntAddress_Succeeds_GivenLineNumber) {
    TokeniseAndAppend("Goto 100");
    TokeniseAndAppend("100 Print \"bar\"");
    PrepareProgram(true);

    const char *actual = GetIntAddress(ProgMemory + 3); // Skip initial T_NEWLINE and GOTO.

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 8, actual); // Point to the T_LINENBR on the numbered line.
}

TEST_F(MmBasicCoreTest, GetIntAddress_Errors_GivenNonExistentTarget) {
    TokeniseAndAppend("bar");
    TokeniseAndAppend("Sub foo()");
    TokeniseAndAppend("End Sub");
    PrepareProgram(true);

    const char *actual = GetIntAddress(ProgMemory + 1); // Skip initial T_NEWLINE.

    EXPECT_STREQ("Label/subroutine not found", error_msg);
    EXPECT_EQ(NULL, actual);
}

TEST_F(MmBasicCoreTest, GetIntAddress_Succeeds_GivenMaxNameLength) {
    TokeniseAndAppend(MAX_LENGTH_NAME);
    TokeniseAndAppend("Sub " MAX_LENGTH_NAME "()");
    TokeniseAndAppend("End Sub");
    PrepareProgram(true);

    const char *actual = GetIntAddress(ProgMemory + 1); // Skip initial T_NEWLINE.

    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(ProgMemory + 35, actual); // Point to the SUB token.
}

TEST_F(MmBasicCoreTest, GetIntAddress_Errors_GivenTargetNameTooLong) {
    TokeniseAndAppend(TOO_LONG_NAME);
    TokeniseAndAppend("Sub foo()");
    TokeniseAndAppend("End Sub");
    PrepareProgram(true);

    const char *actual = GetIntAddress(ProgMemory + 1); // Skip initial T_NEWLINE.

    EXPECT_STREQ("Label/subroutine name too long", error_msg);
    EXPECT_EQ(NULL, actual);
}

TEST_F(MmBasicCoreTest, MakeArgs) {
    TokeniseAndAppend("If foo = -1 Then Error \"bar\"");
    PrepareProgram(true);
    const char *p = ProgMemory + 3; // Skip initial T_NEWLINE and IF token
    char argbuf[STRINGSIZE];
    char *argv[10];
    int argc[10];
    char ss[3];
    ss[0] = tokenTHEN;
    ss[1] = tokenELSE;
    ss[2] = 0;

    makeargs(&p, 10, argbuf, argv, argc, ss);
}
