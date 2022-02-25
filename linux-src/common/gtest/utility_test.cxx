#include <gtest/gtest.h>
#include <limits.h>

extern "C" {

#include "../../Configuration.h"
#include "../utility.h"

}

#define UTILITY_TEST_DIR  "/tmp/UtilityTest"

class UtilityTest : public ::testing::Test {

protected:

    void SetUp() override {
        struct stat st = { 0 };
        if (stat(UTILITY_TEST_DIR, &st) == -1) {
            mkdir(UTILITY_TEST_DIR, 0775);
        }
        if (stat(UTILITY_TEST_DIR "/bar", &st) == -1) {
            mkdir(UTILITY_TEST_DIR "/bar", 0775);
        }
        if (stat("bar", &st) == -1) {
            mkdir("bar", 0775);
        }
    }

    void TearDown() override {
        if (FAILED(rmdir("bar"))) perror("rmdir(\"bar\") failed");
        if (FAILED(rmdir(UTILITY_TEST_DIR "/bar"))) perror("rmdir(\"" UTILITY_TEST_DIR "/bar\") failed");
        if (FAILED(rmdir(UTILITY_TEST_DIR))) perror("rmdir(\"" UTILITY_TEST_DIR "\") failed");
    }

};

TEST_F(UtilityTest, CanonicalizePath_GivenAbsolutePath) {
    char out[STRINGSIZE] = { '\0' };
    errno = 0;

    // Simple (non-existing) absolute path.
    char *result = canonicalize_path(UTILITY_TEST_DIR "/foo.bas", out, STRINGSIZE);

    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas", result);
    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas", out);
    EXPECT_EQ(0, errno);

    // Intermediate slash dot dot.
    result = canonicalize_path(UTILITY_TEST_DIR "/bar/../foo.bas", out, STRINGSIZE);

    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas", result);
    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas", out);
    EXPECT_EQ(0, errno);

    // Intermediate slash dot.
    result = canonicalize_path(UTILITY_TEST_DIR "/./foo.bas", out, STRINGSIZE);

    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas", result);
    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas", out);
    EXPECT_EQ(0, errno);

    // Multiple slashes.
    result = canonicalize_path(UTILITY_TEST_DIR "//foo.bas", out, STRINGSIZE);

    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas", result);
    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas", out);
    EXPECT_EQ(0, errno);

    // Trailing dot.
    result = canonicalize_path(UTILITY_TEST_DIR "/foo.bas.", out, STRINGSIZE);

    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas.", result);
    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas.", out);
    EXPECT_EQ(0, errno);

    // Trailing dot dot.
    result = canonicalize_path(UTILITY_TEST_DIR "/foo.bas..", out, STRINGSIZE);

    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas..", result);
    EXPECT_STREQ(UTILITY_TEST_DIR "/foo.bas..", out);
    EXPECT_EQ(0, errno);

    // Trailing slash dot.
    result = canonicalize_path(UTILITY_TEST_DIR "/bar/.", out, STRINGSIZE);

    EXPECT_STREQ(UTILITY_TEST_DIR "/bar", result);
    EXPECT_STREQ(UTILITY_TEST_DIR "/bar", out);
    EXPECT_EQ(0, errno);

    // Trailing slash dot dot.
    result = canonicalize_path(UTILITY_TEST_DIR "/bar/..", out, STRINGSIZE);

    EXPECT_STREQ(UTILITY_TEST_DIR, result);
    EXPECT_STREQ(UTILITY_TEST_DIR, out);
    EXPECT_EQ(0, errno);
}

TEST_F(UtilityTest, CanonicalizePath_GivenRelativePath) {
    char out[STRINGSIZE] = { '\0' };
    errno = 0;

    // Simple (non-existing) relative path.
    char *result = canonicalize_path("foo.bas", out, STRINGSIZE);

    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        perror("getcwd() error");
        return;
    }
    char expected[PATH_MAX + 256];
    sprintf(expected, "%s/foo.bas", cwd);
    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

    // Intermediate slash dot dot.
    result = canonicalize_path("bar/../foo.bas", out, STRINGSIZE);

    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

    // Intermediate slash dot.
    result = canonicalize_path("bar/./foo.bas", out, STRINGSIZE);

    sprintf(expected, "%s/bar/foo.bas", cwd);
    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

     // Multiple slashes.
    result = canonicalize_path("bar//foo.bas", out, STRINGSIZE);

    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

    // Trailing dot.
    result = canonicalize_path("foo.bas.", out, STRINGSIZE);

    sprintf(expected, "%s/foo.bas.", cwd);
    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

    // Trailing dot dot.
    result = canonicalize_path("foo.bas..", out, STRINGSIZE);

    sprintf(expected, "%s/foo.bas..", cwd);
    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

    // Trailing slash dot.
    result = canonicalize_path("bar/.", out, STRINGSIZE);

    sprintf(expected, "%s/bar", cwd);
    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

    // Trailing slash dot.
    result = canonicalize_path("bar/..", out, STRINGSIZE);

    sprintf(expected, "%s", cwd);
    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);
}

TEST_F(UtilityTest, StrReplace) {
    char s[256] = "cat dog fish cat";

    str_replace(s, "cat", "elephant");

    EXPECT_STREQ("elephant dog fish elephant", s);
}
