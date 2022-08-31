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

Options mmb_options;
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
        mmb_options.base = 0;
        vartbl = (struct s_vartbl*) calloc(MAXVARS, sizeof(struct s_vartbl));
        variables_init();
        memset(memory, 0, sizeof(memory));
        memory_count = 0;
    }

    void TearDown() override {
        free(vartbl);
    }
};

TEST_F(VariablesTest, Add_ScalarInt) {
    int idx = variables_add("global_0", T_INT, GLOBAL_VAR, NULL, 0);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(NULL, vartbl[idx].val.s);

    idx = variables_add("local_1", T_INT, 1, NULL, 0);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(NULL, vartbl[idx].val.s);

    idx = variables_add("local_2", T_INT, 2, NULL, 0);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(NULL, vartbl[idx].val.s);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, variables_free_idx);
    EXPECT_EQ(0, memory_count);
}

TEST_F(VariablesTest, Add_ScalarFloat) {
    int idx = variables_add("global_0", T_NBR, GLOBAL_VAR, NULL, 0);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(NULL, vartbl[idx].val.s);

    idx = variables_add("local_1", T_NBR, 1, NULL, 0);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(NULL, vartbl[idx].val.s);

    idx = variables_add("local_2", T_NBR, 2, NULL, 0);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(NULL, vartbl[idx].val.s);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, variables_free_idx);
    EXPECT_EQ(0, memory_count);
}

TEST_F(VariablesTest, Add_ScalarString) {
    int idx = variables_add("global_0", T_STR, GLOBAL_VAR, NULL, 255);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(255, vartbl[idx].size);
    EXPECT_EQ((char *) memory, vartbl[idx].val.s);
    EXPECT_EQ(256, memory[0]);

    idx = variables_add("local_1", T_STR, 1, NULL, 128);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(128, vartbl[idx].size);
    EXPECT_EQ((char *) (memory + 1), vartbl[idx].val.s);
    EXPECT_EQ(129, memory[1]);

    idx = variables_add("local_2", T_STR, 2, NULL, 1);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(1, vartbl[idx].size);
    EXPECT_EQ((char *) (memory + 2), vartbl[idx].val.s);
    EXPECT_EQ(2, memory[2]);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, variables_free_idx);
    EXPECT_EQ(3, memory_count);
}

TEST_F(VariablesTest, Add_ArrayInt) {
    DIMTYPE dims[MAXDIM] = { 0 };

    // 1D array with 11 elements if OPTION BASE 0.
    dims[0] = 10;
    int idx = variables_add("global_0", T_INT, GLOBAL_VAR, dims, 0);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ((MMINTEGER *) memory, vartbl[idx].val.ia);
    EXPECT_EQ((char *) memory, vartbl[idx].val.s);
    EXPECT_EQ(11 * sizeof(MMINTEGER), memory[0]);

    // 2D array with 66 elements if OPTION BASE 0.
    dims[0] = 10;
    dims[1] = 5;
    idx = variables_add("local_1", T_INT, 1, dims, 0);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].dims[2]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ((MMINTEGER *) (memory + 1), vartbl[idx].val.ia);
    EXPECT_EQ((char *) (memory + 1), vartbl[idx].val.s);
    EXPECT_EQ(66 * sizeof(MMINTEGER), memory[1]);

    // Assert that changing the local dims[] has not changed
    // vartbl[0], which should have taken an independent copy.
    EXPECT_EQ(10, vartbl[0].dims[0]);
    EXPECT_EQ(0, vartbl[0].dims[1]);

    // 3D array with 150 elements if OPTION BASE 1.
    dims[0] = 10;
    dims[1] = 5;
    dims[2] = 3;
    mmb_options.base = 1;
    idx = variables_add("local_2", T_INT, 2, dims, 0);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(3, vartbl[idx].dims[2]);
    EXPECT_EQ(0, vartbl[idx].dims[3]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ((MMINTEGER *) (memory + 2), vartbl[idx].val.ia);
    EXPECT_EQ((char *) (memory + 2), vartbl[idx].val.s);
    EXPECT_EQ(150 * sizeof(MMINTEGER), memory[2]);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, variables_free_idx);
    EXPECT_EQ(3, memory_count);
}

