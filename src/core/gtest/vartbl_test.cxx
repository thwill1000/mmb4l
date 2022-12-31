/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

#include "../../common/gtest/test_config.h"

extern "C" {

#include "../vartbl.h"
#include "../../common/hash.h"
#include "../../common/mmb4l.h"

const struct s_vartbl EMPTY_VAR = {};

Options mmb_options;
struct s_vartbl vartbl[MAXVARS];
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

#define vartbl_TEST_DIR  TMP_DIR "/VartblTest"

class VartblTest : public ::testing::Test {

protected:

    VarHashValue GLOBAL_0_HASH;
    VarHashValue LOCAL_1_HASH;
    VarHashValue LOCAL_2_HASH;

    void SetUp() override {
        mmb_options.base = 0;
        vartbl_init_called = false;
        vartbl_init();
        memset(memory, 0, sizeof(memory));
        memory_count = 0;

        GLOBAL_0_HASH = hash_cstring("global_0", MAXVARLEN) % VARS_HASHMAP_SIZE;
        LOCAL_1_HASH = hash_cstring("local_1", MAXVARLEN) % VARS_HASHMAP_SIZE;
        LOCAL_2_HASH = hash_cstring("local_2", MAXVARLEN) % VARS_HASHMAP_SIZE;
    }

    void TearDown() override {
    }
};

TEST_F(VartblTest, Add_ScalarInt) {
    int idx = vartbl_add("global_0", T_INT, GLOBAL_VAR, NULL, 0);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(GLOBAL_0_HASH, vartbl[idx].hash);
    EXPECT_EQ(NULL, vartbl[idx].val.s);
    EXPECT_EQ(0, vartbl_hashmap[GLOBAL_0_HASH]);

    idx = vartbl_add("local_1", T_INT, 1, NULL, 0);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(LOCAL_1_HASH, vartbl[idx].hash);
    EXPECT_EQ(NULL, vartbl[idx].val.s);
    EXPECT_EQ(1, vartbl_hashmap[LOCAL_1_HASH]);

    idx = vartbl_add("local_2", T_INT, 2, NULL, 0);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(LOCAL_2_HASH, vartbl[idx].hash);
    EXPECT_EQ(NULL, vartbl[idx].val.s);
    EXPECT_EQ(2, vartbl_hashmap[LOCAL_2_HASH]);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, vartbl_free_idx);
    EXPECT_EQ(0, memory_count);
}

TEST_F(VartblTest, Add_ScalarFloat) {
    int idx = vartbl_add("global_0", T_NBR, GLOBAL_VAR, NULL, 0);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(hash_cstring("global_0", MAXVARLEN) % VARS_HASHMAP_SIZE, vartbl[idx].hash);
    EXPECT_EQ(NULL, vartbl[idx].val.s);
    EXPECT_EQ(0, vartbl_hashmap[GLOBAL_0_HASH]);

    idx = vartbl_add("local_1", T_NBR, 1, NULL, 0);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(LOCAL_1_HASH, vartbl[idx].hash);
    EXPECT_EQ(NULL, vartbl[idx].val.s);
    EXPECT_EQ(1, vartbl_hashmap[LOCAL_1_HASH]);

    idx = vartbl_add("local_2", T_NBR, 2, NULL, 0);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(LOCAL_2_HASH, vartbl[idx].hash);
    EXPECT_EQ(NULL, vartbl[idx].val.s);
    EXPECT_EQ(2, vartbl_hashmap[LOCAL_2_HASH]);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, vartbl_free_idx);
    EXPECT_EQ(0, memory_count);
}

TEST_F(VartblTest, Add_ScalarString) {
    int idx = vartbl_add("global_0", T_STR, GLOBAL_VAR, NULL, 255);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(255, vartbl[idx].size);
    EXPECT_EQ(hash_cstring("global_0", MAXVARLEN) % VARS_HASHMAP_SIZE, vartbl[idx].hash);
    EXPECT_EQ((char *) memory, vartbl[idx].val.s);
    EXPECT_EQ(0, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(256, memory[0]);

    idx = vartbl_add("local_1", T_STR, 1, NULL, 128);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(128, vartbl[idx].size);
    EXPECT_EQ(LOCAL_1_HASH, vartbl[idx].hash);
    EXPECT_EQ((char *) (memory + 1), vartbl[idx].val.s);
    EXPECT_EQ(1, vartbl_hashmap[LOCAL_1_HASH]);
    EXPECT_EQ(129, memory[1]);

    idx = vartbl_add("local_2", T_STR, 2, NULL, 1);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(1, vartbl[idx].size);
    EXPECT_EQ(LOCAL_2_HASH, vartbl[idx].hash);
    EXPECT_EQ((char *) (memory + 2), vartbl[idx].val.s);
    EXPECT_EQ(2, vartbl_hashmap[LOCAL_2_HASH]);
    EXPECT_EQ(2, memory[2]);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, vartbl_free_idx);
    EXPECT_EQ(3, memory_count);
}

