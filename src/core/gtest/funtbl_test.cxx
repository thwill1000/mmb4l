/*
 * Copyright (c) 2022-2023 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../funtbl.h"
#include "../../common/memory.h"

const struct s_funtbl EMPTY_FUN = {};

// Defined in "common/memory.c"
char ProgMemory[PROG_FLASH_SIZE];

} // extern "C"

#define MAX_LENGTH_NAME     "_32_character_name_9012345678901"
#define MAX_LENGTH_NAME_UC  "_32_CHARACTER_NAME_9012345678901"
#define ADDRESS_1           ProgMemory
#define ADDRESS_2           (ProgMemory + 20)
#define ADDRESS_3           (ProgMemory + 40)
#define ADDRESS_4           (ProgMemory + 60)

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
        funtbl[500].type = kSub;
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
    MmResult result = funtbl_add(MAX_LENGTH_NAME, kSub, ADDRESS_1, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);

    // Name will be stored without '\0' termination.
    // 33rd element will be the type byte.
    EXPECT_EQ(0, memcmp(MAX_LENGTH_NAME "\x02", funtbl[fun_idx].name, 33));

    EXPECT_EQ(ADDRESS_1, funtbl[fun_idx].addr);
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
    MmResult result = funtbl_add(silly_name, kSub, ADDRESS_1, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);

    // Truncated name will be stored without '\0' termination.
    // 33rd element will be the type byte.
    EXPECT_EQ(0, memcmp(MAX_LENGTH_NAME "\x02", funtbl[fun_idx].name, 33));

    EXPECT_EQ(ADDRESS_1, funtbl[fun_idx].addr);
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

TEST_F(FuntblTest, Add_ReturnsInternalFault_GivenInvalidAddress) {
    int fun_idx;

    // 0x0 is invalid.
    MmResult result = funtbl_add("foo", kSub, 0x0, &fun_idx);
    EXPECT_EQ(kInternalFault, result);
    EXPECT_EQ(-1, fun_idx);

    // < ProgMemory is invalid.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    result = funtbl_add("foo", kSub, ProgMemory - 1, &fun_idx);
    EXPECT_EQ(kInternalFault, result);
    EXPECT_EQ(-1, fun_idx);
#pragma GCC diagnostic pop

    // >= ProgMemory + PROG_FLASH_SIZE is invalid.
    result = funtbl_add("foo", kSub, ProgMemory + PROG_FLASH_SIZE, &fun_idx);
    EXPECT_EQ(kInternalFault, result);
    EXPECT_EQ(-1, fun_idx);

    // ProgMemory is valid; if unexpected.
    result = funtbl_add("foo", kSub, ProgMemory, &fun_idx);
    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);

    // ProgMemory + PROG_FLASH_SIZE - 1 is valid; if unexpected.
    result = funtbl_add("bar", kSub, ProgMemory + PROG_FLASH_SIZE - 1, &fun_idx);
    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(FuntblTest, Add_ReturnsDuplicateFunction_GivenTwoSubsWithSameName) {
    int fun_idx;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);

    MmResult result = funtbl_add("foo", kSub, ADDRESS_2, &fun_idx);

    EXPECT_EQ(kDuplicateFunction, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Add_ReturnsDuplicateFunction_GivenTwoFunctionsWithSameName) {
    int fun_idx;
    (void) funtbl_add("foo", kFunction, ADDRESS_1, &fun_idx);

    MmResult result = funtbl_add("foo", kFunction, ADDRESS_2, &fun_idx);

    EXPECT_EQ(kDuplicateFunction, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Add_ReturnsDuplicateFunction_GivenTwoLabelsWithSameName) {
    int fun_idx;
    (void) funtbl_add("foo", kLabel, ADDRESS_1, &fun_idx);

    MmResult result = funtbl_add("foo", kLabel, ADDRESS_2, &fun_idx);

    EXPECT_EQ(kDuplicateFunction, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Add_ReturnsDuplicateFunction_GivenSubAndFunctionWithSameName) {
    int fun_idx;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);
    (void) funtbl_add("bar", kFunction, ADDRESS_2, &fun_idx);

    // Add function with same name as sub.
    MmResult result = funtbl_add("foo", kFunction, ADDRESS_3, &fun_idx);

    EXPECT_EQ(kDuplicateFunction, result);
    EXPECT_EQ(-1, fun_idx);

    // Add sub with same name as function.
    result = funtbl_add("bar", kSub, ADDRESS_4, &fun_idx);

    EXPECT_EQ(kDuplicateFunction, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Add_Succeeds_GivenSubAndLabelWithSameName) {
    int fun_idx;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);
    (void) funtbl_add("bar", kLabel, ADDRESS_2, &fun_idx);

    // Add label with same name as sub.
    MmResult result = funtbl_add("foo", kLabel, ADDRESS_3, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(2, fun_idx);

    // Add sub with same name as label.
    result = funtbl_add("bar", kSub, ADDRESS_4, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(3, fun_idx);
}

TEST_F(FuntblTest, Add_Succeeds_GivenFunctionAndLabelWithSameName) {
    int fun_idx;
    (void) funtbl_add("foo", kFunction, ADDRESS_1, &fun_idx);
    (void) funtbl_add("bar", kLabel, ADDRESS_2, &fun_idx);

    // Add label with same name as function.
    MmResult result = funtbl_add("foo", kLabel, ADDRESS_3, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(2, fun_idx);

    // Add function with same name as label.
    result = funtbl_add("bar", kFunction, ADDRESS_4, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(3, fun_idx);
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

TEST_F(FuntblTest, Find_GivenSubFound_InSecondChoiceSlot) {
    int fun_idx;
    FunHashValue foo_hash = hash_cstring("foo", MAXVARLEN) % FUN_HASHMAP_SIZE;
    strcpy(funtbl[500].name, "bar");
    funtbl[500].type = kSub;
    funtbl_hashmap[foo_hash] = 500;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);
    EXPECT_EQ(0, funtbl_hashmap[foo_hash + 1]);
    EXPECT_EQ(foo_hash + 1, funtbl[0].hash);

    MmResult result = funtbl_find("foo", kSub, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_GivenSubFound_AfterHashmapWraparound) {
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

TEST_F(FuntblTest, Find_GiveSubNotFound_AndHashmapNotFull) {
    int fun_idx;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);
    (void) funtbl_add("bar", kSub, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("wombat", kSub, &fun_idx);

    EXPECT_EQ(kFunctionNotFound, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Find_GivenSubNotFound_AndHashmapFull) {
    GivenHashmapFull();

    int fun_idx;
    MmResult result = funtbl_find("wombat", kSub, &fun_idx);

    EXPECT_EQ(kFunctionNotFound, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Find_GivenLookingForFunctionButFindSub) {
    int fun_idx;
    (void) funtbl_add("fnfoo", kFunction, ADDRESS_1, &fun_idx);
    (void) funtbl_add("subfoo", kSub, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("fnfoo", kSub, &fun_idx);

    EXPECT_EQ(kFunctionTypeMismatch, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_GivenLookingForSubButFindFunction) {
    int fun_idx;
    (void) funtbl_add("fnfoo", kFunction, ADDRESS_1, &fun_idx);
    (void) funtbl_add("subfoo", kSub, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("subfoo", kFunction, &fun_idx);

    EXPECT_EQ(kFunctionTypeMismatch, result);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(FuntblTest, Find_GivenLookingForFunctionOrSub) {
    int fun_idx;
    (void) funtbl_add("fnfoo", kFunction, ADDRESS_1, &fun_idx);
    (void) funtbl_add("subfoo", kSub, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("fnfoo", kFunction | kSub, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);

    result = funtbl_find("subfoo", kFunction | kSub, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(FuntblTest, Find_ReturnsTypeMismatch_GivenLookingForFunction_ButFindsSub) {
    int fun_idx;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);

    MmResult result = funtbl_find("foo", kFunction, &fun_idx);

    EXPECT_EQ(kFunctionTypeMismatch, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_Succeeds_GivenLookingForFunction_ButFirstFindsLabel) {
    int fun_idx;
    (void) funtbl_add("foo", kLabel, ADDRESS_1, &fun_idx);
    (void) funtbl_add("foo", kFunction, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("foo", kFunction, &fun_idx);

    // Ignores the label, finds the function.
    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(FuntblTest, Find_ReturnsTypeMismatch_GivenLookingForSub_ButFindsFunction) {
    int fun_idx;
    (void) funtbl_add("foo", kFunction, ADDRESS_1, &fun_idx);

    MmResult result = funtbl_find("foo", kSub, &fun_idx);

    EXPECT_EQ(kFunctionTypeMismatch, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_Succeeds_GivenLookingForSub_ButFirstFindsLabel) {
    int fun_idx;
    (void) funtbl_add("foo", kLabel, ADDRESS_1, &fun_idx);
    (void) funtbl_add("foo", kSub, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("foo", kSub, &fun_idx);

    // Ignores the label, finds the sub.
    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(FuntblTest, Find_Succeeds_GivenLookingForLabel_ButFirstFindsFunction) {
    int fun_idx;
    (void) funtbl_add("foo", kFunction, ADDRESS_1, &fun_idx);
    (void) funtbl_add("foo", kLabel, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("foo", kLabel, &fun_idx);

    // Ignores the function, finds the label.
    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(FuntblTest, Find_Succeeds_GivenLookingForLabel_ButFirstFindsSub) {
    int fun_idx;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);
    (void) funtbl_add("foo", kLabel, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("foo", kLabel, &fun_idx);

    // Ignores the sub, finds the label.
    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(FuntblTest, Find_ReturnsTypeMismatch_GivenLookingForFunction_ButOnlyFindsLabel) {
    int fun_idx;
    (void) funtbl_add("foo", kLabel, ADDRESS_1, &fun_idx);

    MmResult result = funtbl_find("foo", kFunction, &fun_idx);

    EXPECT_EQ(kFunctionTypeMismatch, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_ReturnsTypeMismatch_GivenLookingForSub_ButOnlyFindsLabel) {
    int fun_idx;
    (void) funtbl_add("foo", kLabel, ADDRESS_1, &fun_idx);

    MmResult result = funtbl_find("foo", kSub, &fun_idx);

    EXPECT_EQ(kFunctionTypeMismatch, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_ReturnsTypeMismatch_GivenLookingForSubOrFunction_ButOnlyFindsLabel) {
    int fun_idx;
    (void) funtbl_add("foo", kLabel, ADDRESS_1, &fun_idx);

    MmResult result = funtbl_find("foo", kFunction | kSub, &fun_idx);

    EXPECT_EQ(kFunctionTypeMismatch, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_ReturnsTypeMismatch_GivenLookingForLabel_ButOnlyFindsFunction) {
    int fun_idx;
    (void) funtbl_add("foo", kFunction, ADDRESS_1, &fun_idx);

    MmResult result = funtbl_find("foo", kLabel, &fun_idx);

    EXPECT_EQ(kFunctionTypeMismatch, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_ReturnsTypeMismatch_GivenLookingForLabel_ButOnlyFindsSub) {
    int fun_idx;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);

    MmResult result = funtbl_find("foo", kLabel, &fun_idx);

    EXPECT_EQ(kFunctionTypeMismatch, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_ReturnsMismatchedSubInPreferenceToLabel) {
    int fun_idx;
    (void) funtbl_add("foo", kSub, ADDRESS_1, &fun_idx);
    (void) funtbl_add("foo", kLabel, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("foo", kFunction, &fun_idx);

    EXPECT_EQ(kFunctionTypeMismatch, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_ReturnMismatchedFunctionInPreferenceToLabel) {
    int fun_idx;
    (void) funtbl_add("foo", kFunction, ADDRESS_1, &fun_idx);
    (void) funtbl_add("foo", kLabel, ADDRESS_2, &fun_idx);

    MmResult result = funtbl_find("foo", kSub, &fun_idx);

    EXPECT_EQ(kFunctionTypeMismatch, result);
    EXPECT_EQ(0, fun_idx);
}