TEST_F(VariablesTest, Add_ArrayFloat) {
    DIMTYPE dims[MAXDIM] = { 0 };

    // 1D array with 11 elements if OPTION BASE 0.
    dims[0] = 10;
    int idx = variables_add("global_0", T_NBR, GLOBAL_VAR, dims, 0);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ((MMFLOAT *) memory, vartbl[idx].val.fa);
    EXPECT_EQ((char *) memory, vartbl[idx].val.s);
    EXPECT_EQ(11 * sizeof(MMFLOAT), memory[0]);

    // 2D array with 66 elements if OPTION BASE 0.
    dims[0] = 10;
    dims[1] = 5;
    idx = variables_add("local_1", T_NBR, 1, dims, 0);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].dims[2]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ((MMFLOAT *) (memory + 1), vartbl[idx].val.fa);
    EXPECT_EQ((char *) (memory + 1), vartbl[idx].val.s);
    EXPECT_EQ(66 * sizeof(MMFLOAT), memory[1]);

    // Assert that changing the local dims[] has not changed
    // vartbl[0], which should have taken an independent copy.
    EXPECT_EQ(10, vartbl[0].dims[0]);
    EXPECT_EQ(0, vartbl[0].dims[1]);

    // 3D array with 150 elements if OPTION BASE 1.
    dims[0] = 10;
    dims[1] = 5;
    dims[2] = 3;
    mmb_options.base = 1;
    idx = variables_add("local_2", T_NBR, 2, dims, 0);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(3, vartbl[idx].dims[2]);
    EXPECT_EQ(0, vartbl[idx].dims[3]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ((MMFLOAT *) (memory + 2), vartbl[idx].val.fa);
    EXPECT_EQ((char *) (memory + 2), vartbl[idx].val.s);
    EXPECT_EQ(150 * sizeof(MMFLOAT), memory[2]);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, variables_free_idx);
    EXPECT_EQ(3, memory_count);
}

TEST_F(VariablesTest, Add_ArrayString) {
    DIMTYPE dims[MAXDIM] = { 0 };

    // 1D array with 11 elements if OPTION BASE 0.
    dims[0] = 10;
    int idx = variables_add("global_0", T_STR, GLOBAL_VAR, dims, 255);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].dims[1]);
    EXPECT_EQ(255, vartbl[idx].size);
    EXPECT_EQ((char *) memory, vartbl[idx].val.s);
    EXPECT_EQ(11 * 256, memory[0]);

    // 2D array with 66 elements if OPTION BASE 0.
    dims[0] = 10;
    dims[1] = 5;
    idx = variables_add("local_1", T_STR, 1, dims, 128);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].dims[2]);
    EXPECT_EQ(128, vartbl[idx].size);
    EXPECT_EQ((char *) (memory + 1), vartbl[idx].val.s);
    EXPECT_EQ(66 * 129, memory[1]);

    // Assert that changing the local dims[] has not changed
    // vartbl[0], which should have taken an independent copy.
    EXPECT_EQ(10, vartbl[0].dims[0]);
    EXPECT_EQ(0, vartbl[0].dims[1]);

    // 3D array with 150 elements if OPTION BASE 1.
    dims[0] = 10;
    dims[1] = 5;
    dims[2] = 3;
    mmb_options.base = 1;
    idx = variables_add("local_2", T_STR, 2, dims, 1);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(3, vartbl[idx].dims[2]);
    EXPECT_EQ(0, vartbl[idx].dims[3]);
    EXPECT_EQ(1, vartbl[idx].size);
    EXPECT_EQ((char *) (memory + 2), vartbl[idx].val.s);
    EXPECT_EQ(150 * 2, memory[2]);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, variables_free_idx);
    EXPECT_EQ(3, memory_count);
}

TEST_F(VariablesTest, Add_GivenTooManyVariables) {
    for (int ii = 0; ii < MAXVARS; ++ii) {
        EXPECT_EQ(ii, variables_add("foo", T_INT, GLOBAL_VAR, NULL, 0));
    }

    EXPECT_EQ(-1, variables_add("bar", T_INT, GLOBAL_VAR, NULL, 0));
}

TEST_F(VariablesTest, Add_GivenMaxLengthName) {
    int idx = variables_add(
            "_32_chars_long_67890123456789012", T_INT, GLOBAL_VAR, NULL, 0);
    EXPECT_EQ(0, idx);

    // 33rd element will be the value of vartbl[idx].type.
    EXPECT_EQ(0, memcmp("_32_chars_long_67890123456789012\x4", vartbl[idx].name, 33));

    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(NULL, vartbl[idx].val.s);
}

TEST_F(VariablesTest, Add_GivenNameTooLong) {
    // Longer than maximum length name, will be stored truncated and without
    // '\0' termination.
    char silly_name[1024];
    memset(silly_name, 'a', 1023);
    silly_name[1023] = '\0';
    int idx = variables_add(silly_name, T_INT, GLOBAL_VAR, NULL, 0);
    EXPECT_EQ(0, idx);

    // 33rd element will be the value of vartbl[idx].type.
    EXPECT_EQ(
            0,
            memcmp(
                    "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\x4",
                    vartbl[idx].name,
                    33));

    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(NULL, vartbl[idx].val.s);

    // Check we did not overrun into the next element.
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 1, sizeof(struct s_vartbl)));
}