TEST_F(VartblTest, Add_ArrayInt) {
    DIMTYPE dims[MAXDIM] = { 0 };

    // 1D array with 11 elements if OPTION BASE 0.
    dims[0] = 10;
    int idx = vartbl_add("global_0", T_INT, GLOBAL_VAR, dims, 0);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(hash_cstring("global_0", MAXVARLEN) % VARS_HASHMAP_SIZE, vartbl[idx].hash);
    EXPECT_EQ((MMINTEGER *) memory, vartbl[idx].val.ia);
    EXPECT_EQ((char *) memory, vartbl[idx].val.s);
    EXPECT_EQ(0, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(11 * sizeof(MMINTEGER), memory[0]);

    // 2D array with 66 elements if OPTION BASE 0.
    dims[0] = 10;
    dims[1] = 5;
    idx = vartbl_add("local_1", T_INT, 1, dims, 0);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].dims[2]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(LOCAL_1_HASH, vartbl[idx].hash);
    EXPECT_EQ((MMINTEGER *) (memory + 1), vartbl[idx].val.ia);
    EXPECT_EQ((char *) (memory + 1), vartbl[idx].val.s);
    EXPECT_EQ(1, vartbl_hashmap[LOCAL_1_HASH]);
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
    idx = vartbl_add("local_2", T_INT, 2, dims, 0);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(3, vartbl[idx].dims[2]);
    EXPECT_EQ(0, vartbl[idx].dims[3]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(LOCAL_2_HASH, vartbl[idx].hash);
    EXPECT_EQ((MMINTEGER *) (memory + 2), vartbl[idx].val.ia);
    EXPECT_EQ((char *) (memory + 2), vartbl[idx].val.s);
    EXPECT_EQ(2, vartbl_hashmap[LOCAL_2_HASH]);
    EXPECT_EQ(150 * sizeof(MMINTEGER), memory[2]);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, vartbl_free_idx);
    EXPECT_EQ(3, memory_count);
}

TEST_F(VartblTest, Add_ArrayFloat) {
    DIMTYPE dims[MAXDIM] = { 0 };

    // 1D array with 11 elements if OPTION BASE 0.
    dims[0] = 10;
    int idx = vartbl_add("global_0", T_NBR, GLOBAL_VAR, dims, 0);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(hash_cstring("global_0", MAXVARLEN) % VARS_HASHMAP_SIZE, vartbl[idx].hash);
    EXPECT_EQ((MMFLOAT *) memory, vartbl[idx].val.fa);
    EXPECT_EQ((char *) memory, vartbl[idx].val.s);
    EXPECT_EQ(0, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(11 * sizeof(MMFLOAT), memory[0]);

    // 2D array with 66 elements if OPTION BASE 0.
    dims[0] = 10;
    dims[1] = 5;
    idx = vartbl_add("local_1", T_NBR, 1, dims, 0);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].dims[2]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(LOCAL_1_HASH, vartbl[idx].hash);
    EXPECT_EQ((MMFLOAT *) (memory + 1), vartbl[idx].val.fa);
    EXPECT_EQ((char *) (memory + 1), vartbl[idx].val.s);
    EXPECT_EQ(1, vartbl_hashmap[LOCAL_1_HASH]);
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
    idx = vartbl_add("local_2", T_NBR, 2, dims, 0);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_NBR, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(3, vartbl[idx].dims[2]);
    EXPECT_EQ(0, vartbl[idx].dims[3]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(LOCAL_2_HASH, vartbl[idx].hash);
    EXPECT_EQ((MMFLOAT *) (memory + 2), vartbl[idx].val.fa);
    EXPECT_EQ((char *) (memory + 2), vartbl[idx].val.s);
    EXPECT_EQ(2, vartbl_hashmap[LOCAL_2_HASH]);
    EXPECT_EQ(150 * sizeof(MMFLOAT), memory[2]);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, vartbl_free_idx);
    EXPECT_EQ(3, memory_count);
}

