/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

#include "test_config.h"

extern "C" {

#include "../mmb4l.h"
#include "../variables.h"

const struct s_vartbl EMPTY_VAR = {};

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
        variables_init();
        memset(memory, 0, sizeof(memory));
        memory_count = 0;
    }

    void TearDown() override {
        free(vartbl);
    }
};

TEST_F(VariablesTest, Add) {
    EXPECT_EQ(0, variables_add("global", GLOBAL_VAR, 0));
    EXPECT_STREQ("global", vartbl[0].name);
    EXPECT_EQ(T_INT, vartbl[0].type);
    EXPECT_EQ(0, vartbl[0].level);

    EXPECT_EQ(1, variables_add("local_1", 1, 0));
    EXPECT_STREQ("local_1", vartbl[1].name);
    EXPECT_EQ(T_INT, vartbl[1].type);
    EXPECT_EQ(1, vartbl[1].level);

    EXPECT_EQ(2, variables_add("local_2", 2, 0));
    EXPECT_STREQ("local_2", vartbl[2].name);
    EXPECT_EQ(T_INT, vartbl[2].type);
    EXPECT_EQ(2, vartbl[2].level);

    // Maximum length name, will be stored without '\0' termination.
    //memset(vartbl + 3, 'x', sizeof(struct s_vartbl));
    EXPECT_EQ(3, variables_add("_32_chars_long_67890123456789012", GLOBAL_VAR, 0));
    //EXPECT_EQ(0, memcmp("_32_chars_long_67890123456789012x", vartbl[3].name, 33));
    EXPECT_EQ(T_INT, vartbl[3].type);
    EXPECT_EQ(0, vartbl[3].level);

    // Longer than maximum length name, will be stored truncated and without
    // '\0' termination.
    //memset(vartbl + 4, 'y', sizeof(struct s_vartbl));
    EXPECT_EQ(4, variables_add("_33_chars_long_678901234567890123", GLOBAL_VAR, 0));
    //EXPECT_EQ(0, memcmp("_33_chars_long_67890123456789012y", vartbl[4].name, 33));
    EXPECT_EQ(T_INT, vartbl[4].type);
    EXPECT_EQ(0, vartbl[4].level);

    EXPECT_EQ(5, variables_add("non_scalar_1", GLOBAL_VAR, 10));
    EXPECT_EQ(10, memory[0]);
    EXPECT_EQ(memory, (size_t *) vartbl[5].val.s);
    EXPECT_EQ(T_INT, vartbl[5].type);
    EXPECT_EQ(0, vartbl[5].level);

    EXPECT_EQ(6, variables_add("non_scalar_2", GLOBAL_VAR, 20));
    EXPECT_EQ(20, memory[1]);
    EXPECT_EQ(memory + 1, (size_t *) vartbl[6].val.s);
    EXPECT_EQ(T_INT, vartbl[6].type);
    EXPECT_EQ(0, vartbl[6].level);

    EXPECT_EQ(2, memory_count);
    EXPECT_EQ(7, varcnt);
    EXPECT_EQ(7, variables_free_idx);
}

TEST_F(VariablesTest, Add_GivenTooManyVariables) {
    for (int ii = 0; ii < MAXVARS; ++ii) {
        EXPECT_EQ(ii, variables_add("foo", GLOBAL_VAR, 0));
    }

    EXPECT_EQ(-1, variables_add("bar", GLOBAL_VAR, 0));
}

