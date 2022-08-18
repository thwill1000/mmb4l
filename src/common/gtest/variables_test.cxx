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
size_t memory[10];
size_t memory_count;

/** Sets value of pointed to memory slot to 0. */
void FreeMemory(void *addr) {
    if (!addr) return;
    *((size_t *) addr) = 0;
}

/** Gets pointer to next memory slot, filling it with the value of size. */
void *GetMemory(size_t size) {
    memory[memory_count] = size;
    return memory + memory_count++;
}

} // extern "C"

#define VARIABLES_TEST_DIR  TMP_DIR "/VariablesTest"

class VariablesTest : public ::testing::Test {

protected:

    void SetUp() override {
        vartbl = (struct s_vartbl*) calloc(MAXVARS, sizeof(struct s_vartbl));
        varcnt = 0;
        memset(memory, 0, sizeof(memory));
        memory_count = 0;
    }

    void TearDown() override {
        free(vartbl);
    }
};

TEST_F(VariablesTest, Add) {
    EXPECT_EQ(0, variables_add("foo", 0));
    EXPECT_STREQ("foo", vartbl[0].name);
    EXPECT_EQ(1, variables_add("bar", 0));
    EXPECT_STREQ("bar", vartbl[1].name);
    EXPECT_EQ(2, variables_add("FOO", 0));
    EXPECT_STREQ("FOO", vartbl[2].name);

    // Maximum length name, will be stored without '\0' termination.
    memset(vartbl + 3, 'x', sizeof(struct s_vartbl));
    EXPECT_EQ(3, variables_add("_32_chars_long_67890123456789012", 0));
    EXPECT_EQ(0, memcmp("_32_chars_long_67890123456789012x", vartbl[3].name, 33));

    // Longer than maximum length name, will be stored truncated and without
    // '\0' termination.
    memset(vartbl + 4, 'y', sizeof(struct s_vartbl));
    EXPECT_EQ(4, variables_add("_33_chars_long_678901234567890123", 0));
    EXPECT_EQ(0, memcmp("_33_chars_long_67890123456789012y", vartbl[4].name, 33));

    EXPECT_EQ(5, variables_add("non_scalar_1", 10));
    EXPECT_EQ(10, memory[0]);
    EXPECT_EQ(memory, (size_t *) vartbl[5].val.s);

    EXPECT_EQ(6, variables_add("non_scalar_2", 20));
    EXPECT_EQ(20, memory[1]);
    EXPECT_EQ(memory + 1, (size_t *) vartbl[6].val.s);

    EXPECT_EQ(2, memory_count);
    EXPECT_EQ(7, varcnt);
}

TEST_F(VariablesTest, Delete) {
    const struct s_vartbl EMPTY_VAR = {};

    variables_add("foo", 0);
    variables_add("bar", 0);
    variables_add("FOO", 0);
    variables_add("_32_chars_long_67890123456789012", 0);
    variables_add("", 0);
    variables_add("non_scalar_1", 10);
    variables_add("non_scalar_2", 20);
    EXPECT_EQ(7, varcnt);

    variables_delete(2);   // "FOO"
    EXPECT_EQ(7, varcnt);  // Because we didn't delete the last variable.
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 2, sizeof(struct s_vartbl)));

    variables_delete(6);   // "non_scalar_2"
    EXPECT_EQ(6, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 6, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memory[1]);

    variables_delete(5);   // "non_scalar_1"
    EXPECT_EQ(5, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 5, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memory[0]);
}

TEST_F(VariablesTest, Find) {
    variables_add("foo", 0);
    variables_add("bar", 0);
    variables_add("FOO", 0);
    variables_add("_32_chars_long_67890123456789012", 0);
    variables_add("", 0);

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