TEST_F(VartblTest, Add_ArrayString) {
    DIMTYPE dims[MAXDIM] = { 0 };

    // 1D array with 11 elements if OPTION BASE 0.
    dims[0] = 10;
    int idx = vartbl_add("global_0", T_STR, GLOBAL_VAR, dims, 255);

    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].dims[1]);
    EXPECT_EQ(255, vartbl[idx].size);
    EXPECT_EQ(hash_cstring("global_0", MAXVARLEN) % VARS_HASHMAP_SIZE, vartbl[idx].hash);
    EXPECT_EQ((char *) memory, vartbl[idx].val.s);
    EXPECT_EQ(0, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(11 * 256, memory[0]);

    // 2D array with 66 elements if OPTION BASE 0.
    dims[0] = 10;
    dims[1] = 5;
    idx = vartbl_add("local_1", T_STR, 1, dims, 128);

    EXPECT_EQ(1, idx);
    EXPECT_STREQ("local_1", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(1, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].dims[2]);
    EXPECT_EQ(128, vartbl[idx].size);
    EXPECT_EQ(LOCAL_1_HASH, vartbl[idx].hash);
    EXPECT_EQ((char *) (memory + 1), vartbl[idx].val.s);
    EXPECT_EQ(1, vartbl_hashmap[LOCAL_1_HASH]);
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
    idx = vartbl_add("local_2", T_STR, 2, dims, 1);

    EXPECT_EQ(2, idx);
    EXPECT_STREQ("local_2", vartbl[idx].name);
    EXPECT_EQ(T_STR, vartbl[idx].type);
    EXPECT_EQ(2, vartbl[idx].level);
    EXPECT_EQ(10, vartbl[idx].dims[0]);
    EXPECT_EQ(5, vartbl[idx].dims[1]);
    EXPECT_EQ(3, vartbl[idx].dims[2]);
    EXPECT_EQ(0, vartbl[idx].dims[3]);
    EXPECT_EQ(1, vartbl[idx].size);
    EXPECT_EQ(LOCAL_2_HASH, vartbl[idx].hash);
    EXPECT_EQ((char *) (memory + 2), vartbl[idx].val.s);
    EXPECT_EQ(2, vartbl_hashmap[LOCAL_2_HASH]);
    EXPECT_EQ(150 * 2, memory[2]);

    EXPECT_EQ(3, varcnt);
    EXPECT_EQ(3, vartbl_free_idx);
    EXPECT_EQ(3, memory_count);
}

TEST_F(VartblTest, Add_GivenTooManyVariables) {
    for (int ii = 0; ii < MAXVARS; ++ii) {
        EXPECT_EQ(ii, vartbl_add("foo", T_INT, GLOBAL_VAR, NULL, 0));
    }

    EXPECT_EQ(-1, vartbl_add("bar", T_INT, GLOBAL_VAR, NULL, 0));
}

TEST_F(VartblTest, Add_GivenMaxLengthName) {
    int idx = vartbl_add(
            "_32_chars_long_67890123456789012", T_INT, GLOBAL_VAR, NULL, 0);
    EXPECT_EQ(0, idx);

    // 33rd element will be the value of vartbl[idx].type.
    EXPECT_EQ(0, memcmp("_32_chars_long_67890123456789012\x4", vartbl[idx].name, 33));

    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(0, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].size);
    VarHashValue expected_hash =
            hash_cstring("_32_chars_long_67890123456789012", MAXVARLEN) % VARS_HASHMAP_SIZE;
    EXPECT_EQ(expected_hash, vartbl[idx].hash);
    EXPECT_EQ(NULL, vartbl[idx].val.s);
    EXPECT_EQ(0, vartbl_hashmap[expected_hash]);
}

TEST_F(VartblTest, Add_GivenNameTooLong) {
    // Longer than maximum length name, will be stored truncated and without
    // '\0' termination.
    char silly_name[1024];
    memset(silly_name, 'a', 1023);
    silly_name[1023] = '\0';
    int idx = vartbl_add(silly_name, T_INT, GLOBAL_VAR, NULL, 0);
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
    VarHashValue expected_hash =
            hash_cstring("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", MAXVARLEN) % VARS_HASHMAP_SIZE;
    EXPECT_EQ(expected_hash, vartbl[idx].hash);
    EXPECT_EQ(NULL, vartbl[idx].val.s);
    EXPECT_EQ(0, vartbl_hashmap[expected_hash]);

    // Check we did not overrun into the next element.
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 1, sizeof(struct s_vartbl)));
}

TEST_F(VartblTest, Add_GivenEightDimensionalArray) {
    DIMTYPE dims[MAXDIM] = { 2, 3, 4, 5, 6, 7, 8, 9 };
    int idx = vartbl_add("global_0", T_INT, GLOBAL_VAR, dims, 0);

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
    EXPECT_EQ(GLOBAL_0_HASH, vartbl[idx].hash);
    EXPECT_EQ((MMINTEGER *) memory, vartbl[idx].val.ia);
    EXPECT_EQ((char *) memory, vartbl[idx].val.s);
    EXPECT_EQ(0, vartbl_hashmap[GLOBAL_0_HASH]);

    // OPTION BASE 0, so add one to each bound to determine number of elements.
    EXPECT_EQ(3 * 4 * 5 * 6 * 7 * 8 * 9 * 10 * sizeof(MMINTEGER), memory[0]);
}

