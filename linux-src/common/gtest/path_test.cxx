#include <climits>
#include <gtest/gtest.h>

extern "C" {

#include "../path.h"
#include "../utility.h"

}

#define PATH_TEST_DIR  "/tmp/PathTest"

class PathTest : public ::testing::Test {

protected:

    void SetUp() override {
        struct stat st = { 0 };
        if (stat(PATH_TEST_DIR, &st) == -1) {
            mkdir(PATH_TEST_DIR, 0775);
        }
        if (stat(PATH_TEST_DIR "/bar", &st) == -1) {
            mkdir(PATH_TEST_DIR "/bar", 0775);
        }
        if (stat("bar", &st) == -1) {
            mkdir("bar", 0775);
        }
    }

    void TearDown() override {
        system("rm -rf " PATH_TEST_DIR);
    }

};

TEST_F(PathTest, GetCanonical_GivenAbsolutePath) {
    char out[256] = { '\0' };
    errno = 0;

    // Simple (non-existing) absolute path.
    char *result = path_get_canonical(PATH_TEST_DIR "/foo.bas", out, 256);

    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas", result);
    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas", out);
    EXPECT_EQ(0, errno);

    // Intermediate slash dot dot.
    result = path_get_canonical(PATH_TEST_DIR "/bar/../foo.bas", out, 256);

    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas", result);
    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas", out);
    EXPECT_EQ(0, errno);

    // Intermediate slash dot.
    result = path_get_canonical(PATH_TEST_DIR "/./foo.bas", out, 256);

    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas", result);
    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas", out);
    EXPECT_EQ(0, errno);

    // Multiple slashes.
    result = path_get_canonical(PATH_TEST_DIR "//foo.bas", out, 256);

    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas", result);
    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas", out);
    EXPECT_EQ(0, errno);

    // Trailing dot.
    result = path_get_canonical(PATH_TEST_DIR "/foo.bas.", out, 256);

    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas.", result);
    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas.", out);
    EXPECT_EQ(0, errno);

    // Trailing dot dot.
    result = path_get_canonical(PATH_TEST_DIR "/foo.bas..", out, 256);

    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas..", result);
    EXPECT_STREQ(PATH_TEST_DIR "/foo.bas..", out);
    EXPECT_EQ(0, errno);

    // Trailing slash dot.
    result = path_get_canonical(PATH_TEST_DIR "/bar/.", out, 256);

    EXPECT_STREQ(PATH_TEST_DIR "/bar", result);
    EXPECT_STREQ(PATH_TEST_DIR "/bar", out);
    EXPECT_EQ(0, errno);

    // Trailing slash dot dot.
    result = path_get_canonical(PATH_TEST_DIR "/bar/..", out, 256);

    EXPECT_STREQ(PATH_TEST_DIR, result);
    EXPECT_STREQ(PATH_TEST_DIR, out);
    EXPECT_EQ(0, errno);
}

TEST_F(PathTest, GetCanonical_GivenRelativePath) {
    char out[256] = { '\0' };
    errno = 0;

    // Simple (non-existing) relative path.
    char *result = path_get_canonical("foo.bas", out, 256);

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
    result = path_get_canonical("bar/../foo.bas", out, 256);

    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

    // Intermediate slash dot.
    result = path_get_canonical("bar/./foo.bas", out, 256);

    sprintf(expected, "%s/bar/foo.bas", cwd);
    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

     // Multiple slashes.
    result = path_get_canonical("bar//foo.bas", out, 256);

    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

    // Trailing dot.
    result = path_get_canonical("foo.bas.", out, 256);

    sprintf(expected, "%s/foo.bas.", cwd);
    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

    // Trailing dot dot.
    result = path_get_canonical("foo.bas..", out, 256);

    sprintf(expected, "%s/foo.bas..", cwd);
    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

    // Trailing slash dot.
    result = path_get_canonical("bar/.", out, 256);

    sprintf(expected, "%s/bar", cwd);
    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);

    // Trailing slash dot.
    result = path_get_canonical("bar/..", out, 256);

    sprintf(expected, "%s", cwd);
    EXPECT_STREQ(expected, result);
    EXPECT_STREQ(expected, out);
    EXPECT_EQ(0, errno);
}

TEST_F(PathTest, GetExtension) {
    const char *filename = "foo.bas";
    const char *empty = "";
    const char *extension_only = ".bas";
    const char *no_extension = "foo";

    EXPECT_STREQ(path_get_extension(filename), ".bas");
    EXPECT_STREQ(path_get_extension(empty), "");
    EXPECT_STREQ(path_get_extension(extension_only), ".bas");
    EXPECT_STREQ(path_get_extension(no_extension), "");
}

TEST_F(PathTest, Exists) {
    EXPECT_EQ(path_exists("/bin/vi"), true);
    EXPECT_EQ(path_exists("/bin/does-not-exist"), false);
}

TEST_F(PathTest, IsDirectory) {
    EXPECT_EQ(true, path_is_directory("/"));
    EXPECT_EQ(true, path_is_directory("/bin"));
    EXPECT_EQ(false, path_is_directory("/bin/vi"));
    EXPECT_EQ(false, path_is_directory("/does-not-exist"));    
}

TEST_F(PathTest, IsEmpty) {
    EXPECT_EQ(path_is_empty("/bin/vi"), 0);

    char filename[] = "/tmp/is_empty_XXXXXX";
    int fd = mkstemp(filename);
    close(fd);
    EXPECT_EQ(path_exists(filename), true);
    EXPECT_EQ(path_is_empty(filename), true);
}

TEST_F(PathTest, IsRegular) {
    EXPECT_EQ(path_is_regular("/bin/vi"), true);
    EXPECT_EQ(path_is_regular("/bin"), false);
}

TEST_F(PathTest, HasSuffix) {
    EXPECT_EQ(path_has_suffix("foo.bas", ".bas", false), true);
    EXPECT_EQ(path_has_suffix("foo.bas", ".BAS", false), false);
    EXPECT_EQ(path_has_suffix("foo.bas", ".BAS", true), true);
    EXPECT_EQ(path_has_suffix("foo.bas", ".inc", true), false);
}

TEST_F(PathTest, MkDir) {
    EXPECT_EQ(kOk, path_mkdir(PATH_TEST_DIR "/ab/cd/ef"));

    EXPECT_TRUE(path_exists(PATH_TEST_DIR));
    EXPECT_TRUE(path_exists(PATH_TEST_DIR "/ab"));
    EXPECT_TRUE(path_exists(PATH_TEST_DIR "/ab/cd"));
    EXPECT_TRUE(path_exists(PATH_TEST_DIR "/ab/cd/ef"));
    EXPECT_TRUE(path_is_directory(PATH_TEST_DIR "/ab/cd/ef"));
}

TEST_F(PathTest, MkDir_GivenExistingDirectory) {
    system("mkdir " PATH_TEST_DIR "/existing-dir");

    EXPECT_EQ(kOk, path_mkdir(PATH_TEST_DIR "/existing-dir"));

    EXPECT_TRUE(path_exists(PATH_TEST_DIR "/existing-dir"));
    EXPECT_TRUE(path_is_directory(PATH_TEST_DIR "/existing-dir"));
}

TEST_F(PathTest, MkDir_GivenExistingFile) {
    system("touch " PATH_TEST_DIR "/existing-file");

    EXPECT_EQ(kFileExists, path_mkdir(PATH_TEST_DIR "/existing-file"));
    EXPECT_EQ(kNotADirectory, path_mkdir(PATH_TEST_DIR "/existing-file/foo"));
}
