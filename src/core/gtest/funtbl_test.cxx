/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../FunTable.h"
#include "../funtbl.h"

char error_msg[256];

void error_throw_legacy(const char *msg, ...) {
    strcpy(error_msg, msg);
}

// Declared in "MMBasic.c"
struct s_funtbl funtbl[MAXSUBFUN];
const char *subfun[MAXSUBFUN];

} // extern "C"

#define EXAMPLE_MAX_LENGTH_NAME     "_32_character_name_9012345678901"
#define EXAMPLE_MAX_LENGTH_NAME_UC  "_32_CHARACTER_NAME_9012345678901"

class FuntblTest : public ::testing::Test {

protected:

    void SetUp() override {
        error_msg[0] = '\0';
        memset(funtbl, 0, sizeof(funtbl));
        memset(subfun, 0, sizeof(subfun));
        m_program[0] = '\0';
    }

    void TearDown() override {
    }

    char m_program[256];

};

TEST_F(FuntblTest, Hash) {
    char name[MAXVARLEN + 1];
    HashValue hash;

    sprintf(m_program, "foo");
    int actual = funtbl_hash(m_program, name, &hash);

    EXPECT_EQ(0, actual);
    EXPECT_STREQ("FOO", name);
    EXPECT_EQ(503, hash);

    sprintf(m_program, "bar");
    actual = funtbl_hash(m_program, name, &hash);

    EXPECT_EQ(0, actual);
    EXPECT_STREQ("BAR", name);
    EXPECT_EQ(122, hash);
}

TEST_F(FuntblTest, Hash_GivenMaximumLengthName) {
    char name[MAXVARLEN + 1];
    HashValue hash;

    sprintf(m_program, EXAMPLE_MAX_LENGTH_NAME);
    int actual = funtbl_hash(m_program, name, &hash);

    EXPECT_EQ(0, actual);
    EXPECT_STREQ(EXAMPLE_MAX_LENGTH_NAME_UC, name);
    EXPECT_EQ(479, hash);
}

TEST_F(FuntblTest, Hash_GivenNameTooLong) {
    char name[MAXVARLEN + 1];
    HashValue hash;

    sprintf(m_program, "_33_character_name_90123456789012");
    int actual = funtbl_hash(m_program, name, &hash);

    EXPECT_EQ(-1, actual);
    EXPECT_STREQ("_33_CHARACTER_NAME_9012345678901", name);
    EXPECT_EQ(24, hash);
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

    funtbl_prepare(true);

    EXPECT_EQ(2, funtbl_size());
    EXPECT_STREQ("", funtbl[0].name);
    EXPECT_EQ(0, funtbl[0].index);
    EXPECT_STREQ("BAR", funtbl[122].name);
    EXPECT_EQ(1, funtbl[122].index);
    EXPECT_STREQ("FOO", funtbl[503].name);
    EXPECT_EQ(0, funtbl[503].index);
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

    funtbl_prepare(true);

    EXPECT_EQ(2, funtbl_size());
    EXPECT_STREQ("", funtbl[0].name);
    EXPECT_EQ(0, funtbl[0].index);
    EXPECT_STREQ("NAME_32_CHARACTERS_XXXXXXXXXXXXX", funtbl[308].name);
    EXPECT_EQ(0, funtbl[308].index);
    EXPECT_STREQ("NAME_33_CHARACTERS_ZZZZZZZZZZZZZ", funtbl[431].name);
    EXPECT_EQ(1, funtbl[431].index);
    EXPECT_STREQ("SUB/FUNCTION name too long", error_msg);
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

    funtbl_prepare(true);

    EXPECT_EQ(1, funtbl_size());
    EXPECT_STREQ("", funtbl[0].name);
    EXPECT_EQ(0, funtbl[0].index);
    EXPECT_STREQ("FOO", funtbl[503].name);
    EXPECT_EQ(0, funtbl[503].index);
    EXPECT_STREQ("Duplicate SUB/FUNCTION declaration", error_msg);
}

TEST_F(FuntblTest, Find) {
    sprintf(m_program,
            "# foo\n"
            "#\n"
            "# bar\n"
            "#\n");
    subfun[0] = m_program;
    subfun[1] = m_program + 8;
    subfun[2] = NULL;

    funtbl_prepare(true);

    EXPECT_EQ(0,  funtbl_find("Foo("));
    EXPECT_EQ(1,  funtbl_find("BAr "));
    EXPECT_EQ(-1, funtbl_find("WOMBAT("));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(-1, funtbl_find("name_33_characters_zzzzzzzzzzzzzz"));
    EXPECT_STREQ("SUB/FUNCTION name too long", error_msg);
}
