/*
 * Copyright (c) 2021-2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.
#include <climits>
#include <dirent.h>

#include "test_config.h"

extern "C" {

#include "test_helper.h"
#include "../error.h"
#include "../memory.h"
#include "../options.h"
#include "../program.h"
#include "../utility.h"
#include "../../core/Commands.h"
#include "../../core/MMBasic.h"
#include "../../core/tokentbl.h"
#include "../../core/vartbl.h"
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

#define CMD_DATA    "\x94\x80"
#define CMD_DIM     "\x96\x80"
#define CMD_END     "\x9C\x80"
#define CMD_LET     "\xB9\x80"
#define CMD_MMDEBUG "\xC5\x80"
#define CMD_PRINT   "\xD1\x80"
#define OP_EQUALS   "\xEC"

#define EXPECT_PROGRAM_EQ(prog) \
    EXPECT_THAT(std::vector<char>(ProgMemory, ProgMemory + prog.length()), \
                ::testing::ElementsAreArray(prog.buffer, prog.length()));

class ExpectedProgram {

public:
    ExpectedProgram() {
        p = buffer;
    }

    void appendChar(char c) {
        *p++ = c;
    }

    void appendLine(const char *s) {
        appendChar('\x01');
        appendString(s);
    }

    void appendString(const char *s) {
        do {
            appendChar(*s);
        } while (*s++);
    }

    void end() {
        appendChar('\0');
        appendChar('\0');
        appendChar('\xFF');
    }

    size_t length() {
        return p - buffer;
    }

    char buffer[512];
    char *p;
};

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

        vartbl_init_called = false;
        errno = 0;
        strcpy(error_msg, "");
        InitBasic();
        clear_prog_memory();
    }

    void TearDown() override {
        RemoveTemporaryFiles();
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

TEST_F(ProgramTest, HardcodedTokenValuesAreCorrect) {
    char buf[3];
    char *p = buf;
    commandtbl_encode(&p, commandtbl_get("Data"));
    EXPECT_STREQ(CMD_DATA, buf);
    p = buf;
    commandtbl_encode(&p, commandtbl_get("Dim"));
    EXPECT_STREQ(CMD_DIM, buf);
    p = buf;
    commandtbl_encode(&p, commandtbl_get("End"));
    EXPECT_STREQ(CMD_END, buf);
    p = buf;
    commandtbl_encode(&p, commandtbl_get("Let"));
    EXPECT_STREQ(CMD_LET, buf);
    p = buf;
    commandtbl_encode(&p, commandtbl_get("MmDebug"));
    EXPECT_STREQ(CMD_MMDEBUG, buf);
    p = buf;
    commandtbl_encode(&p, commandtbl_get("Print"));
    EXPECT_STREQ(CMD_PRINT, buf);

    sprintf(buf, "%c", (char) tokentbl_get("="));
    EXPECT_STREQ(OP_EQUALS, buf);
}

TEST_F(ProgramTest, LoadFile_GivenFileNotFound) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";

    EXPECT_EQ(kFileNotFound, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
}

TEST_F(ProgramTest, LoadFile_GivenEmptyFile) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path, "");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenLineTooLong) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890");  // 260 chars.

    EXPECT_EQ(kLineTooLong, program_load_file(main_path));
    EXPECT_STREQ("Line too long", error_msg); // Because we hit the legacy error handling.
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenEmptyLine_StripsIt) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "\n"
        "Dim a = 1\n"
        "    ");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenExtraWhitespace_StripsIt) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path, "     Print   \"Hello World\"    ");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenRemCommandWithContent_StripsIt) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "REM foo bar\n"
        "Dim a = 1");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenRemCommandWithoutContent_StripsIt) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "  REM\n"
        "Dim a = 1");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenRemIsNotFirstCommandOnLine_StripsIt) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\" : REM foo bar\n"
        "Dim a = 1:REM");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    // TODO: Not quite what we want; the trailing ':' are transformed to '\0' in the output.
    //       We do not naively strip trailing ':' because they may terminate a label.
    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\" "); e.appendString("'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1");  e.appendString("'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenComment_StripsIt) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\" 'comment1\n"
        "  ' comment2\n"
        "Dim a = 1    'comment3");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenCommentContainingDoubleQuotes_StripsIt) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\" 'foo \"bar\n"
        "  ' \"wom bat\"\n"
        "Dim a = 1    'comment3");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenStringContainingSingleQuote_DoesNotTreatAsComment) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello 'World\"\n"
        "\n"
        "Dim a = 1");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello 'World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenUnclosedDoubleQuote_InsertsClosingQuote) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\n"
        "Dim a = 1");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenCommentContainingUnclosedDoubleQuote_StripsIt) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print '\"Hello World\n"
        "Dim a = 1");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenTabs_ReplacesWithSpaces) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print\t\"Hello\t\tWorld\t\n"
        "Dim\t\ta\t=\t1");
    mmb_options.tab = 2;

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello    World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenData_DoesNotConvertToUpperCase) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\n"
        "Data abc, \"def\"");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DATA "abc, \"def\"'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenIncludeDirective_IncludesFiles) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Dim Main = 0\n"
        "#Include \"one.inc\"\n"
        "#Include \"two.inc\"");
    const char *one_path = PROGRAM_TEST_DIR "/one.inc";
    MakeFile(one_path,
        "Dim One = 1\n"
        "#Include \"three.inc\"");
    const char *two_path = PROGRAM_TEST_DIR "/two.inc";
    MakeFile(two_path, "Dim Two = 2");
    const char *three_path = PROGRAM_TEST_DIR "/three.inc";
    MakeFile(three_path, "Dim Three = 3");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_DIM "MAIN " OP_EQUALS " 0'|1");
    e.appendLine(CMD_DIM "ONE " OP_EQUALS " 1'|one.inc,1");
    e.appendLine(CMD_DIM "THREE " OP_EQUALS " 3'|three.inc,1");
    e.appendLine(CMD_DIM "TWO " OP_EQUALS " 2'|two.inc,1");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenIncludeDirective_AndFileNotFound) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path, "#Include \"not_found.inc\"");

    EXPECT_EQ(kFileNotFound, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(1, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenDefineDirective_AppliesReplacement) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#define \"foo\", \"bar\"\n"
        "Dim foo = 1\n"
        "FOO = 2"); // And is case-insensitive.

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "BAR " OP_EQUALS " 1'|3");
    e.appendLine(CMD_LET "BAR " OP_EQUALS " 2'|4");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenDefineDirectiveWithMissingComma) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#define \"foo\" \"bar\"\n"
        "Dim foo = 1\n"
        "FOO = 2"); // And is case-insensitive.

    EXPECT_EQ(kSyntax, program_load_file(main_path));
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
    EXPECT_STREQ("", error_msg);
}

TEST_F(ProgramTest, LoadFile_GivenUnknownDirective_StripsIt) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#Unknown\n"
        "Dim a = 1");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenLabel_TokenisesLabel) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "my_label:\n"
        "Dim a = 1");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine("\x03\x08MY_LABEL'|2");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
}

TEST_F(ProgramTest, LoadFile_GivenMultilineComment_StripsIt) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello /*World*/\"\n"
        "Dim /*Strip this*/a = 1\n"
        "/* Strip this \n"
        "multi line comment */");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello /*World*/\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenMultilineCommentStartedButNotEnded) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "Dim /*Unterminated comment");

    EXPECT_EQ(kUnterminatedComment, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenMultilineCommentEndedButNotStarted) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "Dim a = 1*/");

    EXPECT_EQ(kNoCommentToTerminate, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenNestedMultilineComment) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "Dim a = 1 /* foo /* bar */ wombat */");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenCommentDirective) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#COMMENT START\n"
        "Dim a = 1\n"
        "#COMMENT END");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenCommentDirective_WithMissingSubDirective_Fails) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#COMMENT");  // Missing comment sub-directive.

    EXPECT_EQ(kSyntax, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenCommentDirective_WithInvalidSubDirective_Fails) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#COMMENT foo");  // Invalid comment sub-directive.

    EXPECT_EQ(kSyntax, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenNestedCommentDirective_WithMissingSubDirective_Fails) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#COMMENT START\n"
        "#COMMENT");  // Missing comment sub-directive.

    EXPECT_EQ(kSyntax, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(3, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenNestedCommentDirective_WithInvalidSubDirective_Fails) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#COMMENT START\n"
        "#COMMENT foo");  // Invalid comment sub-directive.

    EXPECT_EQ(kSyntax, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(3, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenCommentStartDirective_WithTrailingText_Fails) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#COMMENT START foo\n"  // Invalid trailing text.
        "#COMMENT END");

    EXPECT_EQ(kSyntax, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenNestedCommentStartDirective_WithTrailingText_Fails) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#COMMENT START\n"
        "#COMMENT START foo\n"  // Invalid trailing text in nested comment.
        "#COMMENT END\n"
        "#COMMENT END");

    EXPECT_EQ(kSyntax, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(3, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugDirective) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "#MMDEBUG ON\n"
        "MmDebug \"Hello World\"\n"
        "MmDebug Break\n"
        "#MMDEBUG OFF\n"
        "MmDebug \"Goodbye World\"\n"
        "MmDebug Break\n"
        "#MMDEBUG ON\n"
        "MmDebug \"Hello Again World\"\n"
        "MmDebug Break");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_MMDEBUG "\"Hello World\"'|2");
    e.appendLine(CMD_MMDEBUG "BREAK'|3");
    e.appendLine(CMD_MMDEBUG "\"Hello Again World\"'|8");
    e.appendLine(CMD_MMDEBUG "BREAK'|9");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugOnDirective_WhileAlreadyOn_Succeeds) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "#MMDEBUG ON\n"
        "MmDebug \"Hello World\"\n"
        "MmDebug Break\n"
        "#MMDEBUG ON\n"
        "MmDebug \"Hello Again World\"\n"
        "MmDebug Break");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_MMDEBUG "\"Hello World\"'|2");
    e.appendLine(CMD_MMDEBUG "BREAK'|3");
    e.appendLine(CMD_MMDEBUG "\"Hello Again World\"'|5");
    e.appendLine(CMD_MMDEBUG "BREAK'|6");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugOffDirective_WhileAlreadyOff_Succeeds) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "#MMDEBUG ON\n"
        "MmDebug \"Hello World\"\n"
        "MmDebug Break\n"
        "#MMDEBUG OFF\n"
        "MmDebug \"Goodbye World\"\n"
        "MmDebug Break\n"
        "#MMDEBUG OFF\n"
        "MmDebug \"Hello Again World\"\n"
        "MmDebug Break");

    EXPECT_EQ(kOk, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendLine("'/tmp/ProgramTest/main.bas");
    e.appendLine(CMD_MMDEBUG "\"Hello World\"'|2");
    e.appendLine(CMD_MMDEBUG "BREAK'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugDirective_WithInvalidSubDirective_Fails) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#MMDEBUG foo");

    EXPECT_EQ(kSyntax, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugDirective_WithMissingSubDirective_Fails) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#MMDEBUG");

    EXPECT_EQ(kSyntax, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugOnDirective_WithTrailingText_Fails) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#MMDEBUG ON foo");

    EXPECT_EQ(kSyntax, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}


TEST_F(ProgramTest, LoadFile_GivenMmDebugOffDirective_WithTrailingText_Fails) {
    const char *main_path = PROGRAM_TEST_DIR "/main.bas";
    MakeFile(main_path,
        "Print \"Hello World\"\n"
        "#MMDEBUG OFF foo");

    EXPECT_EQ(kSyntax, program_load_file(main_path));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_STREQ(main_path, mmb_error_state_ptr->file);
}
