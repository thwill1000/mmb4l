#include <climits>
#include <gtest/gtest.h>

extern "C" {

#include "../path.h"
#include "../utility.h"

}

#define PATH_TEST_DIR     "/tmp/PathTest"
#define FILE_THAT_EXISTS  "/bin/cp"

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

#define TEST_MUNGE(path, expected)  memset(out, '\0', 256); \
        result = path_munge(path, out, 256); \
        EXPECT_STREQ(expected, result); \
        EXPECT_STREQ(expected, out); \
        EXPECT_EQ(0, errno)

#define HOME_DIR "/home/thwill"

TEST_F(PathTest, Munge) {
    char out[256];
    errno = 0;
    char *result;

    // Absolute paths.
    TEST_MUNGE("a:",              "/");
    TEST_MUNGE("A:",              "/");
    TEST_MUNGE("c:",              "/");
    TEST_MUNGE("C:",              "/");
    TEST_MUNGE("a:/",             "/");
    TEST_MUNGE("A:/",             "/");
    TEST_MUNGE("c:/",             "/");
    TEST_MUNGE("C:/",             "/");
    TEST_MUNGE("/",               "/");
    TEST_MUNGE("//",              "/");
    TEST_MUNGE("///",             "/");
    TEST_MUNGE("/.",              "/");
    TEST_MUNGE("/..",             "/..");
    TEST_MUNGE("/./..",           "/..");
    TEST_MUNGE("/./foo",          "/foo");
    TEST_MUNGE("/foo/.",          "/foo");
    TEST_MUNGE("/foo/..",         "/");
    TEST_MUNGE("/foo/../bar",     "/bar");
    TEST_MUNGE("/foo/../bar.bas", "/bar.bas");
    TEST_MUNGE("/foo/.bar",       "/foo/.bar");
    TEST_MUNGE("/foo/..bar",      "/foo/..bar");
    TEST_MUNGE("/foo../bar",      "/foo../bar");
    TEST_MUNGE("/foo../..",       "/");
    TEST_MUNGE("/foo../../bar",   "/bar");
    TEST_MUNGE("/foo//bar",       "/foo/bar");

    // Relative paths.
    TEST_MUNGE(".",               "");
    TEST_MUNGE("..",              "..");
    TEST_MUNGE("./..",            "..");
    TEST_MUNGE("./foo",           "foo");
    TEST_MUNGE("foo/.",           "foo");
    TEST_MUNGE("foo/..",          "");
    TEST_MUNGE("foo/../bar",      "bar");
    TEST_MUNGE("foo/../bar.bas",  "bar.bas");
    TEST_MUNGE("foo/.bar",        "foo/.bar");
    TEST_MUNGE("foo/..bar",       "foo/..bar");
    TEST_MUNGE("foo../bar",       "foo../bar");
    TEST_MUNGE("foo../..",        "");
    TEST_MUNGE("foo../../bar",    "bar");
    TEST_MUNGE("foo//bar",        "foo/bar");
    TEST_MUNGE("../foo",          "../foo");
    TEST_MUNGE("../../foo",       "../../foo");
    TEST_MUNGE("../../../foo",    "../../../foo");

    // HOME relative paths.
    TEST_MUNGE("~",               HOME_DIR);
    TEST_MUNGE("~/",              HOME_DIR);
    TEST_MUNGE("~/.",             HOME_DIR);
    TEST_MUNGE("~/../foo",        "/home/foo");
    TEST_MUNGE("~/.mmbasic",      HOME_DIR "/.mmbasic");
    TEST_MUNGE("~/../../tmp",     "/tmp");

    // Test with backslashes.
    TEST_MUNGE("a:\\",            "/");
    TEST_MUNGE("\\",              "/");
    TEST_MUNGE("\\.",             "/");
    TEST_MUNGE("\\..",            "/..");
    TEST_MUNGE("~\\..\\foo",      "/home/foo");
    TEST_MUNGE("foo\\bar",        "foo/bar");
    TEST_MUNGE("..\\..\\..\\foo", "../../../foo");
}

