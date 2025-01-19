/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../safe_buffer.h"

} // extern "C"

#define TEST_BUFFER_SIZE  32

class SafeBufferTest : public ::testing::Test {

protected:

    void SetUp() override {
        memset(raw, 0, TEST_BUFFER_SIZE);
        raw[TEST_BUFFER_SIZE] = 'x';
        raw[TEST_BUFFER_SIZE + 1] = '\0';
        safe_buffer_init(&buffer, raw, TEST_BUFFER_SIZE);
    }

    void TearDown() override {
    }

    void GivenBufferPartiallyFull(size_t num) {
        for (size_t i = 0; i < num; ++i) {
            EXPECT_EQ(0, safe_buffer_append(&buffer, 'a'));
        }
    }

    void GivenBufferFull() {
        GivenBufferPartiallyFull(TEST_BUFFER_SIZE);
    }

    char raw[TEST_BUFFER_SIZE + 2];
    SafeBuffer buffer;
};

TEST_F(SafeBufferTest, Reset_Succeeds) {
    buffer.overrun = true;
    buffer.ptr = buffer.base + 2;

    safe_buffer_reset(&buffer);

    EXPECT_EQ(false, buffer.overrun);
    EXPECT_EQ(buffer.base, buffer.ptr);
}

TEST_F(SafeBufferTest, Append_Succeeds_GivenBufferNotFull) {
    EXPECT_EQ(0, safe_buffer_append(&buffer, 'a'));
    EXPECT_EQ(false, buffer.overrun);
    EXPECT_STREQ("a", raw);

    EXPECT_EQ(0, safe_buffer_append(&buffer, 'b'));
    EXPECT_EQ(false, buffer.overrun);
    EXPECT_STREQ("ab", raw);
}

TEST_F(SafeBufferTest, Append_Fails_GivenBufferFull) {
    GivenBufferFull();

    EXPECT_EQ(-1, safe_buffer_append(&buffer, 'a'));
    EXPECT_EQ(true, buffer.overrun);
    EXPECT_STREQ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaax", raw);
}

TEST_F(SafeBufferTest, AppendBytes_Succeeds_GivenBufferNotFull) {
    EXPECT_EQ(0, safe_buffer_append_bytes(&buffer, "abc", 3));
    EXPECT_EQ(false, buffer.overrun);
    EXPECT_STREQ("abc", raw);

    EXPECT_EQ(0, safe_buffer_append_bytes(&buffer, "def", 3));
    EXPECT_EQ(false, buffer.overrun);
    EXPECT_STREQ("abcdef", raw);

    EXPECT_EQ(0, safe_buffer_append_bytes(&buffer, "ghi", 1));
    EXPECT_EQ(false, buffer.overrun);
    EXPECT_STREQ("abcdefg", raw);
}

TEST_F(SafeBufferTest, AppendBytes_Fails_GivenBufferFull) {
    GivenBufferFull();

    EXPECT_EQ(-1, safe_buffer_append_bytes(&buffer, "def", 3));
    EXPECT_EQ(true, buffer.overrun);
    EXPECT_STREQ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaax", raw);
}

TEST_F(SafeBufferTest, AppendBytes_Fails_GivenBufferWillFill) {
    GivenBufferPartiallyFull(TEST_BUFFER_SIZE - 2);

    EXPECT_EQ(-1, safe_buffer_append_bytes(&buffer, "DEF", 3));
    EXPECT_EQ(true, buffer.overrun);
    EXPECT_STREQ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaDEx", raw);
}

TEST_F(SafeBufferTest, AppendBytes_Succeeds_GivenZeroBytes) {
    EXPECT_EQ(0, safe_buffer_append_bytes(&buffer, "abc", 0));
    EXPECT_EQ(false, buffer.overrun);
    EXPECT_STREQ("", raw);
}

TEST_F(SafeBufferTest, AppendString_Succeeds_GivenBufferNotFull) {
    EXPECT_EQ(0, safe_buffer_append_string(&buffer, "abc"));
    EXPECT_EQ(false, buffer.overrun);
    EXPECT_STREQ("abc", raw);

    EXPECT_EQ(0, safe_buffer_inc_ptr(&buffer, -1));
    EXPECT_EQ(0, safe_buffer_append_string(&buffer, "def"));
    EXPECT_EQ(false, buffer.overrun);
    EXPECT_STREQ("abcdef", raw);

    EXPECT_EQ(0, safe_buffer_inc_ptr(&buffer, -1));
    EXPECT_EQ(0, safe_buffer_append_string(&buffer, "ghi"));
    EXPECT_EQ(false, buffer.overrun);
    EXPECT_STREQ("abcdefghi", raw);
}

TEST_F(SafeBufferTest, AppendString_Fails_GivenBufferFull) {
    GivenBufferFull();

    EXPECT_EQ(-1, safe_buffer_append_string(&buffer, "def"));
    EXPECT_EQ(true, buffer.overrun);
    EXPECT_STREQ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaax", raw);
}

