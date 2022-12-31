/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../funtbl.h"

const struct s_funtbl EMPTY_FUN = {};

// Declared in "error.c"
void error_throw(MmResult error) { }
void error_throw_ex(MmResult error, const char *msg, ...) { }
void error_throw_legacy(const char *msg, ...) { }

// Declared in "MMBasic.c"
int LocalIndex = 0;
int getint(char *p, int min, int max) { return 0; }
long long int getinteger(char *p) { return 0; }

} // extern "C"

#define MAX_LENGTH_NAME     "_32_character_name_9012345678901"
#define MAX_LENGTH_NAME_UC  "_32_CHARACTER_NAME_9012345678901"

#define EXPECT_FUNTBL_ENTRY(idx, expected_name, expected_code) \
    { \
    EXPECT_STREQ(expected_name, funtbl[idx].name); \
    EXPECT_EQ(expected_code, funtbl[idx].code); \
    FunHashValue expected_hash = hash_cstring(expected_name, MAXVARLEN) % FUN_HASHMAP_SIZE; \
    EXPECT_EQ(expected_hash, funtbl[idx].hash); \
    EXPECT_EQ(idx, funtbl[idx].index); \
    EXPECT_EQ(idx, funtbl_hashmap[expected_hash]); \
    }

class FuntblTest : public ::testing::Test {

protected:

    void SetUp() override {
        funtbl_clear();
        memset(subfun, 0, sizeof(subfun));
        m_program[0] = '\0';
    }

    void TearDown() override {
    }

    void GivenHashmapFull() {
        strcpy(funtbl[500].name, "filler");
        for (int ii = 0; ii < FUN_HASHMAP_SIZE; ++ii) {
            funtbl_hashmap[ii] = 500; // Dummy value.
        }
    }

    char m_program[256];

};

TEST_F(FuntblTest, Add) {
    int fun_idx;
    MmResult result = funtbl_add("foo", m_program, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);
    EXPECT_FUNTBL_ENTRY(0, "foo", m_program);
    EXPECT_EQ(1, funtbl_count);

    result = funtbl_add("bar", m_program + 1, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, fun_idx);
    EXPECT_FUNTBL_ENTRY(1, "bar", m_program + 1);
    EXPECT_EQ(2, funtbl_count);
}