TEST_F(VartblTest, Add_GivenInvalidDimensions) {
    {
        DIMTYPE dims[MAXDIM] = { -2, 0, 0, 0, 0, 0, 0, 0 };
        int idx = vartbl_add("global_0", T_INT, GLOBAL_VAR, dims, 0);
        EXPECT_EQ(-2, idx);
        EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(s_vartbl)));
        EXPECT_EQ(0, memory_count);
    }

    {
        DIMTYPE dims[MAXDIM] = { 10, -1, 0, 0, 0, 0, 0, 0 };
        int idx = vartbl_add("global_0", T_INT, GLOBAL_VAR, dims, 0);
        EXPECT_EQ(-2, idx);
        EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(s_vartbl)));
        EXPECT_EQ(0, memory_count);
    }
}

TEST_F(VartblTest, Add_GivenEmptyArray) {
    // An "empty" array is indicated by having the first dimension be -1
    // (and the others 0).
    DIMTYPE dims[MAXDIM] = { -1, 0, 0, 0, 0, 0, 0, 0 };
    int idx = vartbl_add("global_0", T_INT, GLOBAL_VAR, dims, 0);
    EXPECT_EQ(0, idx);
    EXPECT_STREQ("global_0", vartbl[idx].name);
    EXPECT_EQ(T_INT, vartbl[idx].type);
    EXPECT_EQ(0, vartbl[idx].level);
    EXPECT_EQ(-1, vartbl[idx].dims[0]);
    EXPECT_EQ(0, vartbl[idx].dims[1]);
    EXPECT_EQ(0, vartbl[idx].size);
    EXPECT_EQ(GLOBAL_0_HASH, vartbl[idx].hash);
    EXPECT_EQ(NULL, vartbl[idx].val.s);
    EXPECT_EQ(0, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(1, varcnt);
    EXPECT_EQ(1, vartbl_free_idx);
    EXPECT_EQ(0, memory_count);
}

TEST_F(VartblTest, Add_GivenFirstChoiceHashmapSlotOccupied) {
    vartbl_hashmap[GLOBAL_0_HASH] = 999; // Dummy value.

    int idx = vartbl_add("global_0", T_INT, GLOBAL_VAR, NULL, 0);

    // Expect variable idx to be stored in next slot.
    EXPECT_EQ(GLOBAL_0_HASH + 1, vartbl[idx].hash);
    EXPECT_EQ(0, vartbl_hashmap[GLOBAL_0_HASH + 1]);
}

TEST_F(VartblTest, Add_GivenHashmapWrapAround) {
    for (int ii = GLOBAL_0_HASH; ii < VARS_HASHMAP_SIZE; ++ii) {
        vartbl_hashmap[ii] = 999; // Dummy value.
    }

    int idx = vartbl_add("global_0", T_INT, GLOBAL_VAR, NULL, 0);

    // Expect variable idx to be stored in 0 slot.
    EXPECT_EQ(0, vartbl[idx].hash);
    EXPECT_EQ(0, vartbl_hashmap[0]);
}

TEST_F(VartblTest, Add_GivenNoFreeHashmapSlots) {
    for (int ii = 0; ii < VARS_HASHMAP_SIZE; ++ii) {
        vartbl_hashmap[ii] = 999; // Dummy value.
    }

    int idx = vartbl_add("global_0", T_INT, GLOBAL_VAR, NULL, 0);

    EXPECT_EQ(-3, idx);
}

TEST_F(VartblTest, Add_ReusesDeletedHashmapSlots) {
    const VarHashValue foo_hash = hash_cstring("foo", MAXVARLEN) % VARS_HASHMAP_SIZE;
    int global_0 = vartbl_add("foo", T_INT, GLOBAL_VAR, NULL, 0);
    int local_1 = vartbl_add("foo", T_INT, 1, NULL, 0);
    int local_2 = vartbl_add("foo", T_INT, 2, NULL, 0);
    vartbl_delete(local_1);

    int local_3 = vartbl_add("foo", T_INT, 3, NULL, 0);

    EXPECT_EQ(1, local_3);
    EXPECT_EQ(foo_hash + 1, vartbl[local_3].hash);
}

TEST_F(VartblTest, Delete_GivenScalarInt) {
    (void) vartbl_add("global_0", T_INT, GLOBAL_VAR, NULL, 0);
    memory[0] = 999;

    vartbl_delete(0);

    EXPECT_EQ(0, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(struct s_vartbl)));
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(999, memory[0]); // i.e. unchanged by calling vartbl_delete().
}

TEST_F(VartblTest, Delete_GivenScalarFloat) {
    (void) vartbl_add("global_0", T_NBR, GLOBAL_VAR, NULL, 0);
    memory[0] = 999;

    vartbl_delete(0);

    EXPECT_EQ(0, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(struct s_vartbl)));
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(999, memory[0]); // i.e. unchanged by calling vartbl_delete().
}