TEST_F(SafeBufferTest, AppendString_Fails_GivenBufferWillFill) {
    GivenBufferPartiallyFull(TEST_BUFFER_SIZE - 2);

    EXPECT_EQ(-1, safe_buffer_append_string(&buffer, "DEF"));
    EXPECT_EQ(true, buffer.overrun);
    EXPECT_STREQ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaDEx", raw);
}

TEST_F(SafeBufferTest, AppendBytes_Succeeds_GivenEmptyString) {
    EXPECT_EQ(0, safe_buffer_append_string(&buffer, ""));
    EXPECT_EQ(false, buffer.overrun);
    EXPECT_STREQ("", raw);
}

TEST_F(SafeBufferTest, IsFull_ReturnsTrue_GivenFull) {
    GivenBufferFull();

    EXPECT_EQ(true, safe_buffer_is_full(&buffer));
}

TEST_F(SafeBufferTest, IsFull_ReturnsFalse_GivenPartiallyFull) {
    GivenBufferPartiallyFull(TEST_BUFFER_SIZE - 1);

    EXPECT_EQ(false, safe_buffer_is_full(&buffer));
}

TEST_F(SafeBufferTest, IsFull_ReturnsFalse_GivenEmpty) {
    EXPECT_EQ(false, safe_buffer_is_full(&buffer));
}

TEST_F(SafeBufferTest, IncPtr_Succeeds_GivenPositiveIncrement) {
    EXPECT_EQ(0, safe_buffer_inc_ptr(&buffer, 5));
    EXPECT_EQ(buffer.base + 5, buffer.ptr);

    EXPECT_EQ(0, safe_buffer_inc_ptr(&buffer, 10));
    EXPECT_EQ(buffer.base + 15, buffer.ptr);

    EXPECT_EQ(0, safe_buffer_inc_ptr(&buffer, 16));
    EXPECT_EQ(buffer.base + 31, buffer.ptr);
}

TEST_F(SafeBufferTest, IncPtr_Succeeds_GivenNegativeIncrement) {
    EXPECT_EQ(0, safe_buffer_inc_ptr(&buffer, 15));
    EXPECT_EQ(0, safe_buffer_inc_ptr(&buffer, -10));
    EXPECT_EQ(buffer.base + 5, buffer.ptr);

    EXPECT_EQ(0, safe_buffer_inc_ptr(&buffer, -5));
    EXPECT_EQ(buffer.base, buffer.ptr);
}

TEST_F(SafeBufferTest, IncPtr_Fails_GivenPointerWillGoAboveLimit) {
    EXPECT_EQ(-1, safe_buffer_inc_ptr(&buffer, TEST_BUFFER_SIZE));
    EXPECT_EQ(buffer.base, buffer.ptr);
}

TEST_F(SafeBufferTest, IncPtr_Fails_GivenPointerWillGoBelowBase) {
    EXPECT_EQ(-1, safe_buffer_inc_ptr(&buffer, -1));
    EXPECT_EQ(buffer.base, buffer.ptr);
}

TEST_F(SafeBufferTest, IncPtr_Fails_GivenExistingBufferOverrun) {
    buffer.overrun = true;
    EXPECT_EQ(-1, safe_buffer_inc_ptr(&buffer, 1));
    EXPECT_EQ(buffer.base, buffer.ptr);
}

TEST_F(SafeBufferTest, SetPtr_Succeeds_GivenPointerInRange) {
    EXPECT_EQ(0, safe_buffer_set_ptr(&buffer, buffer.base));
    EXPECT_EQ(buffer.base, buffer.ptr);

    EXPECT_EQ(0, safe_buffer_set_ptr(&buffer, buffer.limit));
    EXPECT_EQ(buffer.limit, buffer.ptr);
}

TEST_F(SafeBufferTest, SetPtr_Fails_GivenPointerAboveLimit) {
    EXPECT_EQ(-1, safe_buffer_set_ptr(&buffer, buffer.limit + 1));
    EXPECT_EQ(buffer.base, buffer.ptr);
}

TEST_F(SafeBufferTest, SetPtr_Fails_GivenPointerBelowBase) {
    EXPECT_EQ(-1, safe_buffer_set_ptr(&buffer, buffer.base - 1));
    EXPECT_EQ(buffer.base, buffer.ptr);
}

TEST_F(SafeBufferTest, SetPtr_Fails_GivenExistingBufferOverrun) {
    buffer.overrun = true;
    EXPECT_EQ(-1, safe_buffer_set_ptr(&buffer, buffer.base + 1));
    EXPECT_EQ(buffer.base, buffer.ptr);
}

TEST_F(SafeBufferTest, Last_Succeeds_GivenSomethingAppended) {
    safe_buffer_append_bytes(&buffer, "abc", 3);

    EXPECT_EQ('c', safe_buffer_last(&buffer));
}

TEST_F(SafeBufferTest, Last_Fails_GivenNothingAppended) {
    EXPECT_EQ(-1, safe_buffer_last(&buffer));
}

TEST_F(SafeBufferTest, Last_Fails_GivenExistingBufferOverrun) {
    GivenBufferFull();
    EXPECT_EQ('a', safe_buffer_last(&buffer));

    safe_buffer_append(&buffer, 'a');
    EXPECT_EQ(-1, safe_buffer_last(&buffer));
}
