#include <gtest/gtest.h>

extern "C" {

#include "../utility.h"

}

TEST(UtilityTest, CstringCat) {
    char s[10];

    strcpy(s, "XXXXXXXXX"); // To help detect overruns.
    strcpy(s, "123");
    EXPECT_EQ(0, cstring_cat(s, "456", sizeof(s - 2)));
    EXPECT_STREQ("123456", s);
    EXPECT_EQ('X', s[7]);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "123");
    EXPECT_EQ(0, cstring_cat(s, "4567", sizeof(s - 2)));
    EXPECT_STREQ("1234567", s);
    EXPECT_EQ('\0', s[7]);
    EXPECT_EQ('X', s[8]);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "123");
    EXPECT_EQ(-1, cstring_cat(s, "45678", sizeof(s - 2)));
    EXPECT_STREQ("1234567", s);
    EXPECT_EQ('X', s[8]);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "123");
    EXPECT_EQ(-1, cstring_cat(s, "456789", sizeof(s - 2)));
    EXPECT_STREQ("1234567", s);
    EXPECT_EQ('X', s[8]);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "123");
    EXPECT_EQ(0, cstring_cat(s, "", sizeof(s - 2)));
    EXPECT_STREQ("123", s);
    EXPECT_EQ('X', s[8]);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "");
    EXPECT_EQ(0, cstring_cat(s, "123", sizeof(s - 2)));
    EXPECT_STREQ("123", s);
    EXPECT_EQ('X', s[8]);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "");
    EXPECT_EQ(0, cstring_cat(s, "", sizeof(s - 2)));
    EXPECT_STREQ("", s);
    EXPECT_EQ('X', s[8]);
}

TEST(UtilityTest, StrReplace) {
    char s[256] = "cat dog fish cat";

    str_replace(s, "cat", "elephant");

    EXPECT_STREQ("elephant dog fish elephant", s);
}
