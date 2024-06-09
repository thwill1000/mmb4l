/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../bitset.h"

} // extern "C"

TEST(BitsetTest, SetGetClear) {
    int bitset_256[32 / sizeof(int)] = { 0 }; // 256 bits.
    bool expected[256] = { 0 };

    // Set bit zero.
    bitset_set(bitset_256, 0);
    expected[0] = true;
    for (size_t i = 0; i < 256; ++i) EXPECT_EQ(expected[i], bitset_get(bitset_256, i));

    // Clear bit zero.
    bitset_clear(bitset_256, 0);
    expected[0] = false;
    for (size_t i = 0; i < 256; ++i) EXPECT_EQ(expected[i], bitset_get(bitset_256, i));

    // Try setting all the bits.
    for (size_t i = 0; i < 256; ++i) {
        bitset_set(bitset_256, i);
        expected[i] = true;
        for (size_t j = 0; j < 256; ++j) EXPECT_EQ(expected[j], bitset_get(bitset_256, j));
    }

    // Try clearing all the bits.
    for (size_t i = 0; i < 256; ++i) {
        bitset_clear(bitset_256, i);
        expected[i] = false;
        for (size_t j = 0; j < 256; ++j) EXPECT_EQ(expected[j], bitset_get(bitset_256, j));
    }
}

TEST(BitsetTest, Reset) {
    int bitset_256[32 / sizeof(int)] = { 0 }; // 256 bits.

    // Set all the bits.
    for (size_t i = 0; i < 256; ++i) {
        bitset_set(bitset_256, i);
    }

    bitset_reset(bitset_256, 256);
    for (size_t j = 0; j < 256; ++j) EXPECT_EQ(false, bitset_get(bitset_256, j));
}