TEST_F(VartblTest, Delete_GivenScalarString) {
    (void) vartbl_add("global_0", T_STR, GLOBAL_VAR, NULL, 255);
    EXPECT_EQ(256, memory[0]);

    vartbl_delete(0);

    EXPECT_EQ(0, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(struct s_vartbl)));
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(0, memory[0]);
}

TEST_F(VartblTest, Delete_GivenIntArray) {
    DIMTYPE dims[MAXDIM] = { 2, 3, 4, 0, 0, 0, 0, 0 };
    (void) vartbl_add("global_0", T_INT, GLOBAL_VAR, dims, 0);
    EXPECT_EQ(3 * 4 * 5 * sizeof(MMINTEGER), memory[0]);

    vartbl_delete(0);

    EXPECT_EQ(0, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(struct s_vartbl)));
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(0, memory[0]);
}

TEST_F(VartblTest, Delete_GivenFloatArray) {
    DIMTYPE dims[MAXDIM] = { 2, 3, 4, 0, 0, 0, 0, 0 };
    (void) vartbl_add("global_0", T_NBR, GLOBAL_VAR, dims, 0);
    EXPECT_EQ(3 * 4 * 5 * sizeof(MMFLOAT), memory[0]);

    vartbl_delete(0);

    EXPECT_EQ(0, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(struct s_vartbl)));
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(0, memory[0]);
}

TEST_F(VartblTest, Delete_GivenStringArray) {
    DIMTYPE dims[MAXDIM] = { 2, 3, 4, 0, 0, 0, 0, 0 };
    (void) vartbl_add("global_0", T_STR, GLOBAL_VAR, dims, 255);
    EXPECT_EQ(3 * 4 * 5 * 256, memory[0]);

    vartbl_delete(0);

    EXPECT_EQ(0, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(struct s_vartbl)));
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(0, memory[0]);
}

TEST_F(VartblTest, Delete_GivenPointerToArray_DoesNotFreeMemory) {
    DIMTYPE dims[MAXDIM] = { 2, 3, 4, 0, 0, 0, 0, 0 };
    (void) vartbl_add("global_0", T_INT, GLOBAL_VAR, dims, 0);
    EXPECT_EQ(3 * 4 * 5 * sizeof(MMINTEGER), memory[0]);
    vartbl[0].type |= T_PTR;

    vartbl_delete(0);

    EXPECT_EQ(0, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(struct s_vartbl)));
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(3 * 4 * 5 * sizeof(MMINTEGER), memory[0]);
}

TEST_F(VartblTest, Delete_GivenPointerToString) {
    (void) vartbl_add("global_0", T_STR, GLOBAL_VAR, NULL, 255);
    EXPECT_EQ(256, memory[0]);
    vartbl[0].type |= T_PTR;

    vartbl_delete(0);

    EXPECT_EQ(0, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(struct s_vartbl)));
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(256, memory[0]);
}

TEST_F(VartblTest, Delete_OnlyDecrementsVarCntWhenLastVariableDeleted) {
    vartbl_add("global_0", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("local_1", T_INT, 1, NULL, 0);
    vartbl_add("local_2", T_INT, 2, NULL, 0);
    EXPECT_EQ(3, varcnt);

    vartbl_delete(1);   // "local_1"
    EXPECT_EQ(3, varcnt);  // Is not decremented.
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[LOCAL_1_HASH]);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 1, sizeof(struct s_vartbl)));

    vartbl_delete(2);   // "local_2"
    EXPECT_EQ(2, varcnt);  // Decrement because we deleted last variable.
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[LOCAL_2_HASH]);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 2, sizeof(struct s_vartbl)));

    vartbl_delete(0);   // "global_0"
    EXPECT_EQ(2, varcnt);  // Is not decremented.
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl, sizeof(struct s_vartbl)));
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[GLOBAL_0_HASH]);
    EXPECT_EQ(0, memory[0]);
}

