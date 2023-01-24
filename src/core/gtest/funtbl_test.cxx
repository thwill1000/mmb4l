/*
 * Copyright (c) 2022-2023 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../funtbl.h"

const struct s_funtbl EMPTY_FUN = {};

} // extern "C"

#define MAX_LENGTH_NAME     "_32_character_name_9012345678901"
#define MAX_LENGTH_NAME_UC  "_32_CHARACTER_NAME_9012345678901"
#define ADDRESS_1           (const char*) 0xABCD
#define ADDRESS_2           (const char*) 0x1234

#define EXPECT_FUNTBL_ENTRY(idx, expected_name, expected_type, expected_addr) \
    { \
    EXPECT_STREQ(expected_name, funtbl[idx].name); \
    EXPECT_EQ(expected_type, funtbl[idx].type); \
    FunHashValue expected_hash = hash_cstring(expected_name, MAXVARLEN) % FUN_HASHMAP_SIZE; \
    EXPECT_EQ(expected_hash, funtbl[idx].hash); \
    EXPECT_EQ(expected_addr, funtbl[idx].addr); \
    EXPECT_EQ(idx, funtbl_hashmap[expected_hash]); \
    }

class FuntblTest : public ::testing::Test {

protected:

    void SetUp() override {
        funtbl_clear();
    }

    void TearDown() override {
    }

    void GivenHashmapFull() {
        strcpy(funtbl[500].name, "filler");
        for (int ii = 0; ii < FUN_HASHMAP_SIZE; ++ii) {
            funtbl_hashmap[ii] = 500; // Dummy value.
        }
    }

};

TEST_F(FuntblTest, Add) {
    int fun_idx;
    MmResult result = funtbl_add("foo", kFunction, ADDRESS_1, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);
    EXPECT_FUNTBL_ENTRY(0, "foo", kFunction, ADDRESS_1);
    EXPECT_EQ(1, funtbl_count);

    result = funtbl_add("bar", kSub, ADDRESS_2, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, fun_idx);
    EXPECT_FUNTBL_ENTRY(1, "bar", kSub, ADDRESS_2);
    EXPECT_EQ(2, funtbl_count);
}

TEST_F(FuntblTest, Add_GivenTooManyFunctions) {
    int fun_idx;
    char buf[MAXVARLEN + 1];
    for (int ii = 0; ii < MAXSUBFUN; ++ii) {
        sprintf(buf, "fun_%d", ii);
        EXPECT_EQ(kOk, funtbl_add(buf, kSub, ADDRESS_1, &fun_idx));
        EXPECT_EQ(ii, fun_idx);
    }
    EXPECT_EQ(512, funtbl_count);

    MmResult result = funtbl_add("bar", kSub, ADDRESS_1, &fun_idx);

    EXPECT_EQ(kTooManyFunctions, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Add_GivenMaxLengthName) {
    int fun_idx;
    MmResult result = funtbl_add(MAX_LENGTH_NAME, kSub, (const char *) -1, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);

    // Name will be stored without '\0' termination.
    // 33rd element will be the type byte.
    EXPECT_EQ(0, memcmp(MAX_LENGTH_NAME "\x02", funtbl[fun_idx].name, 33));

    EXPECT_EQ((const char *) -1, funtbl[fun_idx].addr);
    FunHashValue expected_hash = hash_cstring(MAX_LENGTH_NAME, MAXVARLEN) % FUN_HASHMAP_SIZE;
    EXPECT_EQ(expected_hash, funtbl[fun_idx].hash);
    EXPECT_EQ(fun_idx, funtbl_hashmap[expected_hash]);
    EXPECT_EQ(1, funtbl_count);
}

TEST_F(FuntblTest, Add_GivenNameTooLong) {
    char silly_name[1024];
    memset(silly_name, 'a', 1023);
    memcpy(silly_name, MAX_LENGTH_NAME, MAXVARLEN);

    int fun_idx;
    MmResult result = funtbl_add(silly_name, kSub, (const char *) -1, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);

    // Truncated name will be stored without '\0' termination.
    // 33rd element will be the type byte.
    EXPECT_EQ(0, memcmp(MAX_LENGTH_NAME "\x02", funtbl[fun_idx].name, 33));

    EXPECT_EQ((const char *) -1, funtbl[fun_idx].addr);
    FunHashValue expected_hash = hash_cstring(MAX_LENGTH_NAME, MAXVARLEN) % FUN_HASHMAP_SIZE;
    EXPECT_EQ(expected_hash, funtbl[fun_idx].hash);
    EXPECT_EQ(fun_idx, funtbl_hashmap[expected_hash]);
    EXPECT_EQ(1, funtbl_count);

    // Check we did not overrun into the next element.
    EXPECT_EQ(0, memcmp(&EMPTY_FUN, funtbl + 1, sizeof(struct s_funtbl)));
}

TEST_F(FuntblTest, Add_GivenFirstChoiceHashmapSlotOccupied) {
    FunHashValue foo_hash = hash_cstring("foo", MAXVARLEN) % FUN_HASHMAP_SIZE;
    strcpy(funtbl[500].name, "bar");
    funtbl_hashmap[foo_hash] = 500;

    int fun_idx;
    MmResult result = funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);
    // Expect function idx to be stored in next slot.
    EXPECT_EQ(foo_hash + 1, funtbl[fun_idx].hash);
    EXPECT_EQ(fun_idx, funtbl_hashmap[foo_hash + 1]);
}

TEST_F(FuntblTest, Add_GivenHashmapWraparound) {
    GivenHashmapFull();
    funtbl_hashmap[1] = -1; // Free hashmap element 1.

    int fun_idx;
    MmResult result = funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);
    EXPECT_EQ(1, funtbl[fun_idx].hash);
    EXPECT_EQ(fun_idx, funtbl_hashmap[1]);
}

TEST_F(FuntblTest, Add_GivenHashmapFull) {
    GivenHashmapFull();

    int fun_idx;
    MmResult result = funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);

    EXPECT_EQ(kHashmapFull, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Find_GivenFunctionFound_InFirstChoiceSlot) {
    int fun_idx;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);
    (void) funtbl_add("bar", kSub, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("foo", kSub, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);

    result = funtbl_find("bar", kSub, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(FuntblTest, Find_GivenFunctionFound_InSecondChoiceSlot) {
    int fun_idx;
    FunHashValue foo_hash = hash_cstring("foo", MAXVARLEN) % FUN_HASHMAP_SIZE;
    strcpy(funtbl[500].name, "bar");
    funtbl_hashmap[foo_hash] = 500;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);
    EXPECT_EQ(0, funtbl_hashmap[foo_hash + 1]);
    EXPECT_EQ(foo_hash + 1, funtbl[0].hash);

    MmResult result = funtbl_find("foo", kSub, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_GivenFunctionFound_AfterHashmapWraparound) {
    GivenHashmapFull();
    funtbl_hashmap[1] = -1; // Free hashmap element 1.
    int fun_idx;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);
    EXPECT_EQ(0, funtbl_hashmap[1]);
    EXPECT_EQ(1, funtbl[0].hash);

    MmResult result = funtbl_find("foo", kSub, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_GivenFunctionNotFound_AndHashmapNotFull) {
    int fun_idx;
    (void)funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);
    (void)funtbl_add("bar", kSub, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("wombat", kSub, &fun_idx);

    EXPECT_EQ(kFunctionNotFound, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Find_GivenFunctionNotFound_AndHashmapFull) {
    GivenHashmapFull();

    int fun_idx;
    MmResult result = funtbl_find("wombat", kSub, &fun_idx);

    EXPECT_EQ(kFunctionNotFound, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Find_GivenLookingForFunctionButFindSub) {
    int fun_idx;
    (void)funtbl_add("fnfoo", kFunction, ADDRESS_1, &fun_idx);
    (void)funtbl_add("subfoo", kSub, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("fnfoo", kSub, &fun_idx);

    EXPECT_EQ(kTargetTypeMismatch, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_GivenLookingForSubButFindFunction) {
    int fun_idx;
    (void)funtbl_add("fnfoo", kFunction, ADDRESS_1, &fun_idx);
    (void)funtbl_add("subfoo", kSub, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("subfoo", kFunction, &fun_idx);

    EXPECT_EQ(kTargetTypeMismatch, result);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(FuntblTest, Find_GivenLookingForFunctionOrSub) {
    int fun_idx;
    (void)funtbl_add("fnfoo", kFunction, ADDRESS_1, &fun_idx);
    (void)funtbl_add("subfoo", kSub, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("fnfoo", kFunction | kSub, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);

    result = funtbl_find("subfoo", kFunction | kSub, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, fun_idx);
}