#define TEST_GET_CANONICAL(path, expected)  memset(out, '\0', 256); \
        result = path_get_canonical(path, out, 256); \
        EXPECT_STREQ(expected, result); \
        EXPECT_STREQ(expected, out); \
        EXPECT_EQ(0, errno)

TEST_F(PathTest, GetCanonical_GivenAbsolutePath) {
    char out[256];
    errno = 0;
    char *result;

    // Simple (non-existing) absolute path.
    TEST_GET_CANONICAL(PATH_TEST_DIR "/foo.bas", PATH_TEST_DIR "/foo.bas");

    // Intermediate slash dot dot.
    TEST_GET_CANONICAL(PATH_TEST_DIR "/bar/../foo.bas", PATH_TEST_DIR "/foo.bas");

    // Intermediate slash dot.
    TEST_GET_CANONICAL(PATH_TEST_DIR "/./foo.bas", PATH_TEST_DIR "/foo.bas");

    // Multiple slashes.
    TEST_GET_CANONICAL(PATH_TEST_DIR "//foo.bas", PATH_TEST_DIR "/foo.bas");

    // Trailing dot.
    TEST_GET_CANONICAL(PATH_TEST_DIR "/foo.bas.", PATH_TEST_DIR "/foo.bas.");

    // Trailing dot dot.
    TEST_GET_CANONICAL(PATH_TEST_DIR "/foo.bas..", PATH_TEST_DIR "/foo.bas..");

    // Trailing slash dot.
    TEST_GET_CANONICAL(PATH_TEST_DIR "/bar/.", PATH_TEST_DIR "/bar");

    // Trailing slash dot dot.
    TEST_GET_CANONICAL(PATH_TEST_DIR "/bar/..", PATH_TEST_DIR);
}

TEST_F(PathTest, GetCanonical_GivenRelativePath) {
    char out[256];
    errno = 0;
    char *result;
    char expected[PATH_MAX + 256];

    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        perror("getcwd() error");
        return;
    }

    // Simple (non-existing) relative path.
    sprintf(expected, "%s/foo.bas", cwd);
    TEST_GET_CANONICAL("foo.bas", expected);

    // Intermediate slash dot dot.
    sprintf(expected, "%s/foo.bas", cwd);
    TEST_GET_CANONICAL("bar/../foo.bas", expected);

    // Intermediate slash dot.
    sprintf(expected, "%s/bar/foo.bas", cwd);
    TEST_GET_CANONICAL("bar/./foo.bas", expected);

     // Multiple slashes.
    sprintf(expected, "%s/bar/foo.bas", cwd);
    TEST_GET_CANONICAL("bar//foo.bas", expected);

    // Trailing dot.
    sprintf(expected, "%s/foo.bas.", cwd);
    TEST_GET_CANONICAL("foo.bas.", expected);

    // Trailing dot dot.
    sprintf(expected, "%s/foo.bas..", cwd);
    TEST_GET_CANONICAL("foo.bas..", expected);

    // Trailing slash dot.
    sprintf(expected, "%s/bar", cwd);
    TEST_GET_CANONICAL("bar/.", expected);

    // Trailing slash dot dot.
    sprintf(expected, "%s", cwd);
    TEST_GET_CANONICAL("bar/..", expected);
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
    EXPECT_EQ(path_exists(FILE_THAT_EXISTS), true);
    EXPECT_EQ(path_exists("/bin/does-not-exist"), false);
}

TEST_F(PathTest, IsDirectory) {
    EXPECT_EQ(true, path_is_directory("/"));
    EXPECT_EQ(true, path_is_directory("/bin"));
    EXPECT_EQ(false, path_is_directory(FILE_THAT_EXISTS));
    EXPECT_EQ(false, path_is_directory("/does-not-exist"));    
}

TEST_F(PathTest, IsEmpty) {
    EXPECT_EQ(path_is_empty(FILE_THAT_EXISTS), 0);

    char filename[] = "/tmp/is_empty_XXXXXX";
    int fd = mkstemp(filename);
    close(fd);
    EXPECT_EQ(path_exists(filename), true);
    EXPECT_EQ(path_is_empty(filename), true);
}

TEST_F(PathTest, IsRegular) {
    EXPECT_EQ(path_is_regular(FILE_THAT_EXISTS), true);
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