TEST_F(VartblTest, DeleteAll) {
    // Fill the table.
    vartbl_add("global_a", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("local_1_a", T_INT, 1, NULL, 0);
    vartbl_add("local_2_a", T_INT, 2, NULL, 0);
    vartbl_add("global_b", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("local_1_b", T_INT, 1, NULL, 0);
    vartbl_add("local_2_b", T_INT, 2, NULL, 0);

    EXPECT_EQ(6, varcnt);
    EXPECT_STREQ("global_a",  vartbl[0].name);
    EXPECT_STREQ("local_1_a", vartbl[1].name);
    EXPECT_STREQ("local_2_a", vartbl[2].name);
    EXPECT_STREQ("global_b",  vartbl[3].name);
    EXPECT_STREQ("local_1_b", vartbl[4].name);
    EXPECT_STREQ("local_2_b", vartbl[5].name);

    // Delete level 2 and above.
    vartbl_delete_all(2);

    EXPECT_EQ(5, varcnt);  // 'varcnt' is not actually the variable count,
                           // it is the index + 1 of the last used slot.
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 0, sizeof(struct s_vartbl)));
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 1, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 2, sizeof(struct s_vartbl)));
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 3, sizeof(struct s_vartbl)));
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 4, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 5, sizeof(struct s_vartbl)));

    // Refill the table.
    vartbl_add("local_2_a", T_INT, 2, NULL, 0);
    vartbl_add("local_2_b", T_INT, 2, NULL, 0);

    EXPECT_EQ(6, varcnt);
    EXPECT_STREQ("local_2_a", vartbl[2].name);
    EXPECT_STREQ("local_2_b", vartbl[5].name);

    // Delete level 1 and above.
    vartbl_delete_all(1);

    EXPECT_EQ(4, varcnt);  // 'varcnt' is not actually the variable count,
                           // it is the index + 1 of the last used slot.
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 0, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 1, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 2, sizeof(struct s_vartbl)));
    EXPECT_NE(0, memcmp(&EMPTY_VAR, vartbl + 3, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 4, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 5, sizeof(struct s_vartbl)));

    // Refill the table.
    vartbl_add("local_1_a", T_INT, 1, NULL, 0);
    vartbl_add("local_1_b", T_INT, 1, NULL, 0);
    vartbl_add("local_2_a", T_INT, 2, NULL, 0);
    vartbl_add("local_2_b", T_INT, 2, NULL, 0);

    EXPECT_EQ(6, varcnt);
    EXPECT_STREQ("local_1_a", vartbl[1].name);
    EXPECT_STREQ("local_1_b", vartbl[2].name);
    EXPECT_STREQ("local_2_a", vartbl[4].name);
    EXPECT_STREQ("local_2_b", vartbl[5].name);

    // Delete ALL variables.
    vartbl_delete_all(0);

    EXPECT_EQ(0, varcnt);
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 0, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 1, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 2, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 3, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 4, sizeof(struct s_vartbl)));
    EXPECT_EQ(0, memcmp(&EMPTY_VAR, vartbl + 5, sizeof(struct s_vartbl)));
    for (int ii = 0; ii < VARS_HASHMAP_SIZE; ++ii) {
        EXPECT_TRUE(vartbl_hashmap[ii] == DELETED_HASH
                || vartbl_hashmap[ii] == UNUSED_HASH);
    }

    // Refill the table.
    vartbl_add("global_a", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("local_1_a", T_INT, 1, NULL, 0);
    vartbl_add("local_2_a", T_INT, 2, NULL, 0);
    vartbl_add("global_b", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("local_1_b", T_INT, 1, NULL, 0);
    vartbl_add("local_2_b", T_INT, 2, NULL, 0);

    EXPECT_EQ(6, varcnt);
    EXPECT_STREQ("global_a",  vartbl[0].name);
    EXPECT_STREQ("local_1_a", vartbl[1].name);
    EXPECT_STREQ("local_2_a", vartbl[2].name);
    EXPECT_STREQ("global_b",  vartbl[3].name);
    EXPECT_STREQ("local_1_b", vartbl[4].name);
    EXPECT_STREQ("local_2_b", vartbl[5].name);
}

TEST_F(VartblTest, DeleteAll_GivenLevel0_SetsAllHashmapSlotsUnused) {
    vartbl_add("global_a", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("local_1_a", T_INT, 1, NULL, 0);
    vartbl_add("local_2_a", T_INT, 2, NULL, 0);
    vartbl_add("global_b", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("local_1_b", T_INT, 1, NULL, 0);
    vartbl_add("local_2_b", T_INT, 2, NULL, 0);

    vartbl_delete_all(0);

    for (int ii = 0; ii < VARS_HASHMAP_SIZE; ++ii) {
        EXPECT_TRUE(vartbl_hashmap[ii] == UNUSED_HASH);
    }
}

TEST_F(VartblTest, DeleteAll_GivenNotLevel0_DoesNotSetAllHashmapSlotsUnused) {
    int var_idx;
    var_idx = vartbl_add("global_a", T_INT, GLOBAL_VAR, NULL, 0);
    VarHashValue global_a_hash = vartbl[var_idx].hash;
    var_idx = vartbl_add("local_1_a", T_INT, 1, NULL, 0);
    VarHashValue local_1_a_hash = vartbl[var_idx].hash;
    var_idx = vartbl_add("local_2_a", T_INT, 2, NULL, 0);
    VarHashValue local_2_a_hash = vartbl[var_idx].hash;
    var_idx = vartbl_add("global_b", T_INT, GLOBAL_VAR, NULL, 0);
    VarHashValue global_b_hash = vartbl[var_idx].hash;
    var_idx = vartbl_add("local_1_b", T_INT, 1, NULL, 0);
    VarHashValue local_1_b_hash = vartbl[var_idx].hash;
    var_idx = vartbl_add("local_2_b", T_INT, 2, NULL, 0);
    VarHashValue local_2_b_hash = vartbl[var_idx].hash;

    vartbl_delete_all(1);

    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[local_1_a_hash]);
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[local_1_b_hash]);

    vartbl_delete_all(2);

    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[local_2_a_hash]);
    EXPECT_EQ(DELETED_HASH, vartbl_hashmap[local_2_b_hash]);
}

