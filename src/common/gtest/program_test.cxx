/*
 * Copyright (c) 2021-2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <climits>
#include <dirent.h>

#include "test_config.h"

extern "C" {

#include "../error.h"
#include "../memory.h"
#include "../options.h"
#include "../program.h"
#include "../utility.h"
#include "../../core/Commands.h"
#include "../../core/commandtbl.h"
#include "../../core/MMBasic.h"
#include "../../core/gtest/command_stubs.h"
#include "../../core/gtest/function_stubs.h"
#include "../../core/gtest/operation_stubs.h"

// Defined in "main.c"
char *CFunctionFlash;
char *CFunctionLibrary;
char **FontTable;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Options mmb_options;
ErrorState mmb_normal_error_state;
int MMgetchar(void) { return 0; }

// Defined in "commands/cmd_read.c"
void cmd_read_clear_cache()  { }

// Defined in "common/console.c"
int console_kbhit(void) { return 0; }
char console_putc(char c) { return c; }
void console_puts(const char *s) { }
void console_set_title(const char *title) { }
size_t console_write(const char *buf, size_t sz) { return 0; }

// Defined in "common/graphics.c"
void graphics_term(void) { }

// Defined in "core/Commands.c"
char DimUsed;
int doindex;
struct s_dostack dostack[MAXDOLOOPS];
const char *errorstack[MAXGOSUB];
int forindex;
struct s_forstack forstack[MAXFORLOOPS + 1];
int gosubindex;
const char *gosubstack[MAXGOSUB];
int TraceBuffIndex;
const char *TraceBuff[TRACE_BUFF_SIZE];
int TraceOn;
void CheckAbort(void) { }
void ListNewLine(int *ListCnt, int all) { }

} // extern "C"

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
        MakeFile(path, "");
    }

    void MakeFile(const char *path, const char *contents) {
        FILE *f = fopen(path, "w");
        fprintf(f, "%s", contents); // Assuming contents contains no NULLs.
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
        program_internal_alloc();
    }

    void TearDown() override {
        RemoveTemporaryFiles();
        program_internal_free();
    }
};

#define TEST_PROGRAM_GET_BAS_FILE(filename, expected) \
    result = program_get_bas_file(filename, out); \
    EXPECT_EQ(kOk, result); \
    EXPECT_STREQ(expected, out)

TEST_F(ProgramTest, GetBasFile_GivenAbsolutePath) {
    char out[STRINGSIZE] = { '\0' };
    MmResult result;

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
    MmResult result;

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
    TEST_PROGRAM_GET_BAS_FILE("foo",     PathToFileInCwd("foo").c_str());
}

TEST_F(ProgramTest, GetBasFile_GivenRunningProgram_AndAbsolutePath) {
    char out[STRINGSIZE] = { '\0' };
    MmResult result;
    CurrentLinePtr = (char *) 1; // anything other than 0.
    strcpy(CurrentFile, PROGRAM_TEST_DIR "/current.bas");

    TEST_PROGRAM_GET_BAS_FILE(PROGRAM_TEST_DIR "/bar/foo.bas", PROGRAM_TEST_DIR "/bar/foo.bas");
}

TEST_F(ProgramTest, GetBasFile_GivenRunningProgram_AndRelativePath) {
    char out[STRINGSIZE] = { '\0' };
    MmResult result;
    CurrentLinePtr = (char *) 1; // anything other than 0.
    strcpy(CurrentFile, PROGRAM_TEST_DIR "/current.bas");

    // Contrary to my original belief the file should be is resolved relative
    // to CWD and not to the directory containing the currently running program.
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PathToFileInCwd("foo.bas").c_str());
}

TEST_F(ProgramTest, GetBasFile_GivenOnlyInSearchPath) {
    char out[STRINGSIZE] = { '\0' };
    MmResult result;
    MakeEmptyFile(PROGRAM_TEST_DIR "/foo.bas");

    // Given no SEARCH PATH,
    // expect to resolve to non-existent file in CWD.
    strcpy(mmb_options.search_path, "");
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PathToFileInCwd("foo.bas").c_str());

    // Given matching file in SEARCH PATH,
    // expect to resolve to file in SEARCH PATH.
    strcpy(mmb_options.search_path, PROGRAM_TEST_DIR);
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PROGRAM_TEST_DIR "/foo.bas");

    // Given file in SEARCH PATH with non-matching extension,
    // expect to resolve to non-existent file in CWD.
    strcpy(mmb_options.search_path, PROGRAM_TEST_DIR);
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", PathToFileInCwd("foo.BAS").c_str());

    // Given no extension and no matching file,
    // expect to resolve to file with extension in SEARCH PATH.
    strcpy(mmb_options.search_path, PROGRAM_TEST_DIR);
    TEST_PROGRAM_GET_BAS_FILE("foo", PROGRAM_TEST_DIR "/foo.bas");

    // Given no extension and matching file in SEARCH PATH,
    // expect to resolve to file in SEARCH PATH.
    MakeEmptyFile(PROGRAM_TEST_DIR "/foo");
    strcpy(mmb_options.search_path, PROGRAM_TEST_DIR);
    TEST_PROGRAM_GET_BAS_FILE("foo", PROGRAM_TEST_DIR "/foo");

    // Given no extension and matching file in CWD,
    // expect to resolve to file in CWD.
    MakeEmptyFile("foo");
    strcpy(mmb_options.search_path, PROGRAM_TEST_DIR);
    TEST_PROGRAM_GET_BAS_FILE("foo", PathToFileInCwd("foo").c_str());

    // Given file in CWD and SEARCH PATH
    // expect to resolve to file in CWD.
    MakeEmptyFile("foo.bas");
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", PathToFileInCwd("foo.bas").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", PathToFileInCwd("foo.BAS").c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo",     PathToFileInCwd("foo").c_str());
}

#define TEST_PROGRAM_GET_INC_FILE(filename, expected) \
    result = program_get_inc_file(bas_file, filename, out); \
    EXPECT_EQ(kOk, result); \
    EXPECT_STREQ(expected, out)

TEST_F(ProgramTest, GetIncFile_GivenAbsolutePath) {
    const char *bas_file = PROGRAM_TEST_DIR "/bar/myprog.bas";
    char out[STRINGSIZE] = { '\0' };
    MmResult result;

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
    MmResult result;

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
    TEST_PROGRAM_GET_INC_FILE("foo",     PROGRAM_TEST_DIR "/bar/foo");
}

TEST_F(ProgramTest, ProcessLine_GivenEmptyLine) {
    char line[STRINGSIZE] = { 0 };

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("", line);
}

TEST_F(ProgramTest, ProcessLine_GivenLeadingWhitespace_StripsWhitespace) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "    Print 5");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("PRINT 5", line);
}

TEST_F(ProgramTest, ProcessLine_GivenRemCommandWithContent) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "REM foo bar");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("", line);
}

TEST_F(ProgramTest, ProcessLine_GivenRemCommandWithNoContent) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "REM");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("", line);
}

TEST_F(ProgramTest, ProcessLine_GivenComment_RemovesComment) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "Print 5 ' foo bar");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("PRINT 5", line);
}

TEST_F(ProgramTest, ProcessLine_GivenCommentContainingDoubleQuote_RemovesComment) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "Print 5 ' foo \" bar");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("PRINT 5", line);
}

TEST_F(ProgramTest, ProcessLine_GivenStringContainingSingleQuote) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "Print \"'\"");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("PRINT \"'\"", line);
}

TEST_F(ProgramTest, ProcessLine_CompressesSpaces) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "Print   \"foo bar\"  +    \"wom   bat\"");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("PRINT \"foo bar\" + \"wom   bat\"", line);
}

TEST_F(ProgramTest, ProcessLine_GivenUnclosedDoubleQuote) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "Print \"foo bar");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("PRINT \"foo bar\"", line);
}

TEST_F(ProgramTest, ProcessLine_GivenTabs_ReplacesWithSpaces) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "Print\t\"foo\tbar"); // Even within quotes.

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("PRINT \"foo bar\"", line);
}

TEST_F(ProgramTest, ProcessLine_GivenData_DoesNotConvertToUpperCase) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "Data abc, \"def\"");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("Data abc, \"def\"", line); // TODO: Keyword should probably be capitalised.
}

TEST_F(ProgramTest, ProcessLine_GivenLessThanOrEqual_ConvertsToCanonicalForm) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "If a <= b And b =< c Then");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("IF A <= B AND B <= C THEN", line);
}

TEST_F(ProgramTest, ProcessLine_GivenGreaterThanOrEqual_ConvertsToCanonicalForm) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "If a >= b And b => c Then");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("IF A >= B AND B >= C THEN", line);
}

TEST_F(ProgramTest, ProcessLine_GivenDirective) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "#include \"foo\"");

    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("#INCLUDE \"foo\"", line);
}

TEST_F(ProgramTest, ProcessLine_AppliesDefineReplacements) {
    char line[STRINGSIZE] = { 0 };
    strcpy(line, "wom bat");
    program_add_define("wom", "foo");
    program_add_define("bat", "snafu");

    // Matching is case-insensitive and replacements are in upper-case.
    EXPECT_EQ(kOk, program_process_line(line));
    EXPECT_STREQ("FOO SNAFU", line);
}

TEST_F(ProgramTest, ProcessFile_GivenEmpty) {
    const char *file_path = PROGRAM_TEST_DIR "/foo.bas";
    MakeFile(file_path, "");
    char *p = program_get_edit_buffer();

    EXPECT_EQ(kOk, program_process_file(file_path, &p));
    EXPECT_STREQ("", program_get_edit_buffer());
}

TEST_F(ProgramTest, ProcessFile_GivenNotEmpty) {
    const char *file_path = PROGRAM_TEST_DIR "/foo.bas";
    MakeFile(file_path,
        "Print \"Hello World\"\n"
        "Dim a = 1");
    char *p = program_get_edit_buffer();

    EXPECT_EQ(kOk, program_process_file(file_path, &p));
    EXPECT_STREQ(
        "PRINT \"Hello World\"'|1\n"
        "DIM A = 1'|2\n",
        program_get_edit_buffer());
}

TEST_F(ProgramTest, ProcessFile_GivenEmptyLines_IgnoresThem) {
    const char *file_path = PROGRAM_TEST_DIR "/foo.bas";
    MakeFile(file_path,
        "\n"
        "Print \"Hello World\"\n"
        "    \n"
        "Dim a = 1"
        "' this is a comment");
    char *p = program_get_edit_buffer();

    EXPECT_EQ(kOk, program_process_file(file_path, &p));
    EXPECT_STREQ(
        "PRINT \"Hello World\"'|2\n"
        "DIM A = 1'|4\n",
        program_get_edit_buffer());
}

TEST_F(ProgramTest, ProcessFile_GivenHierarchicalInclude) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Main\n"
        "#Include \"one.inc\"\n"
        "#Include \"two.inc\"");
    const char *one_path = PROGRAM_TEST_DIR "/one.inc";
    MakeFile(one_path,
        "One\n"
        "#Include \"three.inc\"");
    const char *two_path = PROGRAM_TEST_DIR "/two.inc";
    MakeFile(two_path, "Two");
    const char *three_path = PROGRAM_TEST_DIR "/three.inc";
    MakeFile(three_path, "Three");
    char *p = program_get_edit_buffer();

    EXPECT_EQ(kOk, program_process_file(main_path, &p));
    EXPECT_STREQ(
        "MAIN'|1\n"
        "ONE'|one.inc,1\n"
        "THREE'|three.inc,1\n"
        "TWO'|two.inc,1\n",
        program_get_edit_buffer());
}

TEST_F(ProgramTest, ProcessFile_GivenDefineDirective_CreatesDefine) {
    const char *file_path = PROGRAM_TEST_DIR "/foo.bas";
    MakeFile(file_path, "#Define \"foo\", \"bar\"");
    char *p = program_get_edit_buffer();

    EXPECT_EQ(kOk, program_process_file(file_path, &p));
    EXPECT_STREQ("", program_get_edit_buffer());
    EXPECT_EQ(1, program_get_num_defines());
    const char *from, *to;
    EXPECT_EQ(kOk, program_get_define(0, &from, &to));
    EXPECT_STREQ("foo", from);
    EXPECT_STREQ("bar", to);
}

TEST_F(ProgramTest, ProcessFile_GivenUnnownDirective_IgnoresIt) {
    const char *file_path = PROGRAM_TEST_DIR "/foo.bas";
    MakeFile(file_path, "#Unknown \"wom.inc\"");
    char *p = program_get_edit_buffer();

    EXPECT_EQ(kOk, program_process_file(file_path, &p));
    EXPECT_STREQ("", program_get_edit_buffer());
}
