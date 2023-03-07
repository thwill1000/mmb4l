/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../hash.h"

} // extern "C"

TEST(HashTest, HashCstring) {
    EXPECT_EQ(2851307223, hash_cstring("foo", 32));

    // Only hash the first 3 values.
    EXPECT_EQ(2851307223, hash_cstring("foobar", 3));

    EXPECT_EQ(3214735720, hash_cstring("foobar", 32));

    // Different case, different hash.
    EXPECT_EQ(2649032616, hash_cstring("FOOBAR", 32));

    // Hash of the empty string.
    EXPECT_EQ(2166136261, hash_cstring("", 32));
}
