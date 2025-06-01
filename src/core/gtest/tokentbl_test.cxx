/*
 * Copyright (c) 2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../../common/gtest/stubs/error_stubs.h"
#include "command_stubs.h"
#include "function_stubs.h"
#include "operation_stubs.h"
#include "../tokentbl.h"

}

#if defined(USE_TWO_BYTE_TOKENS)

uint16_t test_data[] = {
    0,         1, 0x00, 0x00, 0x00, 0x00,
    1,         1, 0x01, 0x00, 0x00, 0x00,
    127,       1, 0x7F, 0x00, 0x00, 0x00,
    128,       2, 0x80, 0x80, 0x00, 0x00,
    128 + 1,   2, 0x81, 0x80, 0x00, 0x00,
    128 + 126, 2, 0xFE, 0x80, 0x00, 0x00,
    128 + 127, 2, 0xFF, 0x80, 0x00, 0x00,
    256,       2, 0x80, 0x81, 0x00, 0x00,
    256 + 1,   2, 0x81, 0x81, 0x00, 0x00,
    256 + 126, 2, 0xFE, 0x81, 0x00, 0x00,
    256 + 127, 2, 0xFF, 0x81, 0x00, 0x00,
    384,       2, 0x80, 0x82, 0x00, 0x00,
};

#else

uint16_t test_data[] = {
    0,         1, 0x00, 0x00, 0x00, 0x00,
    1,         1, 0x01, 0x00, 0x00, 0x00,
    127,       1, 0x7F, 0x00, 0x00, 0x00,
    128,       1, 0x80, 0x00, 0x00, 0x00,
    128 + 1,   1, 0x81, 0x00, 0x00, 0x00,
    128 + 126, 1, 0xFE, 0x00, 0x00, 0x00,
    128 + 127, 3, 0xFF, 0xFF, 0x80, 0x00,
    256,       3, 0xFF, 0x80, 0x81, 0x00,
    256 + 1,   3, 0xFF, 0x81, 0x81, 0x00,
    256 + 126, 3, 0xFF, 0xFE, 0x81, 0x00,
    256 + 127, 3, 0xFF, 0xFF, 0x81, 0x00,
    384,       3, 0xFF, 0x80, 0x82, 0x00,
};

#endif

size_t test_data_sz = sizeof(test_data) / sizeof(uint16_t);

class TokentblTest : public ::testing::Test {

protected:

    void SetUp() override {
        tokentbl_init();
    }

    void TearDown() override {
    }
};

TEST_F(TokentblTest, Get) {
    // Expect these to change as functions are added/deleted/removed
    // from tokentbl[].
    EXPECT_EQ(2 + C_BASETOKEN, tokentbl_get("ABS("));
    EXPECT_EQ(90 + C_BASETOKEN, tokentbl_get("VAL("));
    EXPECT_EQ(106 + C_BASETOKEN, tokentbl_get("+"));
}

TEST_F(TokentblTest, Peek) {
    char buf[4] = { '\0' };

    for (size_t i = 0; i < test_data_sz; i += 6) {
        buf[0] = test_data[i + 2];
        buf[1] = test_data[i + 3];
        buf[2] = test_data[i + 4];
        buf[3] = test_data[i + 5];
        char *p = buf;
        EXPECT_EQ(test_data[i], tokentbl_peek(p));
    }
}

TEST_F(TokentblTest, Read) {
    char buf[4] = { '\0' };

    for (size_t i = 0; i < test_data_sz; i += 6) {
        buf[0] = test_data[i + 2];
        buf[1] = test_data[i + 3];
        buf[2] = test_data[i + 4];
        buf[3] = test_data[i + 5];
        const char *p = buf;
        EXPECT_EQ(test_data[i], tokentbl_read(&p));
        EXPECT_EQ(buf + test_data[i + 1], p);
    }
}

TEST_F(TokentblTest, Write) {
    char buf[4] = { '\0' };

    for (size_t i = 0; i < test_data_sz; i += 6) {
        memset(buf, 0, 4);
        char *p = buf;
        EXPECT_EQ(test_data[i + 1], tokentbl_write(&p, test_data[i]));
        EXPECT_EQ(test_data[i + 2], buf[0]);
        EXPECT_EQ(test_data[i + 3], buf[1]);
        EXPECT_EQ(test_data[i + 4], buf[2]);
        EXPECT_EQ(test_data[i + 5], buf[3]);
    }
}
