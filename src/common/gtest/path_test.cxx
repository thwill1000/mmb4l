/*
 * Copyright (c) 2021-2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <climits>
#include <filesystem>
#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.

extern "C" {

#include "stubs/error_stubs.h"
#include "../file.h"
#include "../path.h"
#include "../utility.h"

// Defined in "common/prompt.c"
MmResult prompt_getc(int *ch) {
    *ch = -1;
    return kOk;
}

// Defined in "core/MMBasic.c"
const char *GetIntAddress(const char *p) { return NULL; }
MMINTEGER getinteger(char *p) { return 0; }
void makeargs(const char **tp, int maxargs, char *argbuf, char *argv[], int *argc,
              const char *delim) {}
int LocalIndex = 0;

}

class PathTest : public ::testing::Test {

protected:

    std::string cwd;
    std::string home;
    std::filesystem::path test_dir;

    void SetUp() override {
        // Create a temporary test directory structure
        test_dir = std::filesystem::temp_directory_path() / "path_test";

        // Clean up any existing test directory
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }

        // Create test directory structure
        std::filesystem::create_directories(test_dir);

        // Store current working directory
        char cwd_[PATH_MAX];
        ASSERT_EQ(kOk, file_getcwd(cwd_, sizeof(cwd_)));
        cwd = cwd_;

        // Store user's home directory
        char home_[PATH_MAX];
        ASSERT_EQ(kOk, file_get_home(home_, sizeof(home_)));
        home = home_;
    }

    void TearDown() override {
        // Restore current working directory.
        ASSERT_EQ(kOk, file_chdir(cwd.c_str()));

        // Clean up test directory
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

};

#define TEST_GET_PARENT(path, expected) \
        EXPECT_EQ(kOk, path_get_parent(path, out, 256)); \
        EXPECT_STREQ(expected, out)

TEST_F(PathTest, GetParent) {
    char out[256];

    TEST_GET_PARENT("/", "/");
    TEST_GET_PARENT("/..", "/");
    TEST_GET_PARENT("/..", "/");
    TEST_GET_PARENT("/foo", "/");
    TEST_GET_PARENT("/foo/bar", "/foo");

    TEST_GET_PARENT("\\", "/");
    TEST_GET_PARENT("\\foo", "/");
    TEST_GET_PARENT("\\foo\\bar", "/foo");

    EXPECT_EQ(kFileNotFound, path_get_parent("", out, 256));

    // Are these the answers we want ?
    EXPECT_EQ(kFileNotFound, path_get_parent("foo", out, 256));
    EXPECT_EQ(kFileNotFound, path_get_parent(".", out, 256));
    EXPECT_EQ(kFileNotFound, path_get_parent("..", out, 256));
    EXPECT_EQ(kFileNotFound, path_get_parent("./foo", out, 256));
    TEST_GET_PARENT("../foo" , "..");
}

extern "C" {
char *path_unwind(char *new_path, char *pdst);
}

TEST_F(PathTest, Unwind) {
    char path[256];
    char *p;

    strcpy(path, "");
    p = path_unwind(path, path + strlen(path));
    EXPECT_STREQ("", p);

    strcpy(path, "/foo/..");
    p = path_unwind(path, path + strlen(path));
    EXPECT_STREQ("", p);  // trailing .. prevents unwinding.

    strcpy(path, "/foo..");
    p = path_unwind(path, path + strlen(path));
    EXPECT_STREQ("/foo..", p);

    strcpy(path, "/foo/bar");
    p = path_unwind(path, path + strlen(path));
    EXPECT_STREQ("/bar", p);

    strcpy(path, "/foo/bar");
    p = path_unwind(path, path + 4);
    EXPECT_STREQ("/foo/bar", p);

    strcpy(path, "/foo");
    p = path_unwind(path, path + strlen(path));
    EXPECT_STREQ("/foo", p);

    strcpy(path, "foo");
    p = path_unwind(path, path + strlen(path));
    EXPECT_STREQ("foo", p);

    strcpy(path, "..");
    p = path_unwind(path, path + strlen(path));
    EXPECT_STREQ("", p); // trailing .. prevents unwinding.

    strcpy(path, "...");
    p = path_unwind(path, path + strlen(path));
    EXPECT_STREQ("...", p);

    strcpy(path, "../foo");
    p = path_unwind(path, path + strlen(path));
    EXPECT_STREQ("/foo", p);
}

#define TEST_MUNGE(path, expected)  memset(out, '\0', 256); \
        result = path_munge(path, out, 256); \
        EXPECT_EQ(kOk, result); \
        EXPECT_STREQ(expected, out)

TEST_F(PathTest, Munge_Succeeds) {
    char out[256];
    char home_parent[PATH_MAX];
    ASSERT_EQ(kOk, file_dirname(home.c_str(), home_parent, sizeof(home_parent)));
    MmResult result;

    // Absolute paths.
#if defined(_WIN32)
    TEST_MUNGE("a:",              "a:");
    TEST_MUNGE("A:",              "A:");
    TEST_MUNGE("c:",              "c:");
    TEST_MUNGE("C:",              "C:");
    TEST_MUNGE("a:/",             "a:"); // TODO: Shouldn't these have trailing slash?
    TEST_MUNGE("A:/",             "A:");
    TEST_MUNGE("c:/",             "c:");
    TEST_MUNGE("C:/",             "C:");
#else
    TEST_MUNGE("a:",              "/");
    TEST_MUNGE("A:",              "/");
    TEST_MUNGE("c:",              "/");
    TEST_MUNGE("C:",              "/");
    TEST_MUNGE("a:/",             "/");
    TEST_MUNGE("A:/",             "/");
    TEST_MUNGE("c:/",             "/");
    TEST_MUNGE("C:/",             "/");
#endif
    TEST_MUNGE("/",               "/");
    TEST_MUNGE("//",              "/");
    TEST_MUNGE("///",             "/");
    TEST_MUNGE("/.",              "/");
    TEST_MUNGE("/..",             "/");
    TEST_MUNGE("/./..",           "/");
    TEST_MUNGE("/../..",          "/");
    TEST_MUNGE("/foo/..",         "/");
    TEST_MUNGE("/foo/../..",      "/");
    TEST_MUNGE("/foo",            "/foo");
    TEST_MUNGE("/./foo",          "/foo");
    TEST_MUNGE("/foo/.",          "/foo");
    TEST_MUNGE("/foo/../bar",     "/bar");
    TEST_MUNGE("/foo/../bar.bas", "/bar.bas");
    TEST_MUNGE("/foo/.bar",       "/foo/.bar");
    TEST_MUNGE("/foo/..bar",      "/foo/..bar");
    TEST_MUNGE("/foo../bar",      "/foo../bar");
    TEST_MUNGE("/foo../..",       "/");
    TEST_MUNGE("/foo../../bar",   "/bar");
    TEST_MUNGE("/foo//bar",       "/foo/bar");
    TEST_MUNGE("/foo//bar",       "/foo/bar");
    TEST_MUNGE("/~/foo",          "/~/foo");
    TEST_MUNGE("/~foo/bar",       "/~foo/bar");

    // Relative paths.
    TEST_MUNGE(".",               ".");
    TEST_MUNGE("..",              "..");
    TEST_MUNGE("./..",            "..");
    TEST_MUNGE("./foo",           "foo");
    TEST_MUNGE("foo/.",           "foo");
    TEST_MUNGE("foo/..",          ".");
    TEST_MUNGE("foo/../bar",      "bar");
    TEST_MUNGE("foo/../bar.bas",  "bar.bas");
    TEST_MUNGE("foo/.bar",        "foo/.bar");
    TEST_MUNGE("foo/..bar",       "foo/..bar");
    TEST_MUNGE("foo../bar",       "foo../bar");
    TEST_MUNGE("foo../..",        ".");
    TEST_MUNGE("foo../../bar",    "bar");
    TEST_MUNGE("foo//bar",        "foo/bar");
    TEST_MUNGE("../foo",          "../foo");
    TEST_MUNGE("../../foo",       "../../foo");
    TEST_MUNGE("../../../foo",    "../../../foo");

    // HOME relative paths.
    TEST_MUNGE("~",               home.c_str());
    TEST_MUNGE("~/",              home.c_str());
    TEST_MUNGE("~/.",             home.c_str());
    TEST_MUNGE("~/../foo",        (std::string(home_parent) + "/foo").c_str());
    TEST_MUNGE("~/.mmbasic",      (home + "/.mmbasic").c_str());
#if defined(_WIN32)
    TEST_MUNGE("~/../../tmp",     "C:/tmp");
#else
    TEST_MUNGE("~/../../tmp",     "/tmp");
#endif

    // Test with backslashes.
#if defined(_WIN32)
    TEST_MUNGE("a:\\",            "a:"); // TODO: Shouldn't this have trailing slash?
#else
    TEST_MUNGE("a:\\",            "/");
#endif
    TEST_MUNGE("\\",              "/");
    TEST_MUNGE("\\.",             "/");
    TEST_MUNGE("\\..",            "/");
    TEST_MUNGE("~\\..\\foo",      (std::string(home_parent) + "/foo").c_str());
    TEST_MUNGE("foo\\bar",        "foo/bar");
    TEST_MUNGE("..\\..\\..\\foo", "../../../foo");
}

// Not comprehensive at all.
TEST_F(PathTest, Munge_Fails_GivenNewPathBufferTooSmall) {
    char out[256];

    EXPECT_EQ(kOk, path_munge("/foo/bar", out, 9));
    EXPECT_EQ(kFilenameTooLong, path_munge("/foo/bar", out, 8));

    EXPECT_EQ(kFilenameTooLong, path_munge("~/foo", out, 10));
}

#define EXPECT_PATH_EQ(expected, actual)                          \
    do {                                                          \
        if (!file_compare_path(expected, actual)) {               \
            ADD_FAILURE() << "Path mismatch:\n"                   \
                          << "  Expected: " << (expected) << "\n" \
                          << "  Actual:   " << (actual);          \
        }                                                         \
    } while (0)

// 'path' and 'expected' are std::string
#define TEST_GET_CANONICAL(path, expected)  memset(out, '\0', 256); \
        result = path_get_canonical((path).c_str(), out, 256); \
        EXPECT_EQ(kOk, result); \
        EXPECT_PATH_EQ((expected).c_str(), out)

TEST_F(PathTest, GetCanonical_GivenAbsolutePath) {
    char out[256];
    const std::string test_dir_str = test_dir.string();
    MmResult result;

    // Root path.
    TEST_GET_CANONICAL(std::string("/"), std::string("/"));

    // The parent of root is still root.
    TEST_GET_CANONICAL(std::string("/.."), std::string("/"));

    // Simple (non-existing) absolute path.
    TEST_GET_CANONICAL(test_dir_str + "/foo.bas", test_dir_str + "/foo.bas");

    // Intermediate slash dot dot.
    TEST_GET_CANONICAL(test_dir_str + "/bar/../foo.bas", test_dir_str + "/foo.bas");

    // Intermediate slash dot.
    TEST_GET_CANONICAL(test_dir_str + "/./foo.bas", test_dir_str + "/foo.bas");

    // Multiple slashes.
    TEST_GET_CANONICAL(test_dir_str + "//foo.bas", test_dir_str + "/foo.bas");

    // Trailing dot.
    TEST_GET_CANONICAL(test_dir_str + "/foo.bas.", test_dir_str + "/foo.bas.");

    // Trailing dot dot.
    TEST_GET_CANONICAL(test_dir_str + "/foo.bas..", test_dir_str + "/foo.bas..");

    // Trailing slash dot.
    TEST_GET_CANONICAL(test_dir_str + "/bar/.", test_dir_str + "/bar");

    // Trailing slash dot dot.
    TEST_GET_CANONICAL(test_dir_str + "/bar/..", test_dir_str);
}

TEST_F(PathTest, GetCanonical_GivenRelativePath) {
    char out[256];
    MmResult result;
    std::string expected("");

    // Empty path.
    expected = cwd;
    TEST_GET_CANONICAL(std::string(""), std::string(expected));

    // Simple (non-existing) relative path.
    expected = cwd + "/foo.bas";
    TEST_GET_CANONICAL(std::string("foo.bas"), std::string(expected));

    // Intermediate slash dot dot.
    expected = cwd + "/foo.bas";
    TEST_GET_CANONICAL(std::string("bar/../foo.bas"), std::string(expected));

    // Intermediate slash dot.
    expected = cwd + "/bar/foo.bas";
    TEST_GET_CANONICAL(std::string("bar/./foo.bas"), std::string(expected));

     // Multiple slashes.
    expected = cwd + "/bar/foo.bas";
    TEST_GET_CANONICAL(std::string("bar//foo.bas"), std::string(expected));

    // Trailing dot.
    expected = cwd + "/foo.bas.";
    TEST_GET_CANONICAL(std::string("foo.bas."), std::string(expected));

    // Trailing dot dot.
    expected = cwd + "/foo.bas..";
    TEST_GET_CANONICAL(std::string("foo.bas.."), std::string(expected));

    // Trailing slash dot.
    expected = cwd + "/bar";
    TEST_GET_CANONICAL(std::string("bar/."), std::string(expected));

    // Trailing slash dot dot.
    expected = cwd;
    TEST_GET_CANONICAL(std::string("bar/.."), std::string(expected));
}

TEST_F(PathTest, GetCanonical_GivenTilde) {
    char out[256];
    MmResult result;
    std::string expected("");

    TEST_GET_CANONICAL(std::string("~"), home);
    TEST_GET_CANONICAL(std::string("/~"), std::string("/~"));
    TEST_GET_CANONICAL(std::string("\\~"), std::string("/~"));
    TEST_GET_CANONICAL(std::string("~/"), home);
    TEST_GET_CANONICAL(std::string("~\\"), home);

    expected = cwd + "/~foo";
    TEST_GET_CANONICAL(std::string("~foo"), expected);

    expected = cwd + "/foo~";
    TEST_GET_CANONICAL(std::string("foo~"), expected);
}

TEST_F(PathTest, GetCanonical_GivenDosDrivePrefix) {
    char out[256];
    MmResult result;

#if defined(_WIN32)
    TEST_GET_CANONICAL(std::string("A:"), std::string("A:"));
    TEST_GET_CANONICAL(std::string("A:/"), std::string("A:")); // TODO: Shouldn't this have trailing slash?
    TEST_GET_CANONICAL(std::string("A:\\"), std::string("A:"));  // TODO: Shouldn't this have trailing slash?
    TEST_GET_CANONICAL(std::string("A:/foo"), std::string("A:/foo"));
    TEST_GET_CANONICAL(std::string("A:\\foo"), std::string("A:/foo"));
    TEST_GET_CANONICAL(std::string("C:"), std::string("C:")); // TODO: Shouldn't this have trailing slash?
    TEST_GET_CANONICAL(std::string("C:/"), std::string("C:")); // TODO: Shouldn't this have trailing slash?
    TEST_GET_CANONICAL(std::string("C:\\"), std::string("C:")); // TODO: Shouldn't this have trailing slash?
    TEST_GET_CANONICAL(std::string("C:/foo"), std::string("C:/foo"));
    TEST_GET_CANONICAL(std::string("C:\\foo"), std::string("C:/foo"));
#else
    TEST_GET_CANONICAL(std::string("A:"), std::string("/"));
    TEST_GET_CANONICAL(std::string("A:/"), std::string("/"));
    TEST_GET_CANONICAL(std::string("A:\\"), std::string("/"));
    TEST_GET_CANONICAL(std::string("A:/foo"), std::string("/foo"));
    TEST_GET_CANONICAL(std::string("A:\\foo"), std::string("/foo"));
    TEST_GET_CANONICAL(std::string("C:"), std::string("/"));
    TEST_GET_CANONICAL(std::string("C:/"), std::string("/"));
    TEST_GET_CANONICAL(std::string("C:\\"), std::string("/"));
    TEST_GET_CANONICAL(std::string("C:/foo"), std::string("/foo"));
    TEST_GET_CANONICAL(std::string("C:\\foo"), std::string("/foo"));
#endif
}

TEST_F(PathTest, GetCanonical_ResolvesSymbolicLinks) {
#if defined(_WIN32)
    GTEST_SKIP() << "Symbolic links not supported on this system";
#else
    char out[256];
    const std::string test_dir_str = test_dir.string();
    MmResult result;

    // ${test_dir}/
    //   bar/
    //     foo.bas
    //     foolink.bas             -> ${test_dir}/bar/foo.bas
    //     foolink2.bas            -> ${test_dir}/bar/foolink.bas
    //     missinglink.bas         -> ${test_dir}/bar/missing.bas (which does not exist)
    //     relativelink.bas        -> ../bar/foo.bas
    //   barlink                   -> ${test_dir}/bar
    //   missinglink               -> ${test_dir}/missing
    //   rootlink                  -> /
    //   homelink                  -> ~
    //   wtflink                   -> /..

    ASSERT_EQ(kOk, file_mkdir((test_dir_str + "/bar").c_str()));
    ASSERT_EQ(kOk, file_mkfile((test_dir_str + "/bar/foo.bas").c_str(), ""));
    ASSERT_EQ(kOk, file_mksymlink((test_dir_str + "/bar/foo.bas").c_str(), (test_dir_str + "/bar/foolink.bas").c_str()));
    ASSERT_EQ(kOk, file_mksymlink((test_dir_str + "/bar/foolink.bas").c_str(), (test_dir_str + "/bar/foolink2.bas").c_str()));
    ASSERT_EQ(kOk, file_mksymlink((test_dir_str + "/bar/../bar/foo.bas").c_str(), (test_dir_str + "/bar/relativelink.bas").c_str()));
    ASSERT_EQ(kOk, file_mksymlink((test_dir_str + "/bar/missing.bas").c_str(), (test_dir_str + "/bar/missinglink.bas").c_str()));
    ASSERT_EQ(kOk, file_mksymlink((test_dir_str + "/bar").c_str(), (test_dir_str + "/barlink").c_str()));
    ASSERT_EQ(kOk, file_mksymlink((test_dir_str + "/missing").c_str(), (test_dir_str + "/missinglink").c_str()));
    ASSERT_EQ(kOk, file_mksymlink("/", (test_dir_str + "/rootlink").c_str()));
    ASSERT_EQ(kOk, file_mksymlink(home.c_str(), (test_dir_str + "/homelink").c_str()));
    ASSERT_EQ(kOk, file_mksymlink("/..", (test_dir_str + "/wtflink").c_str()));

    // Absolute paths.
    TEST_GET_CANONICAL(test_dir_str + "/bar/foolink.bas",             test_dir_str + "/bar/foo.bas");
    TEST_GET_CANONICAL(test_dir_str + "/bar/missing.bas",             test_dir_str + "/bar/missing.bas");
    TEST_GET_CANONICAL(test_dir_str + "/bar/missinglink.bas",         test_dir_str + "/bar/missing.bas");
    TEST_GET_CANONICAL(test_dir_str + "/barlink/foolink.bas",         test_dir_str + "/bar/foo.bas");
    TEST_GET_CANONICAL(test_dir_str + "/barlink/foolink2.bas",        test_dir_str + "/bar/foo.bas");
    TEST_GET_CANONICAL(test_dir_str + "/barlink/relativelink.bas",    test_dir_str + "/bar/foo.bas");
    TEST_GET_CANONICAL(test_dir_str + "/barlink/missinglink.bas",     test_dir_str + "/bar/missing.bas");
    TEST_GET_CANONICAL(test_dir_str + "/missing/foolink.bas",         test_dir_str + "/missing/foolink.bas");
    TEST_GET_CANONICAL(test_dir_str + "/missinglink/foolink.bas",     test_dir_str + "/missing/foolink.bas");
    TEST_GET_CANONICAL(test_dir_str + "/missing/missinglink.bas",     test_dir_str + "/missing/missinglink.bas");
    TEST_GET_CANONICAL(test_dir_str + "/missinglink/missinglink.bas", test_dir_str + "/missing/missinglink.bas");
    TEST_GET_CANONICAL(test_dir_str + "/rootlink",                    std::string("/"));
    TEST_GET_CANONICAL(test_dir_str + "/rootlink/foo.bas",            std::string("/foo.bas"));
    TEST_GET_CANONICAL(test_dir_str + "/homelink",                    home);
    TEST_GET_CANONICAL(test_dir_str + "/homelink/foo.bas",            home + "/foo.bas");
    TEST_GET_CANONICAL(test_dir_str + "/wtflink",                     std::string("/"));
    TEST_GET_CANONICAL(test_dir_str + "/wtflink/foo.bas",             std::string("/foo.bas"));

    // Relative paths.
    ASSERT_EQ(kOk, file_mkdir((test_dir / "wombat").string().c_str()));
    ASSERT_EQ(kOk, file_chdir((test_dir / "wombat").string().c_str()));
    TEST_GET_CANONICAL(std::string("../bar/foolink.bas"),             test_dir_str + "/bar/foo.bas");
    TEST_GET_CANONICAL(std::string("../bar/missing.bas"),             test_dir_str + "/bar/missing.bas");
    TEST_GET_CANONICAL(std::string("../bar/missinglink.bas"),         test_dir_str + "/bar/missing.bas");
    TEST_GET_CANONICAL(std::string("../barlink/foolink.bas"),         test_dir_str + "/bar/foo.bas");
    TEST_GET_CANONICAL(std::string("../barlink/foolink2.bas"),        test_dir_str + "/bar/foo.bas");
    TEST_GET_CANONICAL(std::string("../barlink/relativelink.bas"),    test_dir_str + "/bar/foo.bas");
    TEST_GET_CANONICAL(std::string("../barlink/missinglink.bas"),     test_dir_str + "/bar/missing.bas");
    TEST_GET_CANONICAL(std::string("../missing/foolink.bas"),         test_dir_str + "/missing/foolink.bas");
    TEST_GET_CANONICAL(std::string("../missinglink/foolink.bas"),     test_dir_str + "/missing/foolink.bas");
    TEST_GET_CANONICAL(std::string("../missing/missinglink.bas"),     test_dir_str + "/missing/missinglink.bas");
    TEST_GET_CANONICAL(std::string("../missinglink/missinglink.bas"), test_dir_str + "/missing/missinglink.bas");
    TEST_GET_CANONICAL(std::string("../rootlink"),                    std::string("/"));
    TEST_GET_CANONICAL(std::string("../rootlink/foo.bas"),            std::string("/foo.bas"));
    TEST_GET_CANONICAL(std::string("../homelink"),                    std::string(home));
    TEST_GET_CANONICAL(std::string("../homelink/foo.bas"),            std::string(home + "/foo.bas"));
    TEST_GET_CANONICAL(std::string("../wtflink"),                     std::string("/"));
    TEST_GET_CANONICAL(std::string("../wtflink/foo.bas"),             std::string("/foo.bas"));
#endif
}

// Not comprehensive at all.
TEST_F(PathTest, GetCanonical_Fails_GivenDstBufferTooSmall) {
    char out[64] = { 0 };
    memset(out, 'X', sizeof(out) - 1);

    const std::string filename = (test_dir / "barlink" / "foolink.bas").string();
    EXPECT_EQ(kFilenameTooLong, path_get_canonical(filename.c_str(), out, 32));
    const std::string expected = filename.substr(0, 31);
    EXPECT_PATH_EQ(expected.c_str(), out);
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
    {
        std::filesystem::path filename = test_dir / "file_that_exists.txt";
        ASSERT_EQ(kOk, file_mkfile(filename.string().c_str(), ""));
        EXPECT_EQ(true, path_exists(filename.string().c_str()));
    }

    {
        std::filesystem::path filename = test_dir / "file_that_does_not_exist.txt";
        EXPECT_EQ(false, path_exists(filename.string().c_str()));
    }
}

TEST_F(PathTest, IsDirectory) {
#if defined(_WIN32)
    EXPECT_EQ(true, path_is_directory("C:\\"));
    EXPECT_EQ(true, path_is_directory("C:/"));
    EXPECT_EQ(true, path_is_directory("C:"));
#else
    EXPECT_EQ(true, path_is_directory("/"));
    EXPECT_EQ(true, path_is_directory("\\"));
#endif   

    {
        std::filesystem::path dirname = test_dir / "directory_that_exists";
        ASSERT_EQ(kOk, file_mkdir(dirname.string().c_str()));
        EXPECT_EQ(true, path_is_directory(dirname.string().c_str()));
    }

    {
        std::filesystem::path filename = test_dir / "file_that_exists.txt";
        ASSERT_EQ(kOk, file_mkfile(filename.string().c_str(), ""));
        EXPECT_EQ(false, path_is_directory(filename.string().c_str()));
    }

    {
        std::filesystem::path dirname = test_dir / "does_not_exist";
        EXPECT_EQ(false, path_is_directory(dirname.string().c_str()));
    }
}

TEST_F(PathTest, IsEmpty) {
    {
        std::filesystem::path empty_file = test_dir / "empty_file";
        ASSERT_EQ(kOk, file_mkfile(empty_file.string().c_str(), ""));
        EXPECT_EQ(true, path_is_empty(empty_file.string().c_str()));
    }

    {
        std::filesystem::path non_empty_file = test_dir / "non_empty_file";
        ASSERT_EQ(kOk, file_mkfile(non_empty_file.string().c_str(), ""));
        // Write something to the file to make it non-empty.
        FILE *f = fopen(non_empty_file.string().c_str(), "w");
        ASSERT_NE(f, nullptr);
        fputs("Hello, world!", f);
        fclose(f);
        EXPECT_EQ(false, path_is_empty(non_empty_file.string().c_str()));
    }
}

TEST_F(PathTest, IsRegular) {
    {
        std::filesystem::path filename = test_dir / "file.txt";
        ASSERT_EQ(kOk, file_mkfile(filename.string().c_str(), ""));
        EXPECT_EQ(true, path_is_regular(filename.string().c_str()));
    }

    {
        std::filesystem::path dirname = test_dir / "directory";
        ASSERT_EQ(kOk, file_mkdir(dirname.string().c_str()));
        EXPECT_EQ(false, path_is_regular(dirname.string().c_str()));
    }

    {
        std::filesystem::path filename = test_dir / "does_not_exist.txt";
        EXPECT_EQ(false, path_is_regular(filename.string().c_str()));
    }
}

TEST_F(PathTest, HasExtension) {
    EXPECT_EQ(true, path_has_extension("foo.bas", ".bas", false));
    EXPECT_EQ(false, path_has_extension("foo.bas", ".BAS", false));
    EXPECT_EQ(true, path_has_extension("foo.bas", ".BAS", true));
    EXPECT_EQ(false, path_has_extension("foo.bas", ".inc", true));
}

TEST_F(PathTest, HasExtension_GivenMalformedExtension_ReturnsFalse) {
    // Should always return false if you forget the leading period.
    EXPECT_EQ(false, path_has_extension("foo.bas", "bas", false));
    EXPECT_EQ(false, path_has_extension("foo.bas", "BAS", false));
    EXPECT_EQ(false, path_has_extension("foo.bas", "BAS", true));
    EXPECT_EQ(false, path_has_extension("foo.bas", "inc", true));
}

TEST_F(PathTest, MkDir) {
    EXPECT_EQ(kOk, path_mkdir((test_dir / "ab" / "cd" / "ef").string().c_str()));

    EXPECT_TRUE(path_exists((test_dir / "ab").string().c_str()));
    EXPECT_TRUE(path_exists((test_dir / "ab" / "cd").string().c_str()));
    EXPECT_TRUE(path_exists((test_dir / "ab" / "cd" / "ef").string().c_str()));
    EXPECT_TRUE(path_is_directory((test_dir / "ab" / "cd" / "ef").string().c_str()));
}

TEST_F(PathTest, MkDir_GivenExistingDirectory) {
    std::filesystem::path dirname = test_dir / "existing-dir";
    std::filesystem::create_directories(dirname);

    EXPECT_EQ(kOk, path_mkdir(dirname.string().c_str()));
    EXPECT_TRUE(path_exists(dirname.string().c_str()));
    EXPECT_TRUE(path_is_directory(dirname.string().c_str()));
}

TEST_F(PathTest, MkDir_GivenExistingFile) {
    std::filesystem::path filename = test_dir / "existing-file";
    ASSERT_EQ(kOk, file_mkfile(filename.string().c_str(), ""));

    EXPECT_EQ(kFileExists, path_mkdir(filename.string().c_str()));
    EXPECT_EQ(kNotADirectory, path_mkdir((filename / "foo").string().c_str()));
}

// 'input' and 'expected' are std::string
#define TEST_COMPLETE(input, expected) \
        EXPECT_EQ(kOk, path_complete((input).c_str(), out, 256)); \
        EXPECT_STREQ((expected).c_str(), out)

TEST_F(PathTest, Complete_GivenRelativePath) {
    char out[256];

    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo").string().c_str(), ""));
    ASSERT_EQ(kOk, file_mkfile((test_dir / "bar").string().c_str(), ""));
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foobar").string().c_str(), ""));
    ASSERT_EQ(kOk, file_mkfile((test_dir / "sna fu").string().c_str(), ""));
    ASSERT_EQ(kOk, file_chdir(test_dir.string().c_str()));

    TEST_COMPLETE(std::string("b"),        std::string("ar"));
    TEST_COMPLETE(std::string("ba"),       std::string("r"));
    TEST_COMPLETE(std::string("bar"),      std::string(""));
    TEST_COMPLETE(std::string("f"),        std::string("oo"));
    TEST_COMPLETE(std::string("fo"),       std::string("o"));
    TEST_COMPLETE(std::string("foo"),      std::string(""));
    TEST_COMPLETE(std::string("foob"),     std::string("ar"));
    TEST_COMPLETE(std::string("fooba"),    std::string("r"));
    TEST_COMPLETE(std::string("foobar"),   std::string(""));
    TEST_COMPLETE(std::string("s"),        std::string("na fu"));
    TEST_COMPLETE(std::string("sn"),       std::string("a fu"));
    TEST_COMPLETE(std::string("sna"),      std::string(" fu"));
    TEST_COMPLETE(std::string("sna "),     std::string("fu"));
    TEST_COMPLETE(std::string("sna f"),    std::string("u"));
    TEST_COMPLETE(std::string("sna fu"),   std::string(""));
    TEST_COMPLETE(std::string("w"),        std::string(""));
    TEST_COMPLETE(std::string(""),         std::string(""));
    TEST_COMPLETE(std::string("./f"),      std::string("oo"));
    TEST_COMPLETE(std::string("tmp/../f"), std::string("oo"));
}

TEST_F(PathTest, Complete_GivenAbsolutePath) {
    char out[256];
    const std::string test_dir_str = test_dir.string();

    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo").string().c_str(), ""));
    ASSERT_EQ(kOk, file_mkfile((test_dir / "bar").string().c_str(), ""));
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foobar").string().c_str(), ""));
    ASSERT_EQ(kOk, file_mkfile((test_dir / "sna fu").string().c_str(), ""));

    TEST_COMPLETE(test_dir_str + "/b",        std::string("ar"));
    TEST_COMPLETE(test_dir_str + "/ba",       std::string("r"));
    TEST_COMPLETE(test_dir_str + "/bar",      std::string(""));
    TEST_COMPLETE(test_dir_str + "/f",        std::string("oo"));
    TEST_COMPLETE(test_dir_str + "/fo",       std::string("o"));
    TEST_COMPLETE(test_dir_str + "/foo",      std::string(""));
    TEST_COMPLETE(test_dir_str + "/foob",     std::string("ar"));
    TEST_COMPLETE(test_dir_str + "/fooba",    std::string("r"));
    TEST_COMPLETE(test_dir_str + "/foobar",   std::string(""));
    TEST_COMPLETE(test_dir_str + "/s",        std::string("na fu"));
    TEST_COMPLETE(test_dir_str + "/sn",       std::string("a fu"));
    TEST_COMPLETE(test_dir_str + "/sna",      std::string(" fu"));
    TEST_COMPLETE(test_dir_str + "/sna ",     std::string("fu"));
    TEST_COMPLETE(test_dir_str + "/sna f",    std::string("u"));
    TEST_COMPLETE(test_dir_str + "/sna fu",   std::string(""));
    TEST_COMPLETE(test_dir_str + "/w",        std::string(""));
    TEST_COMPLETE(test_dir_str + "",          std::string(""));
    TEST_COMPLETE(test_dir_str + "/./f",      std::string("oo"));
    TEST_COMPLETE(test_dir_str + "/tmp/../f", std::string("oo"));
}

TEST_F(PathTest, Complete_GivenRootPath) {
    char out[256];

    TEST_COMPLETE(std::string("/"),   std::string(""));
#if defined(_WIN32)
    TEST_COMPLETE(std::string("C:/ProgramD"), std::string("ata"));
    TEST_COMPLETE(std::string("C:/ProgramData/Micros"), std::string("oft"));
#else
    TEST_COMPLETE(std::string("/bi"), std::string("n"));
    TEST_COMPLETE(std::string("/me"), std::string("dia"));

    TEST_COMPLETE(std::string("/"),     std::string(""));
    TEST_COMPLETE(std::string("/./bi"), std::string("n"));
    TEST_COMPLETE(std::string("/./me"), std::string("dia"));

    TEST_COMPLETE(std::string("/tmp/.."),    std::string(""));
    TEST_COMPLETE(std::string("/tmp/../bi"), std::string("n"));
    TEST_COMPLETE(std::string("/tmp/../me"), std::string("dia"));
#endif
}

TEST_F(PathTest, Complete_GivenMultipleMatchesWithNoCommonSuffix) {
    char out[256];

    ASSERT_EQ(kOk, file_mkfile((test_dir / "cat").string().c_str(), ""));
    ASSERT_EQ(kOk, file_mkfile((test_dir / "cow").string().c_str(), ""));
    ASSERT_EQ(kOk, file_mkfile((test_dir / "car").string().c_str(), ""));
    ASSERT_EQ(kOk, file_chdir((test_dir).string().c_str()));

    // All three match "c", suffixes are "at", "ow", "ar" - no common prefix.
    // Expected: "" (nothing safe to complete).
    // Bug: returns the suffix of whichever match comes last in directory order.
    TEST_COMPLETE(std::string("c"), std::string(""));

    // Two match "ca": "cat" and "car", suffixes "t" and "r" - no common prefix.
    // Expected: "".
    // Bug: returns "t" or "r" depending on readdir order.
    TEST_COMPLETE(std::string("ca"), std::string(""));
}

TEST_F(PathTest, TryExtension) {
    std::filesystem::path file1 = test_dir / "ResolveWithExtension" / "one.bas";
    std::filesystem::path file2 = test_dir / "ResolveWithExtension" / "two.Bas";
    std::filesystem::path file3 = test_dir / "ResolveWithExtension" / "three.BAS";

    ASSERT_EQ(kOk, file_mkdir((test_dir / "ResolveWithExtension").string().c_str()));
    ASSERT_EQ(kOk, file_mkfile(file1.string().c_str(), ""));
    ASSERT_EQ(kOk, file_mkfile(file2.string().c_str(), ""));
    ASSERT_EQ(kOk, file_mkfile(file3.string().c_str(), ""));

    char f_out[STRINGSIZE];

    std::string filename = file1.string();
    EXPECT_EQ(kOk, path_try_extension(filename.c_str(), ".bas", f_out, STRINGSIZE));
    EXPECT_STREQ(file1.string().c_str(), f_out);
    EXPECT_EQ(kOk, path_try_extension(filename.c_str(), ".BAS", f_out, STRINGSIZE));
    EXPECT_STREQ(file1.string().c_str(), f_out);

    filename = (test_dir / "ResolveWithExtension" / "one").string();
    EXPECT_EQ(kOk, path_try_extension(filename.c_str(), ".bas", f_out, STRINGSIZE));
    EXPECT_STREQ(file1.string().c_str(), f_out);
    EXPECT_EQ(kOk, path_try_extension(filename.c_str(), ".BAS", f_out, STRINGSIZE));
    EXPECT_STREQ(file1.string().c_str(), f_out);

    filename = (test_dir / "ResolveWithExtension" / "two").string();
    EXPECT_EQ(kOk, path_try_extension(filename.c_str(), ".bas", f_out, STRINGSIZE));
#if defined(_WIN32)
    EXPECT_THAT(f_out, testing::StrCaseEq(file2.string().c_str()));
#else
    EXPECT_STREQ(file2.string().c_str(), f_out);
#endif
    EXPECT_EQ(kOk, path_try_extension(filename.c_str(), ".BAS", f_out, STRINGSIZE));
#if defined(_WIN32)
    EXPECT_THAT(f_out, testing::StrCaseEq(file2.string().c_str()));
#else
    EXPECT_STREQ(file2.string().c_str(), f_out);
#endif

    filename = (test_dir / "ResolveWithExtension" / "three").string();
    EXPECT_EQ(kOk, path_try_extension(filename.c_str(), ".bas", f_out, STRINGSIZE));
#if defined(_WIN32)
    EXPECT_THAT(f_out, testing::StrCaseEq(file3.string().c_str()));
#else
    EXPECT_STREQ(file3.string().c_str(), f_out);
#endif
    EXPECT_EQ(kOk, path_try_extension(filename.c_str(), ".BAS", f_out, STRINGSIZE));
#if defined(_WIN32)
    EXPECT_THAT(f_out, testing::StrCaseEq(file3.string().c_str()));
#else
    EXPECT_STREQ(file3.string().c_str(), f_out);
#endif
}

TEST_F(PathTest, TryExtension_GivenInvalidExtension) {
    std::string filename_str = (test_dir / "ResolveWithExtension_GivenInvalidExtension" / "one.bas").string();
    char f_out[STRINGSIZE];

    EXPECT_EQ(kFileInvalidExtension, path_try_extension(filename_str.c_str(), "", f_out, STRINGSIZE));
    EXPECT_EQ(kFileInvalidExtension, path_try_extension(filename_str.c_str(), "BAS", f_out, STRINGSIZE));
}

TEST_F(PathTest, TryExtension_GivenNoMatchingFile) {
    std::string filename_str = (test_dir / "ResolveWithExtension_GivenInvalidExtension" / "one.bas").string();
    char f_out[STRINGSIZE];

    EXPECT_EQ(kFileNotFound, path_try_extension(filename_str.c_str(), ".bas", f_out, STRINGSIZE));
}