TEST_F(VartblTest, Find) {
    int var_idx;
    int global_idx;

    vartbl_add("global_1", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("local_1", T_INT, 1, NULL, 0);
    vartbl_add("local_2", T_INT, 2, NULL, 0);

    // Find "global_1".
    EXPECT_EQ(
            kOk,
            vartbl_find("global_1", GLOBAL_VAR, &var_idx, &global_idx));
    EXPECT_EQ(0, var_idx);
    EXPECT_EQ(0, global_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("global_1", 1, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(0, global_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("global_1", 2, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(0, global_idx);

    // Find "local_1".
    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("local_1", GLOBAL_VAR, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(-1, global_idx);

    EXPECT_EQ(
            kOk,
            vartbl_find("local_1", 1, &var_idx, &global_idx));
    EXPECT_EQ(1, var_idx);
    EXPECT_EQ(-1, global_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("local_1", 2, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(-1, global_idx);

    // Find "local_2".
    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("local_2", GLOBAL_VAR, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(-1, global_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("local_2", 1, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(-1, global_idx);

    EXPECT_EQ(
            kOk,
            vartbl_find("local_2", 2, &var_idx, &global_idx));
    EXPECT_EQ(2, var_idx);
    EXPECT_EQ(-1, global_idx);
}

TEST_F(VartblTest, Find_GivenMultipleVariablesWithSameName) {
    int var_idx;
    int global_idx;

    vartbl_add("foo", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("foo", T_INT, 1, NULL, 0);
    vartbl_add("foo", T_INT, 2, NULL, 0);

    EXPECT_EQ(
            kOk,
            vartbl_find("foo", GLOBAL_VAR, &var_idx, &global_idx));
    EXPECT_EQ(0, var_idx);
    EXPECT_EQ(0, global_idx);

    EXPECT_EQ(
            kOk,
            vartbl_find("foo", 1, &var_idx, &global_idx));
    EXPECT_EQ(1, var_idx);
    EXPECT_EQ(0, global_idx); // But may be -1 depending on traversal order.

    EXPECT_EQ(
            kOk,
            vartbl_find("foo", 2, &var_idx, &global_idx));
    EXPECT_EQ(2, var_idx);
    EXPECT_EQ(0, global_idx); // But may be -1 depending on traversal order.

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("foo", 3, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(0, global_idx);
}

TEST_F(VartblTest, Find_GivenNameTooLong) {
    int var_idx;
    int global_idx;

    vartbl_add("foo", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add(
            "_32_chars_long_67890123456789012", T_INT, GLOBAL_VAR, NULL, 0);

    // Only the first 32-chars are checked.
    EXPECT_EQ(
            kOk,
            vartbl_find(
                    "_32_chars_long_67890123456789012xxxx",
                    GLOBAL_VAR,
                    &var_idx,
                    &global_idx));
    EXPECT_EQ(1, var_idx);
    EXPECT_EQ(1, global_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find(
                    "_32_chars_long_67890123456789012xxxx",
                    1,
                    &var_idx,
                    &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(1, global_idx);
}

TEST_F(VartblTest, Find_GivenEmptyName) {
    int var_idx;
    int global_idx;

    vartbl_add("foo", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("bar", T_INT, GLOBAL_VAR, NULL, 0);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("", GLOBAL_VAR, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(-1, global_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("", 1, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(-1, global_idx);

    // Possible in tests, but does not happen in real operation.
    vartbl_add("", T_INT, GLOBAL_VAR, NULL, 0);

    EXPECT_EQ(
            kOk,
            vartbl_find("", GLOBAL_VAR, &var_idx, &global_idx));
    EXPECT_EQ(2, var_idx);
    EXPECT_EQ(2, global_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("", 1, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(2, global_idx);
}

TEST_F(VartblTest, Find_GivenVariableNotPresent) {
    int var_idx;
    int global_idx;

    vartbl_add("foo", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("bar", T_INT, GLOBAL_VAR, NULL, 0);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("wombat", GLOBAL_VAR, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(-1, global_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("wombat", 1, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(-1, global_idx);
}

TEST_F(VartblTest, Find_GivenDifferentCaseName) {
    int var_idx;
    int global_idx;

    // Note that whilst we can test this, in real operation variable
    // names are always capitalised before calling vartbl_add() or
    // vartbl_find().

    vartbl_add("foo", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("bar", T_INT, GLOBAL_VAR, NULL, 0);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("FOO", GLOBAL_VAR, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(-1, global_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("BAR", GLOBAL_VAR, &var_idx, &global_idx));
    EXPECT_EQ(-1, var_idx);
    EXPECT_EQ(-1, global_idx);
}

TEST_F(VartblTest, Find_GivenNullGlobalIdxParameter) {
    int var_idx;

    vartbl_add("global_1", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_add("local_1", T_INT, 1, NULL, 0);

    // Find "global_1".
    EXPECT_EQ(
            kOk,
            vartbl_find("global_1", GLOBAL_VAR, &var_idx, NULL));
    EXPECT_EQ(0, var_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("global_1", 1, &var_idx, NULL));
    EXPECT_EQ(-1, var_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("global_1", 2, &var_idx, NULL));
    EXPECT_EQ(-1, var_idx);

    // Find "local_1".
    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("local_1", GLOBAL_VAR, &var_idx, NULL));
    EXPECT_EQ(-1, var_idx);

    EXPECT_EQ(
            kOk,
            vartbl_find("local_1", 1, &var_idx, NULL));
    EXPECT_EQ(1, var_idx);

    EXPECT_EQ(
            kVariableNotFound,
            vartbl_find("local_1", 2, &var_idx, NULL));
    EXPECT_EQ(-1, var_idx);
}

TEST_F(VartblTest, Find_GivenFirstChoiceHashmapSlotOccupiedByDifferentVariable) {
    int var_idx;
    int global_idx;

    var_idx = vartbl_add("global_0", T_INT, GLOBAL_VAR, NULL, 0);
    vartbl_hashmap[GLOBAL_0_HASH] = -1;
    vartbl_hashmap[LOCAL_1_HASH] = var_idx;
    var_idx = vartbl_add("local_1", T_INT, 1, NULL, 0);
    EXPECT_EQ(LOCAL_1_HASH + 1, vartbl[var_idx].hash);

    EXPECT_EQ(
            kOk,
            vartbl_find("local_1", 1, &var_idx, &global_idx));
    EXPECT_EQ(1, var_idx);
    EXPECT_EQ(-1, global_idx);
}

TEST_F(VartblTest, Find_GivenHashmapWrapAround) {
    int var_idx;
    int global_idx;
    char buf[MAXVARLEN + 1];

    // Fill hashmap from LOCAL_2_HASH (900) to the end.
    for (int ii = LOCAL_2_HASH; ii < VARS_HASHMAP_SIZE; ++ii) {
        sprintf(buf, "var_%d", ii);
        EXPECT_NE(-1, vartbl_add(buf, T_INT, GLOBAL_VAR, NULL, 0));
    }
    var_idx = 0;
    memset(vartbl_hashmap, 0xFF, sizeof(vartbl_hashmap));
    for (int ii = LOCAL_2_HASH; ii < VARS_HASHMAP_SIZE; ++ii) {
        vartbl[var_idx].hash = ii;
        vartbl_hashmap[ii] = var_idx++;
    }

    // Add level 2 LOCAL variable "local_2".
    int expected_var_idx = vartbl_add("local_2", T_INT, 2, NULL, 0);
    EXPECT_EQ(VARS_HASHMAP_SIZE - LOCAL_2_HASH, expected_var_idx);
    EXPECT_EQ(0, vartbl[expected_var_idx].hash);

    // Can we find it ?
    EXPECT_EQ(
            kOk,
            vartbl_find("local_2", 2, &var_idx, &global_idx));
    EXPECT_EQ(expected_var_idx, var_idx);
    EXPECT_EQ(-1, global_idx);
}

TEST_F(VartblTest, Find_GivenDeletedFirstItemInHashChain) {
    int var_idx;
    int global_idx;
    int foo_0 = vartbl_add("foo", T_INT, GLOBAL_VAR, NULL, 0);
    int foo_1 = vartbl_add("foo", T_INT, 1, NULL, 0);
    vartbl_delete(foo_0);

    EXPECT_EQ(kOk, vartbl_find("foo", 1, &var_idx, &global_idx));
    EXPECT_EQ(foo_1, var_idx);
    EXPECT_EQ(-1, global_idx);
}

TEST_F(VartblTest, Find_GivenDeletedIntermediateItemInHashChain) {
    int var_idx;
    int global_idx;
    int foo_0 = vartbl_add("foo", T_INT, GLOBAL_VAR, NULL, 0);
    int foo_1 = vartbl_add("foo", T_INT, 1, NULL, 0);
    int foo_2 = vartbl_add("foo", T_INT, 2, NULL, 0);
    vartbl_delete(foo_1);

    EXPECT_EQ(kOk, vartbl_find("foo", 2, &var_idx, &global_idx));
    EXPECT_EQ(foo_2, var_idx);
    EXPECT_EQ(foo_0, global_idx);
}
