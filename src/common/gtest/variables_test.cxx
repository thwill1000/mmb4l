/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

#include "test_config.h"

extern "C" {

#include "../variables.h"
#include "../../core/VarTable.h"

int varcnt;
struct s_vartbl *vartbl;

}

#define VARIABLES_TEST_DIR  TMP_DIR "/VariablesTest"

class VariablesTest : public ::testing::Test {

protected:

    void SetUp() override {
        vartbl = (struct s_vartbl*) calloc(MAXVARS, sizeof(struct s_vartbl));
        varcnt = 0;
    }

    void TearDown() override {
        free(vartbl);
    }
};

TEST_F(VariablesTest, Add) {
    EXPECT_EQ(0, variables_add("foo"));
    EXPECT_STREQ("foo", vartbl[0].name);
    EXPECT_EQ(1, variables_add("bar"));
    EXPECT_STREQ("bar", vartbl[1].name);
    EXPECT_EQ(2, variables_add("FOO"));
    EXPECT_STREQ("FOO", vartbl[2].name);

    // Maximum length name, will be stored without '\0' termination.
    memset(vartbl + 3, 'x', sizeof(struct s_vartbl));
    EXPECT_EQ(3, variables_add("_32_chars_long_67890123456789012"));
    EXPECT_EQ(0, memcmp("_32_chars_long_67890123456789012x", vartbl[3].name, 33));

    // Longer than maximum length name, will be stored truncated and without
    // '\0' termination.
    memset(vartbl + 4, 'y', sizeof(struct s_vartbl));
    EXPECT_EQ(4, variables_add("_33_chars_long_678901234567890123"));
    EXPECT_EQ(0, memcmp("_33_chars_long_67890123456789012y", vartbl[4].name, 33));
}

TEST_F(VariablesTest, Find) {
    variables_add("foo");
    variables_add("bar");
    variables_add("FOO");
    variables_add("_32_chars_long_67890123456789012");
    variables_add("");

    EXPECT_EQ(0, variables_find("foo"));
    EXPECT_EQ(1, variables_find("bar"));
    EXPECT_EQ(-1, variables_find("wombat"));

    // Case-sensitive, but during real execution all names are UPPER-CASE.
    EXPECT_EQ(2, variables_find("FOO"));

    // Only the first 32-chars are checked.
    EXPECT_EQ(3, variables_find("_32_chars_long_67890123456789012"));
    EXPECT_EQ(3, variables_find("_32_chars_long_67890123456789012x"));

    // During real execution we don't expect empty variable names.
    EXPECT_EQ(4, variables_find(""));
}
