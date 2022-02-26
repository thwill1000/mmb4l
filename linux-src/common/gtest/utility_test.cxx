#include <gtest/gtest.h>

extern "C" {

#include "../utility.h"

}

TEST(UtilityTest, StrReplace) {
    char s[256] = "cat dog fish cat";

    str_replace(s, "cat", "elephant");

    EXPECT_STREQ("elephant dog fish elephant", s);
}
