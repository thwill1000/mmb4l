/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../../Hardware_Includes.h"
#include "../FunTable.h"
#include "../mmbasic_core_xtra.h"

char error_msg[256];

void error_throw_legacy(const char *msg, ...) {
    strcpy(error_msg, msg);
}

struct s_funtbl funtbl[MAXSUBFUN] = { 0 };
const char *subfun[MAXSUBFUN] = { 0 };

} // extern "C"

class MmBasicCoreTest : public ::testing::Test {

protected:

    void SetUp() override {
        error_msg[0] = '\0';
    }

    void TearDown() override {
    }

};

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