TEST_F(VariablesTest, Delete) {
    variables_add("foo", GLOBAL_VAR, 0);
    variables_add("bar", GLOBAL_VAR, 0);
    variables_add("FOO", GLOBAL_VAR, 0);
    variables_add("_32_chars_long_67890123456789012", GLOBAL_VAR, 0);
    variables_add("", GLOBAL_VAR, 0);
    variables_add("non_scalar_1", GLOBAL_VAR, 10);
    variables_add("non_scalar_2", GLOBAL_VAR, 20);
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

TEST_F(VariablesTest, DeleteAll) {
    // Fill the table.
    variables_add("global_a", GLOBAL_VAR, 0);
    variables_add("local_1_a", 1, 0);
    variables_add("local_2_a", 2, 0);
    variables_add("global_b", GLOBAL_VAR, 0);
    variables_add("local_1_b", 1, 0);
    variables_add("local_2_b", 2, 0);

    EXPECT_EQ(6, varcnt);
    EXPECT_STREQ("global_a",  vartbl[0].name);
    EXPECT_STREQ("local_1_a", vartbl[1].name);
    EXPECT_STREQ("local_2_a", vartbl[2].name);
    EXPECT_STREQ("global_b",  vartbl[3].name);
    EXPECT_STREQ("local_1_b", vartbl[4].name);
    EXPECT_STREQ("local_2_b", vartbl[5].name);

    // Delete level 2 and above.
    variables_delete_all(2);

    EXPECT_EQ(5, varcnt);  // 'varcnt' is not actually the variable count,
                           // it is the index + 1 of the last used slot.
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 0, sizeof(struct s_vartbl)));
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 1, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 2, sizeof(struct s_vartbl)));
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 3, sizeof(struct s_vartbl)));
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 4, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 5, sizeof(struct s_vartbl)));

    // Refill the table.
    variables_add("local_2_a", 2, 0);
    variables_add("local_2_b", 2, 0);

    EXPECT_EQ(6, varcnt);
    EXPECT_STREQ("local_2_a", vartbl[2].name);
    EXPECT_STREQ("local_2_b", vartbl[5].name);

    // Delete level 1 and above.
    variables_delete_all(1);

    EXPECT_EQ(4, varcnt);  // 'varcnt' is not actually the variable count,
                           // it is the index + 1 of the last used slot.
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 0, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 1, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 2, sizeof(struct s_vartbl)));
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 3, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 4, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 5, sizeof(struct s_vartbl)));

    // Refill the table.
    variables_add("local_1_a", 1, 0);
    variables_add("local_1_b", 1, 0);
    variables_add("local_2_a", 2, 0);
    variables_add("local_2_b", 2, 0);

    EXPECT_EQ(6, varcnt);
    EXPECT_STREQ("local_1_a", vartbl[1].name);
    EXPECT_STREQ("local_1_b", vartbl[2].name);
    EXPECT_STREQ("local_2_a", vartbl[4].name);
    EXPECT_STREQ("local_2_b", vartbl[5].name);

    // Delete ALL variables.
    variables_delete_all(0);

    EXPECT_EQ(0, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 0, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 1, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 2, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 3, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 4, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 5, sizeof(struct s_vartbl)));

    // Refill the table.
    variables_add("global_a", GLOBAL_VAR, 0);
    variables_add("local_1_a", 1, 0);
    variables_add("local_2_a", 2, 0);
    variables_add("global_b", GLOBAL_VAR, 0);
    variables_add("local_1_b", 1, 0);
    variables_add("local_2_b", 2, 0);

    EXPECT_EQ(6, varcnt);
    EXPECT_STREQ("global_a",  vartbl[0].name);
    EXPECT_STREQ("local_1_a", vartbl[1].name);
    EXPECT_STREQ("local_2_a", vartbl[2].name);
    EXPECT_STREQ("global_b",  vartbl[3].name);
    EXPECT_STREQ("local_1_b", vartbl[4].name);
    EXPECT_STREQ("local_2_b", vartbl[5].name);
}

TEST_F(VariablesTest, Find) {
    variables_add("foo", GLOBAL_VAR, 0);
    variables_add("bar", GLOBAL_VAR, 0);
    variables_add("FOO", GLOBAL_VAR, 0);
    variables_add("_32_chars_long_67890123456789012", GLOBAL_VAR, 0);
    variables_add("", GLOBAL_VAR, 0);

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
