/*
 * Copyright (c) 2021-2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <climits>
#include <gtest/gtest.h>

#include "test_config.h"

extern "C" {

#include "../path.h"
#include "../utility.h"

}

#if defined(__ANDROID__)
#define HOME_PARENT       TERMUX_FILES
#else
#define HOME_PARENT       "/home"
#endif
#define FILE_THAT_EXISTS  BIN_DIR "/cp"
#define PATH_TEST_DIR     TMP_DIR "/PathTest"

class PathTest : public ::testing::Test {

private:

    std::string m_cwd;

protected:

    std::string m_home;

    void SetUp() override {
        struct stat st = { 0 };
        if (stat(PATH_TEST_DIR, &st) == -1) {
            mkdir(PATH_TEST_DIR, 0775);
        }

        // Cache current working directory.
        char *cwd = getcwd(NULL, 0);
        m_cwd = cwd;
        free(cwd);

        char *home = getenv("HOME");
        if (home) {
            m_home = home;
        } else {
            FAIL() << "getenv(\"HOME\") failed.";
        }
    }

    void TearDown() override {
        SYSTEM_CALL("rm -rf " PATH_TEST_DIR);

        // Restore current working directory.
        errno = 0;
        if (FAILED(chdir(m_cwd.c_str()))) {
            FAIL() << "chdir() failed: " << errno;
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

TEST_F(PathTest, Munge) {
    char out[256];
    MmResult result;

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
    TEST_MUNGE("~",               m_home.c_str());
    TEST_MUNGE("~/",              m_home.c_str());
    TEST_MUNGE("~/.",             m_home.c_str());
    TEST_MUNGE("~/../foo",        HOME_PARENT "/foo");
    TEST_MUNGE("~/.mmbasic",      (m_home + "/.mmbasic").c_str());
#if defined(__ANDROID__)
    TEST_MUNGE("~/../../tmp",     TERMUX_ROOT "/tmp");
#else
    TEST_MUNGE("~/../../tmp",     TMP_DIR);
#endif

    // Test with backslashes.
    TEST_MUNGE("a:\\",            "/");
    TEST_MUNGE("\\",              "/");
    TEST_MUNGE("\\.",             "/");
    TEST_MUNGE("\\..",            "/");
    TEST_MUNGE("~\\..\\foo",      HOME_PARENT "/foo");
    TEST_MUNGE("foo\\bar",        "foo/bar");
    TEST_MUNGE("..\\..\\..\\foo", "../../../foo");
}

#define TEST_GET_CANONICAL(path, expected)  memset(out, '\0', 256); \
        result = path_get_canonical(path, out, 256); \
        EXPECT_EQ(kOk, result); \
        EXPECT_STREQ(expected, out)

TEST_F(PathTest, GetCanonical_GivenAbsolutePath) {
    char out[256];
    MmResult result;

    // Root path.
    TEST_GET_CANONICAL("/", "/");

    // The parent of root is still root.
    TEST_GET_CANONICAL("/..", "/");

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
    MmResult result;
    char expected[PATH_MAX + 256];

    char cwd[PATH_MAX];
    ASSERT_TRUE(getcwd(cwd, sizeof(cwd))) << "getcwd() returned NULL";

    // Empty path.
    sprintf(expected, "%s", cwd);
    TEST_GET_CANONICAL("", expected);

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

TEST_F(PathTest, GetCanonical_GivenTilde) {
    char out[256];
    MmResult result;
    char expected[PATH_MAX + 256];

    char cwd[PATH_MAX];
    ASSERT_TRUE(getcwd(cwd, sizeof(cwd))) << "getcwd() returned NULL";

    const char *home = getenv("HOME");
    ASSERT_TRUE(home) << "getenv(\"HOME\") returned NULL";

    TEST_GET_CANONICAL("~", home);
    TEST_GET_CANONICAL("/~", "/~");
    TEST_GET_CANONICAL("\\~", "/~");
    TEST_GET_CANONICAL("~/", home);
    TEST_GET_CANONICAL("~\\", home);

    sprintf(expected, "%s/~foo", cwd);
    TEST_GET_CANONICAL("~foo", expected);

    sprintf(expected, "%s/foo~", cwd);
    TEST_GET_CANONICAL("foo~", expected);
}

TEST_F(PathTest, GetCanonical_GivenDosDrivePrefix) {
    char out[256];
    MmResult result;

    TEST_GET_CANONICAL("A:", "/");
    TEST_GET_CANONICAL("A:/", "/");
    TEST_GET_CANONICAL("A:\\", "/");
    TEST_GET_CANONICAL("A:/foo", "/foo");
    TEST_GET_CANONICAL("A:\\foo", "/foo");
    TEST_GET_CANONICAL("C:", "/");
    TEST_GET_CANONICAL("C:/", "/");
    TEST_GET_CANONICAL("C:\\", "/");
    TEST_GET_CANONICAL("C:/foo", "/foo");
    TEST_GET_CANONICAL("C:\\foo", "/foo");
}

TEST_F(PathTest, GetCanonical_ResolvesSymbolicLinks) {
    char out[256];
    MmResult result;

    // ${PATH_TEST_DIR}/
    //   bar/
    //     foo.bas
    //     foolink.bas             -> ${PATH_TEST_DIR}/bar/foo.bas
    //     foolink2.bas            -> ${PATH_TEST_DIR}/bar/foolink.bas
    //     missinglink.bas         -> ${PATH_TEST_DIR}/bar/missing.bas (which does not exist)
    //     relativelink.bas        -> ../bar/foo.bas
    //   barlink                   -> ${PATH_TEST_DIR}/bar
    //   missinglink               -> ${PATH_TEST_DIR}/missing
    //   rootlink                  -> /
    //   homelink                  -> ~
    //   wtflink                   -> /..

    SYSTEM_CALL("mkdir " PATH_TEST_DIR "/bar");
    SYSTEM_CALL("touch " PATH_TEST_DIR "/bar/foo.bas");
    SYSTEM_CALL("ln -s " PATH_TEST_DIR "/bar/foo.bas "      PATH_TEST_DIR "/bar/foolink.bas");
    SYSTEM_CALL("ln -s " PATH_TEST_DIR "/bar/foolink.bas "  PATH_TEST_DIR "/bar/foolink2.bas");
    SYSTEM_CALL("ln -s " "../bar/foo.bas "                  PATH_TEST_DIR "/bar/relativelink.bas");
    SYSTEM_CALL("ln -s " PATH_TEST_DIR "/bar/missing.bas "  PATH_TEST_DIR "/bar/missinglink.bas");
    SYSTEM_CALL("ln -s " PATH_TEST_DIR "/bar "              PATH_TEST_DIR "/barlink");
    SYSTEM_CALL("ln -s " PATH_TEST_DIR "/missing "          PATH_TEST_DIR "/missinglink");
    SYSTEM_CALL("ln -s / "                                  PATH_TEST_DIR "/rootlink");
    SYSTEM_CALL("ln -s ~ "                                  PATH_TEST_DIR "/homelink");
    SYSTEM_CALL("ln -s /.. "                                PATH_TEST_DIR "/wtflink");

    // Absolute paths.
    TEST_GET_CANONICAL(PATH_TEST_DIR "/bar/foolink.bas",             PATH_TEST_DIR "/bar/foo.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/bar/missing.bas",             PATH_TEST_DIR "/bar/missing.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/bar/missinglink.bas",         PATH_TEST_DIR "/bar/missing.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/barlink/foolink.bas",         PATH_TEST_DIR "/bar/foo.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/barlink/foolink2.bas",        PATH_TEST_DIR "/bar/foo.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/barlink/relativelink.bas",    PATH_TEST_DIR "/bar/foo.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/barlink/missinglink.bas",     PATH_TEST_DIR "/bar/missing.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/missing/foolink.bas",         PATH_TEST_DIR "/missing/foolink.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/missinglink/foolink.bas",     PATH_TEST_DIR "/missing/foolink.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/missing/missinglink.bas",     PATH_TEST_DIR "/missing/missinglink.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/missinglink/missinglink.bas", PATH_TEST_DIR "/missing/missinglink.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/rootlink",                    "/");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/rootlink/foo.bas",            "/foo.bas");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/homelink",                    m_home.c_str());
    TEST_GET_CANONICAL(PATH_TEST_DIR "/homelink/foo.bas",            (m_home + "/foo.bas").c_str());
    TEST_GET_CANONICAL(PATH_TEST_DIR "/wtflink",                     "/");
    TEST_GET_CANONICAL(PATH_TEST_DIR "/wtflink/foo.bas",             "/foo.bas");

    // Relative paths.
    SYSTEM_CALL("mkdir " PATH_TEST_DIR "/wombat");
    ASSERT_TRUE(SUCCEEDED(chdir(PATH_TEST_DIR "/wombat"))) << "chdir() failed";
    TEST_GET_CANONICAL("../bar/foolink.bas",             PATH_TEST_DIR "/bar/foo.bas");
    TEST_GET_CANONICAL("../bar/missing.bas",             PATH_TEST_DIR "/bar/missing.bas");
    TEST_GET_CANONICAL("../bar/missinglink.bas",         PATH_TEST_DIR "/bar/missing.bas");
    TEST_GET_CANONICAL("../barlink/foolink.bas",         PATH_TEST_DIR "/bar/foo.bas");
    TEST_GET_CANONICAL("../barlink/foolink2.bas",        PATH_TEST_DIR "/bar/foo.bas");
    TEST_GET_CANONICAL("../barlink/relativelink.bas",    PATH_TEST_DIR "/bar/foo.bas");
    TEST_GET_CANONICAL("../barlink/missinglink.bas",     PATH_TEST_DIR "/bar/missing.bas");
    TEST_GET_CANONICAL("../missing/foolink.bas",         PATH_TEST_DIR "/missing/foolink.bas");
    TEST_GET_CANONICAL("../missinglink/foolink.bas",     PATH_TEST_DIR "/missing/foolink.bas");
    TEST_GET_CANONICAL("../missing/missinglink.bas",     PATH_TEST_DIR "/missing/missinglink.bas");
    TEST_GET_CANONICAL("../missinglink/missinglink.bas", PATH_TEST_DIR "/missing/missinglink.bas");
    TEST_GET_CANONICAL("../rootlink",                    "/");
    TEST_GET_CANONICAL("../rootlink/foo.bas",            "/foo.bas");
    TEST_GET_CANONICAL("../homelink",                    m_home.c_str());
    TEST_GET_CANONICAL("../homelink/foo.bas",            (m_home + "/foo.bas").c_str());
    TEST_GET_CANONICAL("../wtflink",                     "/");
    TEST_GET_CANONICAL("../wtflink/foo.bas",             "/foo.bas");
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

    char filename[] = TMP_DIR "/is_empty_XXXXXX";
    int fd = mkstemp(filename);
    close(fd);
    EXPECT_EQ(path_exists(filename), true);
    EXPECT_EQ(path_is_empty(filename), true);
}

TEST_F(PathTest, IsRegular) {
    EXPECT_EQ(path_is_regular(FILE_THAT_EXISTS), true);
    EXPECT_EQ(path_is_regular("/bin"), false);
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
    EXPECT_EQ(kOk, path_mkdir(PATH_TEST_DIR "/ab/cd/ef"));

    EXPECT_TRUE(path_exists(PATH_TEST_DIR));
    EXPECT_TRUE(path_exists(PATH_TEST_DIR "/ab"));
    EXPECT_TRUE(path_exists(PATH_TEST_DIR "/ab/cd"));
    EXPECT_TRUE(path_exists(PATH_TEST_DIR "/ab/cd/ef"));
    EXPECT_TRUE(path_is_directory(PATH_TEST_DIR "/ab/cd/ef"));
}

TEST_F(PathTest, MkDir_GivenExistingDirectory) {
    SYSTEM_CALL("mkdir " PATH_TEST_DIR "/existing-dir");

    EXPECT_EQ(kOk, path_mkdir(PATH_TEST_DIR "/existing-dir"));

    EXPECT_TRUE(path_exists(PATH_TEST_DIR "/existing-dir"));
    EXPECT_TRUE(path_is_directory(PATH_TEST_DIR "/existing-dir"));
}

TEST_F(PathTest, MkDir_GivenExistingFile) {
    SYSTEM_CALL("touch " PATH_TEST_DIR "/existing-file");

    EXPECT_EQ(kFileExists, path_mkdir(PATH_TEST_DIR "/existing-file"));
    EXPECT_EQ(kNotADirectory, path_mkdir(PATH_TEST_DIR "/existing-file/foo"));
}

TEST_F(PathTest, Append) {
    char out[32];

    EXPECT_EQ(kOk, path_append("foo", "bar", out, 32));
    EXPECT_STREQ("foo/bar", out);

    EXPECT_EQ(kOk, path_append("", "bar", out, 32));
    EXPECT_STREQ("/bar", out);

    EXPECT_EQ(kOk, path_append("foo", "", out, 32));
    EXPECT_STREQ("foo/", out);

    EXPECT_EQ(kOk, path_append("", "", out, 32));
    EXPECT_STREQ("/", out);

    EXPECT_EQ(kOk, path_append("0123456789012345", "01234567890123", out, 32));
    EXPECT_STREQ("0123456789012345/01234567890123", out);

    EXPECT_EQ(kFilenameTooLong, path_append("0123456789012345", "012345678901234", out, 32));
}

#define TEST_COMPLETE(input, expected) \
        EXPECT_EQ(kOk, path_complete(input, out, 256)); \
        EXPECT_STREQ(expected, out)

TEST_F(PathTest, GivenRelativePath) {
    char out[256];

    MAKE_FILE(PATH_TEST_DIR "/foo");
    MAKE_FILE(PATH_TEST_DIR "/bar");
    MAKE_FILE(PATH_TEST_DIR "/foobar");
    MAKE_FILE("\"" PATH_TEST_DIR "/sna fu\"");
    CHDIR(PATH_TEST_DIR);

    TEST_COMPLETE("b",        "ar");
    TEST_COMPLETE("ba",       "r");
    TEST_COMPLETE("bar",      "");
    TEST_COMPLETE("f",        "oo");
    TEST_COMPLETE("fo",       "o");
    TEST_COMPLETE("foo",      "");
    TEST_COMPLETE("foob",     "ar");
    TEST_COMPLETE("fooba",    "r");
    TEST_COMPLETE("foobar",   "");
    TEST_COMPLETE("s",        "na fu");
    TEST_COMPLETE("sn",       "a fu");
    TEST_COMPLETE("sna",      " fu");
    TEST_COMPLETE("sna ",     "fu");
    TEST_COMPLETE("sna f",    "u");
    TEST_COMPLETE("sna fu",   "");
    TEST_COMPLETE("w",        "");
    TEST_COMPLETE("",         "");
    TEST_COMPLETE("./f",      "oo");
    TEST_COMPLETE("tmp/../f", "oo");
}

TEST_F(PathTest, Complete_GivenAbsolutePath) {
    char out[256];

    MAKE_FILE(PATH_TEST_DIR "/foo");
    MAKE_FILE(PATH_TEST_DIR "/bar");
    MAKE_FILE(PATH_TEST_DIR "/foobar");
    MAKE_FILE("\"" PATH_TEST_DIR "/sna fu\"");

    TEST_COMPLETE(PATH_TEST_DIR "/b",        "ar");
    TEST_COMPLETE(PATH_TEST_DIR "/ba",       "r");
    TEST_COMPLETE(PATH_TEST_DIR "/bar",      "");
    TEST_COMPLETE(PATH_TEST_DIR "/f",        "oo");
    TEST_COMPLETE(PATH_TEST_DIR "/fo",       "o");
    TEST_COMPLETE(PATH_TEST_DIR "/foo",      "");
    TEST_COMPLETE(PATH_TEST_DIR "/foob",     "ar");
    TEST_COMPLETE(PATH_TEST_DIR "/fooba",    "r");
    TEST_COMPLETE(PATH_TEST_DIR "/foobar",   "");
    TEST_COMPLETE(PATH_TEST_DIR "/s",        "na fu");
    TEST_COMPLETE(PATH_TEST_DIR "/sn",       "a fu");
    TEST_COMPLETE(PATH_TEST_DIR "/sna",      " fu");
    TEST_COMPLETE(PATH_TEST_DIR "/sna ",     "fu");
    TEST_COMPLETE(PATH_TEST_DIR "/sna f",    "u");
    TEST_COMPLETE(PATH_TEST_DIR "/sna fu",   "");
    TEST_COMPLETE(PATH_TEST_DIR "/w",        "");
    TEST_COMPLETE(PATH_TEST_DIR "",          "");
    TEST_COMPLETE(PATH_TEST_DIR "/./f",      "oo");
    TEST_COMPLETE(PATH_TEST_DIR "/tmp/../f", "oo");
}

TEST_F(PathTest, Complete_GivenRootPath) {
    char out[256];

    TEST_COMPLETE("/",   "");
    TEST_COMPLETE("/bi", "n");
    TEST_COMPLETE("/me", "dia");

    TEST_COMPLETE("/",     "");
    TEST_COMPLETE("/./bi", "n");
    TEST_COMPLETE("/./me", "dia");

    TEST_COMPLETE("/tmp/..",    "");
    TEST_COMPLETE("/tmp/../bi", "n");
    TEST_COMPLETE("/tmp/../me", "dia");
}


TEST_F(PathTest, TryExtension) {
    #define FILE_ONE    PATH_TEST_DIR "/ResolveWithExtension/one.bas"
    #define FILE_TWO    PATH_TEST_DIR "/ResolveWithExtension/two.Bas"
    #define FILE_THREE  PATH_TEST_DIR "/ResolveWithExtension/three.BAS"

    SYSTEM_CALL("mkdir " PATH_TEST_DIR "/ResolveWithExtension");
    SYSTEM_CALL("touch " FILE_ONE);
    SYSTEM_CALL("touch " FILE_TWO);
    SYSTEM_CALL("touch " FILE_THREE);
    char f_out[STRINGSIZE];

    const char *filename = FILE_ONE;
    EXPECT_EQ(kOk, path_try_extension(filename, ".bas", f_out, STRINGSIZE));
    EXPECT_STREQ(FILE_ONE, f_out);
    EXPECT_EQ(kOk, path_try_extension(filename, ".BAS", f_out, STRINGSIZE));
    EXPECT_STREQ(FILE_ONE, f_out);

    filename = PATH_TEST_DIR "/ResolveWithExtension/one";
    EXPECT_EQ(kOk, path_try_extension(filename, ".bas", f_out, STRINGSIZE));
    EXPECT_STREQ(FILE_ONE, f_out);
    EXPECT_EQ(kOk, path_try_extension(filename, ".BAS", f_out, STRINGSIZE));
    EXPECT_STREQ(FILE_ONE, f_out);

    filename = PATH_TEST_DIR "/ResolveWithExtension/two";
    EXPECT_EQ(kOk, path_try_extension(filename, ".bas", f_out, STRINGSIZE));
    EXPECT_STREQ(FILE_TWO, f_out);
    EXPECT_EQ(kOk, path_try_extension(filename, ".BAS", f_out, STRINGSIZE));
    EXPECT_STREQ(FILE_TWO, f_out);

    filename = PATH_TEST_DIR "/ResolveWithExtension/three";
    EXPECT_EQ(kOk, path_try_extension(filename, ".bas", f_out, STRINGSIZE));
    EXPECT_STREQ(FILE_THREE, f_out);
    EXPECT_EQ(kOk, path_try_extension(filename, ".BAS", f_out, STRINGSIZE));
    EXPECT_STREQ(FILE_THREE, f_out);

    #undef FILE_ONE
    #undef FILE_TWO
    #undef FILE_THREE
}

TEST_F(PathTest, TryExtension_GivenInvalidExtension) {
    #define FILE_ONE  PATH_TEST_DIR "/ResolveWithExtension_GivenInvalidExtension/one.bas"

    const char *filename = FILE_ONE;
    char f_out[STRINGSIZE];

    EXPECT_EQ(kFileInvalidExtension, path_try_extension(filename, "", f_out, STRINGSIZE));
    EXPECT_EQ(kFileInvalidExtension, path_try_extension(filename, "BAS", f_out, STRINGSIZE));

    #undef FILE_ONE
}

TEST_F(PathTest, TryExtension_GivenNoMatchingFile) {
        #define FILE_ONE  PATH_TEST_DIR "/ResolveWithExtension_GivenInvalidExtension/one.bas"

    const char *filename = FILE_ONE;
    char f_out[STRINGSIZE];

    EXPECT_EQ(kFileNotFound, path_try_extension(filename, ".bas", f_out, STRINGSIZE));

    #undef FILE_ONE
}
