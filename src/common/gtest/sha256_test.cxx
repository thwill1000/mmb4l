/*
 * Copyright (c) 2026-2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <cstring>
#include <string>

extern "C" {

#include "../sha256.h"

} // extern "C"

class Sha256Test : public ::testing::Test {
protected:
    // Verifies s is exactly 64 lowercase hex characters.
    bool IsFullSha256Hex(const char* s) {
        if (strlen(s) != 64) return false;
        return strspn(s, "0123456789abcdef") == 64;
    }
};

// ---------------------------------------------------------------------------
// NIST / known-answer vectors
// ---------------------------------------------------------------------------

TEST_F(Sha256Test, KnownVector_Empty) {
    char buf[65];
    ASSERT_EQ(sha256_string("", buf, sizeof(buf)), kOk);
    EXPECT_STREQ(buf, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    EXPECT_TRUE(IsFullSha256Hex(buf));
}

TEST_F(Sha256Test, KnownVector_hello) {
    char buf[65];
    ASSERT_EQ(sha256_string("hello", buf, sizeof(buf)), kOk);
    EXPECT_STREQ(buf, "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
    EXPECT_TRUE(IsFullSha256Hex(buf));
}

TEST_F(Sha256Test, KnownVector_abc) {
    char buf[65];
    ASSERT_EQ(sha256_string("abc", buf, sizeof(buf)), kOk);
    EXPECT_STREQ(buf, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    EXPECT_TRUE(IsFullSha256Hex(buf));
}

TEST_F(Sha256Test, KnownVector_LongNistString) {
    // NIST vector: "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
    // Expected:    248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1
    char buf[65];
    ASSERT_EQ(sha256_string(
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
        buf, sizeof(buf)), kOk);
    EXPECT_STREQ(buf, "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    EXPECT_TRUE(IsFullSha256Hex(buf));
}

TEST_F(Sha256Test, KnownVector_ExactlyOneBlock) {
    // 55 bytes of 'a' — pads to exactly one 64-byte block
    char buf[65];
    ASSERT_EQ(sha256_string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
                            buf, sizeof(buf)), kOk);
    EXPECT_TRUE(IsFullSha256Hex(buf));
}

TEST_F(Sha256Test, KnownVector_SpansThreeBlocks) {
    // 200 bytes — exercises multi-block path
    std::string input(200, 'z');
    char buf[65];
    ASSERT_EQ(sha256_string(input.c_str(), buf, sizeof(buf)), kOk);
    EXPECT_TRUE(IsFullSha256Hex(buf));
}

// ---------------------------------------------------------------------------
// Boundary conditions on buf_sz
// ---------------------------------------------------------------------------

TEST_F(Sha256Test, Buffer_ZeroSize_ReturnsError) {
    char buf[65];
    EXPECT_NE(sha256_string("hello", buf, 0), kOk);
}

TEST_F(Sha256Test, Buffer_SizeOne_WritesOnlyNullTerminator) {
    char buf[1] = { '\xff' };
    ASSERT_EQ(sha256_string("hello", buf, sizeof(buf)), kOk);
    EXPECT_EQ(buf[0], '\0');
    EXPECT_EQ(strlen(buf), 0u);
}

TEST_F(Sha256Test, Buffer_SizeTwo_WritesOneHexChar) {
    char buf[2];
    ASSERT_EQ(sha256_string("hello", buf, sizeof(buf)), kOk);
    EXPECT_EQ(strlen(buf), 1u);
    EXPECT_NE(strchr("0123456789abcdef", buf[0]), nullptr);
}

TEST_F(Sha256Test, Buffer_OddSize_TruncatesCleanly) {
    // buf_sz=10 → 9 hex chars + '\0'
    char buf[10];
    ASSERT_EQ(sha256_string("hello", buf, sizeof(buf)), kOk);
    EXPECT_EQ(strlen(buf), 9u);
    EXPECT_EQ(buf[9], '\0');
    EXPECT_EQ(std::string(buf), "2cf24dba5");
}

TEST_F(Sha256Test, Buffer_ExactSize64_MissingNullTerminator) {
    // buf_sz=64 → only 63 hex chars fit (last slot is '\0')
    char buf[64];
    ASSERT_EQ(sha256_string("hello", buf, sizeof(buf)), kOk);
    EXPECT_EQ(strlen(buf), 63u);
    EXPECT_EQ(buf[63], '\0');
    // Must match the first 63 chars of the known hash
    EXPECT_EQ(std::string(buf),
              "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b982");
}

TEST_F(Sha256Test, Buffer_ExactSize65_WritesFullHash) {
    char buf[65];
    ASSERT_EQ(sha256_string("hello", buf, sizeof(buf)), kOk);
    EXPECT_EQ(strlen(buf), 64u);
    EXPECT_TRUE(IsFullSha256Hex(buf));
}

// ---------------------------------------------------------------------------
// Null / invalid pointer robustness
// ---------------------------------------------------------------------------

TEST_F(Sha256Test, NullInput_ReturnsError) {
    char buf[65];
    EXPECT_NE(sha256_string(nullptr, buf, sizeof(buf)), kOk);
}

TEST_F(Sha256Test, NullBuffer_ReturnsError) {
    EXPECT_NE(sha256_string("hello", nullptr, 64), kOk);
}

TEST_F(Sha256Test, NullBoth_ReturnsError) {
    EXPECT_NE(sha256_string(nullptr, nullptr, 0), kOk);
}

// ---------------------------------------------------------------------------
// Determinism
// ---------------------------------------------------------------------------

TEST_F(Sha256Test, Deterministic_SameInputSameOutput) {
    char buf1[65], buf2[65];
    ASSERT_EQ(sha256_string("determinism check", buf1, sizeof(buf1)), kOk);
    ASSERT_EQ(sha256_string("determinism check", buf2, sizeof(buf2)), kOk);
    EXPECT_STREQ(buf1, buf2);
}

TEST_F(Sha256Test, Deterministic_DifferentInputsDifferentOutputs) {
    char buf1[65], buf2[65];
    ASSERT_EQ(sha256_string("input_a", buf1, sizeof(buf1)), kOk);
    ASSERT_EQ(sha256_string("input_b", buf2, sizeof(buf2)), kOk);
    EXPECT_STRNE(buf1, buf2);
}
