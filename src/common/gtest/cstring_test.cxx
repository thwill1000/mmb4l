/*
 * Copyright (c) 2021-2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../cstring.h"
#include "../utility.h"

}

TEST(CstringTest, Cat) {
    char s[10];

    strcpy(s, "XXXXXXXXX"); // To help detect overruns.
    strcpy(s, "123");
    EXPECT_EQ(0, cstring_cat(s, "456", sizeof(s) - 2));
    EXPECT_STREQ("123456", s);
    EXPECT_STREQ("XX", s + strlen(s) + 1);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "123");
    EXPECT_EQ(0, cstring_cat(s, "4567", sizeof(s) - 2));
    EXPECT_STREQ("1234567", s);
    EXPECT_EQ('\0', s[7]);
    EXPECT_STREQ("X", s + strlen(s) + 1);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "123");
    EXPECT_EQ(-1, cstring_cat(s, "45678", sizeof(s) - 2));
    EXPECT_STREQ("1234567", s);
    EXPECT_STREQ("X", s + strlen(s) + 1);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "123");
    EXPECT_EQ(-1, cstring_cat(s, "456789", sizeof(s) - 2));
    EXPECT_STREQ("1234567", s);
    EXPECT_STREQ("X", s + strlen(s) + 1);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "123");
    EXPECT_EQ(0, cstring_cat(s, "", sizeof(s) - 2));
    EXPECT_STREQ("123", s);
    EXPECT_STREQ("XXXXX", s + strlen(s) + 1);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "");
    EXPECT_EQ(0, cstring_cat(s, "123", sizeof(s) - 2));
    EXPECT_STREQ("123", s);
    EXPECT_STREQ("XXXXX", s + strlen(s) + 1);

    strcpy(s, "XXXXXXXXX");
    strcpy(s, "");
    EXPECT_EQ(0, cstring_cat(s, "", sizeof(s) - 2));
    EXPECT_STREQ("", s);
    EXPECT_STREQ("XXXXXXXX", s + strlen(s) + 1);
}

TEST(CstringTest, Cpy) {
    char dst[10] = { 0 };

    EXPECT_EQ(-1, cstring_cpy(dst, "", 0));
    EXPECT_STREQ("", dst);

    EXPECT_EQ(0, cstring_cpy(dst, "", sizeof(dst)));
    EXPECT_STREQ("", dst);

    EXPECT_EQ(0, cstring_cpy(dst, "foobar", sizeof(dst)));
    EXPECT_STREQ("foobar", dst);

    EXPECT_EQ(0, cstring_cpy(dst, "123456789", sizeof(dst)));
    EXPECT_STREQ("123456789", dst);

    EXPECT_EQ(-1, cstring_cpy(dst, "1234567890", sizeof(dst)));
    EXPECT_STREQ("123456789", dst);

    memset(dst, 0xFF, sizeof(dst));
    EXPECT_EQ(-1, cstring_cpy(dst, "1234567890", 2));
    EXPECT_STREQ("1", dst);
    EXPECT_EQ(0, memcmp("1\0\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF", dst, sizeof(dst)));
}

TEST(CstringTest, Enquote) {
    char s[16];

    strcpy(s, "foo bar");
    EXPECT_EQ(0, cstring_enquote(s, 16));
    EXPECT_STREQ("\"foo bar\"", s);

    strcpy(s, "\"foo bar\"");
    EXPECT_EQ(0, cstring_enquote(s, 16));
    EXPECT_STREQ("\"\"foo bar\"\"", s);

    strcpy(s, "a");
    EXPECT_EQ(0, cstring_enquote(s, 16));
    EXPECT_STREQ("\"a\"", s);

    strcpy(s, "");
    EXPECT_EQ(0, cstring_enquote(s, 16));
    EXPECT_STREQ("\"\"", s);

    // Test when string too long.
    strcpy(s, "0123456789ABCDE");
    EXPECT_EQ(-1, cstring_enquote(s, 16));
    EXPECT_STREQ("0123456789ABCDE", s);

    strcpy(s, "0123456789ABCDE");
    EXPECT_EQ(-1, cstring_enquote(s, 0));
    EXPECT_STREQ("0123456789ABCDE", s);

    EXPECT_EQ(-1, cstring_enquote(NULL, 16));
}

TEST(CstringTest, Quote) {
    char s[16];

    strcpy(s, "foo bar");
    EXPECT_EQ(0, cstring_quote(s, 16));
    EXPECT_STREQ("\"foo bar\"", s);

    strcpy(s, "\"foo bar\"");
    EXPECT_EQ(0, cstring_quote(s, 16));
    EXPECT_STREQ("\"\"foo bar\"\"", s);

    strcpy(s, "a");
    EXPECT_EQ(0, cstring_quote(s, 16));
    EXPECT_STREQ("\"a\"", s);

    strcpy(s, "");
    EXPECT_EQ(0, cstring_quote(s, 16));
    EXPECT_STREQ("\"\"", s);

    // Test when string too long.
    strcpy(s, "0123456789ABCDE");
    EXPECT_EQ(-1, cstring_quote(s, 16));
    EXPECT_STREQ("0123456789ABCDE", s);

    strcpy(s, "0123456789ABCDE");
    EXPECT_EQ(-1, cstring_quote(s, 0));
    EXPECT_STREQ("0123456789ABCDE", s);

    EXPECT_EQ(-1, cstring_quote(NULL, 16));
}

TEST(CstringTest, IsQuoted) {
    EXPECT_TRUE(cstring_isquoted("\"foo bar\""));
    EXPECT_TRUE(cstring_isquoted("\"a\""));
    EXPECT_TRUE(cstring_isquoted("\"\""));

    EXPECT_FALSE(cstring_isquoted("foo bar"));
    EXPECT_FALSE(cstring_isquoted("\"foo bar"));
    EXPECT_FALSE(cstring_isquoted("foo bar\""));
    EXPECT_FALSE(cstring_isquoted(" \"foo bar\" "));
    EXPECT_FALSE(cstring_isquoted(""));
    EXPECT_FALSE(cstring_isquoted("\""));
}

TEST(CstringTest, Replace) {
    char s[256] = "cat dog fish cat";

    cstring_replace(s, "cat", "elephant");

    EXPECT_STREQ("elephant dog fish elephant", s);
}

TEST(CstringTest, ToLower) {
    char s[256];

    strcpy(s, "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    EXPECT_EQ(s, cstring_tolower(s));
    EXPECT_STREQ("1234567890abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", s);

    strcpy(s, "A");
    EXPECT_EQ(s, cstring_tolower(s));
    EXPECT_STREQ("a", s);

    strcpy(s, "");
    EXPECT_EQ(s, cstring_tolower(s));
    EXPECT_STREQ("", s);
}

TEST(CstringTest, ToUpper) {
    char s[256];

    strcpy(s, "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    EXPECT_EQ(s, cstring_toupper(s));
    EXPECT_STREQ("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", s);

    strcpy(s, "a");
    EXPECT_EQ(s, cstring_toupper(s));
    EXPECT_STREQ("A", s);

    strcpy(s, "");
    EXPECT_EQ(s, cstring_toupper(s));
    EXPECT_STREQ("", s);
}

TEST(CstringTest, Trim) {
    char s[256];

    strcpy(s, "foo bar");
    EXPECT_EQ(s, cstring_trim(s));
    EXPECT_STREQ("foo bar", s);

    strcpy(s, " foo bar ");
    EXPECT_EQ(s, cstring_trim(s));
    EXPECT_STREQ("foo bar", s);

    strcpy(s, " \r \n \tfoo bar   \n \r  \t  ");
    EXPECT_EQ(s, cstring_trim(s));
    EXPECT_STREQ("foo bar", s);

    strcpy(s, "a");
    EXPECT_EQ(s, cstring_trim(s));
    EXPECT_STREQ("a", s);

    strcpy(s, "a ");
    EXPECT_EQ(s, cstring_trim(s));
    EXPECT_STREQ("a", s);

    strcpy(s,  " a");
    EXPECT_EQ(s, cstring_trim(s));
    EXPECT_STREQ("a", s);

    strcpy(s, "");
    EXPECT_EQ(s, cstring_trim(s));
    EXPECT_STREQ("", s);

    strcpy(s, " ");
    EXPECT_EQ(s, cstring_trim(s));
    EXPECT_STREQ("", s);
}

TEST(CstringTest, Unquote) {
    char s[16];

    strcpy(s, "\"foo bar\"");
    EXPECT_EQ(s, cstring_unquote(s));
    EXPECT_STREQ("foo bar", s);

    strcpy(s, "\"a\"");
    EXPECT_EQ(s, cstring_unquote(s));
    EXPECT_STREQ("a", s);

    strcpy(s, "\"foo bar");
    EXPECT_EQ(s, cstring_unquote(s));
    EXPECT_STREQ("\"foo bar", s);

    strcpy(s, "foo bar\"");
    EXPECT_EQ(s, cstring_unquote(s));
    EXPECT_STREQ("foo bar\"", s);

    strcpy(s, "\"\"");
    EXPECT_EQ(s, cstring_unquote(s));
    EXPECT_STREQ("", s);

    strcpy(s, "\"");
    EXPECT_EQ(s, cstring_unquote(s));
    EXPECT_STREQ("\"", s);

    strcpy(s, "");
    EXPECT_EQ(s, cstring_unquote(s));
    EXPECT_STREQ("", s);
}
