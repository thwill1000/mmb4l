/*
 * Copyright (c) 2021-2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <climits>
#include <dirent.h>

#include "test_config.h"

extern "C" {

#include "../options.h"
#include "../program.h"
#include "../utility.h"

char *CFunctionFlash = NULL;
char cmdCSUB = '\0';
char *CurrentLinePtr = NULL;
char inpbuf[STRINGSIZE] = { '\0' };
char ProgMemory[PROG_FLASH_SIZE] = { '\0' } ;
char tknbuf[STRINGSIZE] = { '\0' };
Options mmb_options = { 0 };

const char *checkstring(const char *p, char *tkn) { return NULL; }
void ClearProgram(void) { }
void ClearSpecificTempMemory(void *addr) { }
void console_set_title(const char *title) { }
int error_check(void) { return 0; }
void error_throw(MmResult error) { }
void error_throw_ex(MmResult error, char *msg, ...) { }
void file_close(int fnbr) { }
int file_eof(int fnbr) { return 0; }
int file_find_free(void) { return 1; }
void file_open(char *fname, char *mode, int fnbr) { }
int GetCommandValue(char *n) { return 0; }
char *getCstring(char *p) { return NULL; }
void *GetTempMemory(int NbrBytes) { return NULL; }
void IntToStr(char *strr, long long int nbr, unsigned int base) { }
void ListNewLine(int *ListCnt, int all) { }
void makeargs(char **p, int maxargs, char *argbuf, char *argv[], int *argc, char *delim) { }
void MMgetline(int filenbr, char *p) { }
void MMPrintString(char* s) { }
void tokenise(int console) { }

}

#define PROGRAM_TEST_DIR  TMP_DIR "/ProgramTest"

class ProgramTest : public ::testing::Test {

protected:

    void MakeDir(const char* path) {
        struct stat st = { 0 };
        if (stat(path, &st) == -1) {
            mkdir(path, 0775);
        }
    }

    void MakeEmptyFile(const char* path) {
        FILE *f = fopen(path, "w");
        fclose(f);
    }

    std::string PathToFileInCwd(const char* path) {
        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) {
            perror("getcwd() error");
            return "";
        }
        return std::string(cwd) + "/" + path;
    }

    void RemoveRecursively(const char *dir_path) {
        struct stat st = { 0 };
        if (stat(dir_path, &st) == -1) return; // Does not exist.

        DIR *dir = opendir(dir_path);
        struct dirent *next_file;
        char file_path[PATH_MAX];

        while ((next_file = readdir(dir)) != NULL) {
            if (strcmp(next_file->d_name, ".") != 0
                    && strcmp(next_file->d_name, "..") != 0) {
                sprintf(file_path, "%s/%s", dir_path, next_file->d_name);
                if (next_file->d_type == DT_DIR) RemoveRecursively(file_path);
                remove(file_path);
            }
        }

        closedir(dir);
    }

    void RemoveDir(const char *path) {
        RemoveFile(path);
    }

    void RemoveFile(const char *path) {
        if (FAILED(remove(path)) && errno != ENOENT) {
            char buf[PATH_MAX];
            sprintf(buf, "remove(\"%s\") failed", path);
            perror(buf);
        }
    }

protected:

    void RemoveTemporaryFiles() {
        RemoveRecursively(PROGRAM_TEST_DIR);
        RemoveDir("bar");
        RemoveFile("foo");
        RemoveFile("foo.bas");
        RemoveFile("foo.BAS");
        RemoveFile("foo.Bas");
    }

    void SetUp() override {
        RemoveTemporaryFiles();
        MakeDir(PROGRAM_TEST_DIR);
        MakeDir(PROGRAM_TEST_DIR "/bar");
        MakeDir("bar");

        errno = 0;
        CurrentLinePtr = (char *) 0;
        strcpy(CurrentFile, "");
    }

    void TearDown() override {
        RemoveTemporaryFiles();
    }
};

#define TEST_PROGRAM_GET_BAS_FILE(filename, expected) \
    result = program_get_bas_file(filename, out); \
    EXPECT_STREQ(expected, result); \
    EXPECT_STREQ(expected, out); \
    EXPECT_EQ(0, errno)

TEST_F(ProgramTest, GetBasFile_GivenAbsolutePath) {
    char out[STRINGSIZE] = { '\0' };
    char *result;

    // Test when no file present.
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.bas", PROGRAM_TEST_DIR "/foo.bas");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.BAS", PROGRAM_TEST_DIR "/foo.BAS");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.Bas", PROGRAM_TEST_DIR "/foo.Bas");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo",     PROGRAM_TEST_DIR "/foo.bas");

    // Test when .Bas file present.
    MakeEmptyFile(PROGRAM_TEST_DIR "/foo.Bas");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.bas", PROGRAM_TEST_DIR "/foo.bas");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.BAS", PROGRAM_TEST_DIR "/foo.BAS");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.Bas", PROGRAM_TEST_DIR "/foo.Bas");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo",     PROGRAM_TEST_DIR "/foo.Bas");

    // Test when .BAS and .Bas files present.
    MakeEmptyFile(PROGRAM_TEST_DIR "/foo.BAS");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.bas", PROGRAM_TEST_DIR "/foo.bas");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.BAS", PROGRAM_TEST_DIR "/foo.BAS");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.Bas", PROGRAM_TEST_DIR "/foo.Bas");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo",     PROGRAM_TEST_DIR "/foo.BAS");

    // Test when .bas, BAS and .Bas files present.
    MakeEmptyFile(PROGRAM_TEST_DIR "/foo.bas");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.bas", PROGRAM_TEST_DIR "/foo.bas");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.BAS", PROGRAM_TEST_DIR "/foo.BAS");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo.Bas", PROGRAM_TEST_DIR "/foo.Bas");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo",     PROGRAM_TEST_DIR "/foo.bas");

    // Test when "foo" without extension is present.
    MakeEmptyFile("foo");
    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/foo",     PROGRAM_TEST_DIR "/foo.bas");
}

TEST_F(ProgramTest, GetBasFile_GivenRelativePath) {
    char out[STRINGSIZE] = { '\0' };
    char *result;

    // Test when no file present.
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PathToFileInCwd("foo.bas").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", PathToFileInCwd("foo.BAS").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.Bas", PathToFileInCwd("foo.Bas").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo",     PathToFileInCwd("foo.bas").c_str());

    // Test when .Bas file present.
    MakeEmptyFile("foo.Bas");
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PathToFileInCwd("foo.bas").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", PathToFileInCwd("foo.BAS").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.Bas", PathToFileInCwd("foo.Bas").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo",     PathToFileInCwd("foo.Bas").c_str());

    // Test when .BAS and .Bas files present.
    MakeEmptyFile("foo.BAS");
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PathToFileInCwd("foo.bas").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", PathToFileInCwd("foo.BAS").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.Bas", PathToFileInCwd("foo.Bas").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo",     PathToFileInCwd("foo.BAS").c_str());

    // Test when .bas, BAS and .Bas files present.
    MakeEmptyFile("foo.bas");
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PathToFileInCwd("foo.bas").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", PathToFileInCwd("foo.BAS").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.Bas", PathToFileInCwd("foo.Bas").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo",     PathToFileInCwd("foo.bas").c_str());

    // Test when "foo" without extension is present.
    MakeEmptyFile("foo");
    TEST_PROGRAM_GET_BAS_FILE("foo",     PathToFileInCwd("foo.bas").c_str());
}

TEST_F(ProgramTest, GetBasFile_GivenRunningProgram_AndAbsolutePath) {
    char out[STRINGSIZE] = { '\0' };
    errno = 0;
    CurrentLinePtr = (char *) 1; // anything other than 0.
    strcpy(CurrentFile, PROGRAM_TEST_DIR "/current.bas");
    char *result;

    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/bar/foo.bas", PROGRAM_TEST_DIR "/bar/foo.bas");
}

TEST_F(ProgramTest, GetBasFile_GivenRunningProgram_AndRelativePath) {
    char out[STRINGSIZE] = { '\0' };
    errno = 0;
    CurrentLinePtr = (char *) 1; // anything other than 0.
    strcpy(CurrentFile, PROGRAM_TEST_DIR "/current.bas");
    char *result;

    // Contrary to my original belief the file should be is resolved relative
    // to CWD and not to the directory containing the currently running program.
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PathToFileInCwd("foo.bas").c_str());
}

TEST_F(ProgramTest, GetBasFile_GivenOnlyInSearchPath) {
    char out[STRINGSIZE] = { '\0' };
    char *result;
    MakeEmptyFile(PROGRAM_TEST_DIR "/foo.bas");

    // Given no search path, expect to resolve to file in CWD.
    strcpy(mmb_options.search_path, "");
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PathToFileInCwd("foo.bas").c_str());

    // Given search path contains file, expect to resolve to file in search path.
    strcpy(mmb_options.search_path, PROGRAM_TEST_DIR);
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PROGRAM_TEST_DIR "/foo.bas");

    // Given extension is not an exact match, expect to resolve to file in CWD.
    strcpy(mmb_options.search_path, PROGRAM_TEST_DIR);
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", PathToFileInCwd("foo.BAS").c_str());

    // Given no extension, expect to resolve to file in search path.
    strcpy(mmb_options.search_path, PROGRAM_TEST_DIR);
    TEST_PROGRAM_GET_BAS_FILE("foo", PROGRAM_TEST_DIR "/foo.bas");

    // Given file in CWD and search path, expect to resolve to file in CWD.
    MakeEmptyFile("foo.bas");
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PathToFileInCwd("foo.bas").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", PathToFileInCwd("foo.BAS").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo",     PathToFileInCwd("foo.bas").c_str());
}

#define TEST_PROGRAM_GET_INC_FILE(filename, expected) \
    result = program_get_inc_file(bas_file, filename, out); \
    EXPECT_STREQ(expected, result); \
    EXPECT_STREQ(expected, out); \
    EXPECT_EQ(0, errno)

TEST_F(ProgramTest, GetIncFile_GivenAbsolutePath) {
    const char *bas_file = PROGRAM_TEST_DIR "/bar/myprog.bas";
    char out[STRINGSIZE] = { '\0' };
    char *result;

    // Test when no file present.
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.inc", PROGRAM_TEST_DIR "/foo.inc");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.INC", PROGRAM_TEST_DIR "/foo.INC");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.Inc", PROGRAM_TEST_DIR "/foo.Inc");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo",     PROGRAM_TEST_DIR "/foo.inc");

    // Test when .Inc file present.
    MakeEmptyFile(PROGRAM_TEST_DIR "/foo.Inc");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.inc", PROGRAM_TEST_DIR "/foo.inc");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.INC", PROGRAM_TEST_DIR "/foo.INC");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.Inc", PROGRAM_TEST_DIR "/foo.Inc");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo",     PROGRAM_TEST_DIR "/foo.Inc");

    // Test when .INC and .inc files present.
    MakeEmptyFile(PROGRAM_TEST_DIR "/foo.INC");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.inc", PROGRAM_TEST_DIR "/foo.inc");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.INC", PROGRAM_TEST_DIR "/foo.INC");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.Inc", PROGRAM_TEST_DIR "/foo.Inc");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo",     PROGRAM_TEST_DIR "/foo.INC");

    // Test when .inc, .INC and .Inc files present.
    MakeEmptyFile(PROGRAM_TEST_DIR "/foo.inc");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.inc", PROGRAM_TEST_DIR "/foo.inc");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.INC", PROGRAM_TEST_DIR "/foo.INC");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo.Inc", PROGRAM_TEST_DIR "/foo.Inc");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo",     PROGRAM_TEST_DIR "/foo.inc");

    // Test when "foo" without extension is present.
    MakeEmptyFile("foo");
    TEST_PROGRAM_GET_INC_FILE(PROGRAM_TEST_DIR "/foo",     PROGRAM_TEST_DIR "/foo.inc");
}

TEST_F(ProgramTest, GetIncFile_GivenRelativePath) {
    const char *bas_file = PROGRAM_TEST_DIR "/bar/myprog.bas";
    char out[STRINGSIZE] = { '\0' };
    char *result;

    // Test when no file present.
    TEST_PROGRAM_GET_INC_FILE("foo.inc", PROGRAM_TEST_DIR "/bar/foo.inc");
    TEST_PROGRAM_GET_INC_FILE("foo.INC", PROGRAM_TEST_DIR "/bar/foo.INC");
    TEST_PROGRAM_GET_INC_FILE("foo.Inc", PROGRAM_TEST_DIR "/bar/foo.Inc");
    TEST_PROGRAM_GET_INC_FILE("foo",     PROGRAM_TEST_DIR "/bar/foo.inc");

    // Test when .Inc file present.
    MakeEmptyFile(PROGRAM_TEST_DIR "/bar/foo.Inc");
    TEST_PROGRAM_GET_INC_FILE("foo.inc", PROGRAM_TEST_DIR "/bar/foo.inc");
    TEST_PROGRAM_GET_INC_FILE("foo.INC", PROGRAM_TEST_DIR "/bar/foo.INC");
    TEST_PROGRAM_GET_INC_FILE("foo.Inc", PROGRAM_TEST_DIR "/bar/foo.Inc");
    TEST_PROGRAM_GET_INC_FILE("foo",     PROGRAM_TEST_DIR "/bar/foo.Inc");

    // Test when .INC and .inc files present.
    MakeEmptyFile(PROGRAM_TEST_DIR "/bar/foo.INC");
    TEST_PROGRAM_GET_INC_FILE("foo.inc", PROGRAM_TEST_DIR "/bar/foo.inc");
    TEST_PROGRAM_GET_INC_FILE("foo.INC", PROGRAM_TEST_DIR "/bar/foo.INC");
    TEST_PROGRAM_GET_INC_FILE("foo.Inc", PROGRAM_TEST_DIR "/bar/foo.Inc");
    TEST_PROGRAM_GET_INC_FILE("foo",     PROGRAM_TEST_DIR "/bar/foo.INC");

    // Test when .inc, .INC and .Inc files present.
    MakeEmptyFile(PROGRAM_TEST_DIR "/bar/foo.inc");
    TEST_PROGRAM_GET_INC_FILE("foo.inc", PROGRAM_TEST_DIR "/bar/foo.inc");
    TEST_PROGRAM_GET_INC_FILE("foo.INC", PROGRAM_TEST_DIR "/bar/foo.INC");
    TEST_PROGRAM_GET_INC_FILE("foo.Inc", PROGRAM_TEST_DIR "/bar/foo.Inc");
    TEST_PROGRAM_GET_INC_FILE("foo",     PROGRAM_TEST_DIR "/bar/foo.inc");

    // Test when "foo" without extension is present.
    MakeEmptyFile(PROGRAM_TEST_DIR "/bar/foo");
    TEST_PROGRAM_GET_INC_FILE("foo",     PROGRAM_TEST_DIR "/bar/foo.inc");
}