TEST_F(VariablesTest, Add_GivenEightDimensionalArray) {
    DIMTYPE dims[MAXDIM] = { 2, 3, 4, 5, 6, 7, 8, 9 };
    int idx = variables_add("global_0", T_INT, GLOBAL_VAR, dims, 0);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(2, vartbl[idx].dims[0]);
    EXPECT_EQ(3, vartbl[idx].dims[1]);
    EXPECT_EQ(4, vartbl[idx].dims[2]);
    EXPECT_EQ(5, vartbl[idx].dims[3]);
    EXPECT_EQ(6, vartbl[idx].dims[4]);
    EXPECT_EQ(7, vartbl[idx].dims[5]);
    EXPECT_EQ(8, vartbl[idx].dims[6]);
    EXPECT_EQ(9, vartbl[idx].dims[7]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ((MMINTEGER *) memory, vartbl[idx].val.ia);
    EXPECT_EQ((char *) memory, vartbl[idx].val.s);

    // OPTION BASE 0, so add one to each bound to determine number of elements.
    EXPECT_EQ(3 * 4 * 5 * 6 * 7 * 8 * 9 * 10 * sizeof(MMINTEGER), memory[0]);
}

TEST_F(VariablesTest, Add_GivenInvalidDimensions) {
    {
        DIMTYPE dims[MAXDIM] = { -2, 0, 0, 0, 0, 0, 0, 0 };
        int idx = variables_add("global_0", T_INT, GLOBAL_VAR, dims, 0);
        EXPECT_EQ(-2, idx);
        EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(s_vartbl)));
        EXPECT_EQ(0, memory_count);
    }

    {
        DIMTYPE dims[MAXDIM] = { 10, -1, 0, 0, 0, 0, 0, 0 };
        int idx = variables_add("global_0", T_INT, GLOBAL_VAR, dims, 0);
        EXPECT_EQ(-2, idx);
        EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(s_vartbl)));
        EXPECT_EQ(0, memory_count);
    }
}

TEST_F(VariablesTest, Add_GivenEmptyArray) {
    // An "empty" array is indicated by having the first dimension be -1
    // (and the others 0).
    DIMTYPE dims[MAXDIM] = { -1, 0, 0, 0, 0, 0, 0, 0 };
    int idx = variables_add("global_0", T_INT, GLOBAL_VAR, dims, 0);
    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(-1, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(NULL, vartbl[idx].val.s);
    EXPECT_EQ(1, varcnt);
    EXPECT_EQ(1, variables_free_idx);
    EXPECT_EQ(0, memory_count);
}

TEST_F(VariablesTest, Delete) {
    variables_add("foo", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("bar", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("FOO", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add(
            "_32_chars_long_67890123456789012", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("non_scalar_1", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("non_scalar_2", T_INT, GLOBAL_VAR, NULL, 0);
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
    variables_add("global_a", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("local_1_a", T_INT, 1, NULL, 0);
    variables_add("local_2_a", T_INT, 2, NULL, 0);
    variables_add("global_b", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("local_1_b", T_INT, 1, NULL, 0);
    variables_add("local_2_b", T_INT, 2, NULL, 0);

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
    variables_add("local_2_a", T_INT, 2, NULL, 0);
    variables_add("local_2_b", T_INT, 2, NULL, 0);

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
    variables_add("local_1_a", T_INT, 1, NULL, 0);
    variables_add("local_1_b", T_INT, 1, NULL, 0);
    variables_add("local_2_a", T_INT, 2, NULL, 0);
    variables_add("local_2_b", T_INT, 2, NULL, 0);

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
    variables_add("global_a", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("local_1_a", T_INT, 1, NULL, 0);
    variables_add("local_2_a", T_INT, 2, NULL, 0);
    variables_add("global_b", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("local_1_b", T_INT, 1, NULL, 0);
    variables_add("local_2_b", T_INT, 2, NULL, 0);

    EXPECT_EQ(6, varcnt);
    EXPECT_STREQ("global_a",  vartbl[0].name);
    EXPECT_STREQ("local_1_a", vartbl[1].name);
    EXPECT_STREQ("local_2_a", vartbl[2].name);
    EXPECT_STREQ("global_b",  vartbl[3].name);
    EXPECT_STREQ("local_1_b", vartbl[4].name);
    EXPECT_STREQ("local_2_b", vartbl[5].name);
}

TEST_F(VariablesTest, Find) {
    variables_add("foo", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("bar", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("FOO", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add(
            "_32_chars_long_67890123456789012", T_INT, GLOBAL_VAR, NULL, 0);
    variables_add("", T_INT, GLOBAL_VAR, NULL, 0);

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
