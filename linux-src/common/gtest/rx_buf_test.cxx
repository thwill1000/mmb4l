/*
 * Copyright (c) 2021-2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "../rx_buf.h"

}

TEST(RxBufTest, Init) {
    char data[16] = { 0 };
    RxBuf buf;
    rx_buf_init(&buf, data, sizeof(data));

    EXPECT_EQ(buf.head, 0);
    EXPECT_EQ(buf.tail, 0);
    EXPECT_EQ(buf.data, data);
    EXPECT_EQ(buf.data_sz, 16);
}

TEST(RxBufTest, Put) {
    char data[16] = { 0 };
    RxBuf buf;
    rx_buf_init(&buf, data, sizeof(data));

    EXPECT_EQ(rx_buf_put(&buf, 5), 0);

    {
        int expected[16] = { 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        EXPECT_THAT(std::vector<int>(data, data + 16),
                    ::testing::ElementsAreArray(expected));
        EXPECT_EQ(buf.head, 1);
        EXPECT_EQ(buf.tail, 0);
    }

    EXPECT_EQ(rx_buf_put(&buf, 255), 0);

    {
        int expected[16] = { 5, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        EXPECT_THAT(std::vector<int>(data, data + 16),
                    ::testing::ElementsAreArray(expected));
        EXPECT_EQ(buf.head, 2);
        EXPECT_EQ(buf.tail, 0);
    }
}

TEST(RxBufTest, Size) {
    char data[16] = { 0 };
    RxBuf buf;
    rx_buf_init(&buf, data, sizeof(data));


    EXPECT_EQ(rx_buf_size(&buf), 0);

    rx_buf_put(&buf, 5);
    rx_buf_put(&buf, 255);

    EXPECT_EQ(rx_buf_size(&buf), 2);
}

TEST(RxBufTest, Get) {
    char data[16] = { 0 };
    RxBuf buf;
    rx_buf_init(&buf, data, sizeof(data));

    for (int i = 1; i < 16; ++i) {
        rx_buf_put(&buf, i);
    }

    for (int i = 1; i < 16; ++i) {
        EXPECT_EQ(rx_buf_get(&buf), i);
    }
}

TEST(RxBufTest, GetGivenBufferEmpty) {
    char data[16] = { 0 };
    RxBuf buf;
    rx_buf_init(&buf, data, sizeof(data));

    EXPECT_EQ(rx_buf_get(&buf), -1);

    for (int i = 1; i < 8; ++i) rx_buf_put(&buf, i);
    for (int i = 1; i < 8; ++i) rx_buf_get(&buf);

    EXPECT_EQ(rx_buf_get(&buf), -1);
}

TEST(RxBufTest, Clear) {
    char data[16] = { 0 };
    RxBuf buf;
    rx_buf_init(&buf, data, sizeof(data));

    rx_buf_put(&buf, 5);
    rx_buf_put(&buf, 255);

    rx_buf_clear(&buf);

    EXPECT_EQ(buf.head, 0);
    EXPECT_EQ(buf.tail, 0);
    EXPECT_EQ(rx_buf_size(&buf), 0);
}

TEST(RxBufTest, Unget) {
    char data[16] = { 0 };
    RxBuf buf;
    rx_buf_init(&buf, data, sizeof(data));

    // Fill the buffer.
    for (int i = 1; i < 16; ++i) {
        rx_buf_put(&buf, i);
    }

    for (int i = 1; i < 16; ++i) {
        EXPECT_EQ(rx_buf_get(&buf), i);
        for (int j = i; j > 0; --j) {
            EXPECT_EQ(rx_buf_unget(&buf, 0xED), 0);
        }
        for (int j = 1; j <= i; ++j) {
            EXPECT_EQ(rx_buf_get(&buf), 0xED);
        }
    }

    EXPECT_EQ(rx_buf_size(&buf), 0);
}

TEST(RxBufTest, UngetMoreItemsThanBufferCanHold) {
    char data[16] = { 0 };
    RxBuf buf;
    rx_buf_init(&buf, data, sizeof(data));


    for (int i = 1; i <= 100; ++i) {
        EXPECT_EQ(rx_buf_unget(&buf, i), i > 15 ? -1 : 0);
    }

    EXPECT_EQ(rx_buf_size(&buf), 15);

    for (int i = 100; i > 85; --i) {
        EXPECT_EQ(rx_buf_get(&buf), i);
    }

    EXPECT_EQ(rx_buf_size(&buf), 0);
}

TEST(RxBufTest, PutGivenBufferFull) {
    char data[16] = { 0 };
    RxBuf buf;
    rx_buf_init(&buf, data, sizeof(data));

    // Fill the buffer
    for (int i = 1; i < 16; ++i) {
        EXPECT_EQ(rx_buf_put(&buf, i), 0);
    }

    // Put a character in a full buffer.
    EXPECT_EQ(rx_buf_put(&buf, 0xED), -1);

    // Empty the buffer
    for (int i = 2; i < 16; ++i) {
        EXPECT_EQ(rx_buf_get(&buf), i);
    }
    EXPECT_EQ(rx_buf_get(&buf), 0xED);

    EXPECT_EQ(rx_buf_get(&buf), -1);
    EXPECT_EQ(rx_buf_size(&buf), 0);
}

TEST(RxBufTest, CircularBuffer) {
    char data[16] = { 0 };
    RxBuf buf;
    rx_buf_init(&buf, data, sizeof(data));


    for (int i = 1; i <= 255; ++i) {  // Fill the buffer
        if (i < 16) {
            EXPECT_EQ(rx_buf_put(&buf, i), 0);
        } else {
            EXPECT_EQ(rx_buf_get(&buf), i + 1 - 16);
            EXPECT_EQ(rx_buf_put(&buf, i), 0);
        }
    }

    for (int i = 1; i < 16; ++i) {
        EXPECT_EQ(rx_buf_get(&buf), i + 240);
    }

    EXPECT_EQ(rx_buf_size(&buf), 0);
}