TEST_F(FuntblTest, Add_GivenTooManyFunctions) {
    int fun_idx;
    char buf[MAXVARLEN + 1];
    for (int ii = 0; ii < MAXSUBFUN; ++ii) {
        sprintf(buf, "fun_%d", ii);
        EXPECT_EQ(kOk, funtbl_add(buf, m_program, &fun_idx));
        EXPECT_EQ(ii, fun_idx);
    }
    EXPECT_EQ(512, funtbl_count);

    MmResult result = funtbl_add("bar", m_program, &fun_idx);

    EXPECT_EQ(kTooManyFunctions, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Add_GivenMaxLengthName) {
    int fun_idx;
    MmResult result = funtbl_add(MAX_LENGTH_NAME, (const char *) -1, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);

    // Name will be stored without '\0' termination.
    // 33rd element will be the first byte of the code pointer.
    EXPECT_EQ(0, memcmp(MAX_LENGTH_NAME "\xFF", funtbl[fun_idx].name, 33));

    EXPECT_EQ((const char *) -1, funtbl[fun_idx].code);
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
    MmResult result = funtbl_add(silly_name, (const char *) -1, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);

    // Truncated name will be stored without '\0' termination.
    // 33rd element will be the first byte of the code pointer.
    EXPECT_EQ(0, memcmp(MAX_LENGTH_NAME "\xFF", funtbl[fun_idx].name, 33));

    EXPECT_EQ((const char *) -1, funtbl[fun_idx].code);
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
    MmResult result = funtbl_add("foo", m_program, &fun_idx);

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
    MmResult result = funtbl_add("foo", m_program, &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);
    EXPECT_EQ(1, funtbl[fun_idx].hash);
    EXPECT_EQ(fun_idx, funtbl_hashmap[1]);
}

TEST_F(FuntblTest, Add_GivenHashmapFull) {
    GivenHashmapFull();

    int fun_idx;
    MmResult result = funtbl_add("foo", m_program, &fun_idx);

    EXPECT_EQ(kHashmapFull, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Prepare) {
    sprintf(m_program,
            "# foo\n"
            "#\n"
            "# bar\n"
            "#\n");
    subfun[0] = m_program;
    subfun[1] = m_program + 8;
    subfun[2] = NULL;

    MmResult result = funtbl_prepare(true);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(2, funtbl_size());
    EXPECT_FUNTBL_ENTRY(0, "FOO", m_program);
    EXPECT_FUNTBL_ENTRY(1, "BAR", m_program + 8);
}

TEST_F(FuntblTest, Prepare_GivenNameTooLong) {
    sprintf(m_program,
            "# name_32_characters_xxxxxxxxxxxxx\n"
            "#\n"
            "# name_33_characters_zzzzzzzzzzzzzz\n"
            "#\n");
    subfun[0] = m_program;
    subfun[1] = m_program + 37;
    subfun[2] = NULL;

    MmResult result = funtbl_prepare(true);

    EXPECT_EQ(kNameTooLong, result);
    EXPECT_EQ(1, funtbl_size());
    EXPECT_EQ(0, strncmp("NAME_32_CHARACTERS_XXXXXXXXXXXXX", funtbl[0].name, MAXVARLEN));
    EXPECT_EQ(m_program, funtbl[0].code);
    FunHashValue expected_hash =
            hash_cstring("NAME_32_CHARACTERS_XXXXXXXXXXXXX", MAXVARLEN) % FUN_HASHMAP_SIZE;
    EXPECT_EQ(expected_hash, funtbl[0].hash);
    EXPECT_EQ(0, funtbl[0].index);
}

TEST_F(FuntblTest, Prepare_GivenDuplicateName) {
    sprintf(m_program,
            "# foo\n"
            "#\n"
            "# fOo\n"
            "#\n");
    subfun[0] = m_program;
    subfun[1] = m_program + 8;
    subfun[2] = NULL;

    MmResult result = funtbl_prepare(true);

    EXPECT_EQ(kDuplicateFunction, result);
    EXPECT_EQ(1, funtbl_size());
    EXPECT_FUNTBL_ENTRY(0, "FOO", m_program);
}

TEST_F(FuntblTest, Find_GivenFunctionFound_InFirstChoiceSlot) {
    int fun_idx;
    (void) funtbl_add("foo", m_program, &fun_idx);
    (void) funtbl_add("bar", m_program, &fun_idx);

    MmResult result = funtbl_find("foo", &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);

    result = funtbl_find("bar", &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(1, fun_idx);
}

TEST_F(FuntblTest, Find_GivenFunctionFound_InSecondChoiceSlot) {
    int fun_idx;
    FunHashValue foo_hash = hash_cstring("foo", MAXVARLEN) % FUN_HASHMAP_SIZE;
    strcpy(funtbl[500].name, "bar");
    funtbl_hashmap[foo_hash] = 500;
    (void) funtbl_add("foo", m_program, &fun_idx);
    EXPECT_EQ(0, funtbl_hashmap[foo_hash + 1]);
    EXPECT_EQ(foo_hash + 1, funtbl[0].hash);

    MmResult result = funtbl_find("foo", &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_GivenFunctionFound_AfterHashmapWraparound) {
    GivenHashmapFull();
    funtbl_hashmap[1] = -1; // Free hashmap element 1.
    int fun_idx;
    (void) funtbl_add("foo", m_program, &fun_idx);
    EXPECT_EQ(0, funtbl_hashmap[1]);
    EXPECT_EQ(1, funtbl[0].hash);

    MmResult result = funtbl_find("foo", &fun_idx);

    EXPECT_EQ(kOk, result);
    EXPECT_EQ(0, fun_idx);
}

TEST_F(FuntblTest, Find_GivenFunctionNotFound_AndHashmapNotFull) {
    int fun_idx;
    (void)funtbl_add("foo", m_program, &fun_idx);
    (void)funtbl_add("bar", m_program, &fun_idx);

    MmResult result = funtbl_find("wombat", &fun_idx);

    EXPECT_EQ(kFunctionNotFound, result);
    EXPECT_EQ(-1, fun_idx);
}

TEST_F(FuntblTest, Find_GivenFunctionNotFound_AndHashmapFull) {
    GivenHashmapFull();

    int fun_idx;
    MmResult result = funtbl_find("wombat", &fun_idx);

    EXPECT_EQ(kFunctionNotFound, result);
    EXPECT_EQ(-1, fun_idx);
}
