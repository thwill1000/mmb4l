/*
 * Copyright (c) 2021-2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <filesystem>
#include <gtest/gtest.h>
#include <gmock/gmock.h> // Needed for EXPECT_THAT.
#include <climits>

#if !defined(ENABLE_GTEST_EXTRAS)
#define ENABLE_GTEST_EXTRAS
#endif

extern "C" {

#include "test_helper.h"
#include "stubs/error_stubs.h"
#include "../features.h"
#include "../graphics.h"
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
Features mmb_features;
Options mmb_options;
ErrorState mmb_normal_error_state;

// Defined in "commands/cmd_read.c"
void cmd_read_clear_cache()  { }

// Defined in "common/events.c"
void events_pump() { }

// Defined in "common/fonttbl.c"
void font_clear_user_defined() { }

// Defined in "common/gpio.c"
MmResult gpio_term() { return kOk; }
MmResult gpio_translate_from_pin_gp(uint8_t pin_gp, uint8_t *pin_num) { return kOk; }

// Defined in "common/prompt.c"
MmResult prompt_getc(int *ch) {
    *ch = -1;
    return kOk;
}
MmResult prompt_save_history(const char *filepath) {
    return kOk;
}

// Defined in "core/Commands.c"
char DimUsed;
int doindex;
struct s_dostack dostack[MAXDOLOOPS];
const char *errorstack[MAXGOSUB];
int forindex;
struct s_forstack forstack[MAXFORLOOPS + 1];
int gosubindex;
const char *gosubstack[MAXGOSUB];
void ListNewLine(int *ListCnt, int all) { }

} // extern "C"

#define CMD_CSUB        "\x95\x80"
#define CMD_DATA        "\x97\x80"
#define CMD_DEFINEFONT  "\x98\x80"
#define CMD_DIM         "\x9A\x80"
#define CMD_END         "\xA0\x80"
#define CMD_LET         "\xC2\x80"
#define CMD_MMDEBUG     "\xCF\x80"
#define CMD_PRINT       "\xDE\x80"
#if defined(USE_TWO_BYTE_TOKENS)
#define OP_EQUALS       "\xF7\x80"
#else
#define OP_EQUALS       "\xF7"
#endif

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

    void appendProgramPath(const char *path) {
        appendChar('\x01');
        appendChar('\'');
        char normalized_path[PATH_MAX];
        ASSERT_EQ(kOk, file_normalize_separators(path, normalized_path, sizeof(normalized_path)));
        appendString(normalized_path);
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

    std::filesystem::path cwd;
    std::filesystem::path test_dir;

    void SetUp() override {
        // Create a temporary test directory structure
        test_dir = std::filesystem::temp_directory_path() / "program_test";

        // Get the current working directory
        char cwd_[PATH_MAX] = { '\0' };
        ASSERT_EQ(kOk, file_getcwd(cwd_, sizeof(cwd_)));
        cwd = cwd_;

        // Clean up any existing test directory
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }

        // Create test directory structure
        std::filesystem::create_directories(test_dir);
        std::filesystem::create_directory(test_dir / "bar");
        std::filesystem::create_directory("bar"); // In CWD

        vartbl_init_called = false;
        errno = 0;
        strcpy(error_msg, "");
        ASSERT_EQ(kOk, memory_init());
        ASSERT_EQ(kOk, InitBasic());
        clear_prog_memory();
    }

    void TearDown() override {
        // Clean up test directory
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }

        // Clean up working directory
        std::filesystem::remove_all("bar");
        std::filesystem::remove_all("foo");
        std::filesystem::remove_all("foo.bas");
        std::filesystem::remove_all("foo.BAS");
        std::filesystem::remove_all("foo.Bas");

        ASSERT_EQ(kOk, memory_term());
    }
};

#define EXPECT_PATH_EQ(expected, actual)                          \
    do {                                                          \
        if (!file_compare_path(expected, actual)) {               \
            ADD_FAILURE() << "Path mismatch:\n"                   \
                          << "  Expected: " << (expected) << "\n" \
                          << "  Actual:   " << (actual);          \
        }                                                         \
    } while (0)

#define TEST_PROGRAM_GET_BAS_FILE(filename, expected) \
    do {                                              \
        result = program_get_bas_file(filename, out); \
        EXPECT_EQ(kOk, result);                       \
        EXPECT_PATH_EQ(expected, out);                \
    } while (0)

TEST_F(ProgramTest, GetBasFile_GivenAbsolutePath) {
    char out[STRINGSIZE] = { '\0' };
    MmResult result;

    // Test when no file present.
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.bas").string().c_str(), (test_dir / "foo.bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.BAS").string().c_str(), (test_dir / "foo.BAS").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.Bas").string().c_str(), (test_dir / "foo.Bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo").string().c_str(),     (test_dir / "foo.bas").string().c_str());

    // Test when .Bas file present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo.Bas").string().c_str(), ""));
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.bas").string().c_str(), (test_dir / "foo.bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.BAS").string().c_str(), (test_dir / "foo.BAS").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.Bas").string().c_str(), (test_dir / "foo.Bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo").string().c_str(),     (test_dir / "foo.Bas").string().c_str());

#if !defined(_WIN32)  // Not valid on Windows because of case-insensitivity.
    // Test when .BAS and .Bas files present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo.BAS").string().c_str(), ""));
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.bas").string().c_str(), (test_dir / "foo.bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.BAS").string().c_str(), (test_dir / "foo.BAS").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.Bas").string().c_str(), (test_dir / "foo.Bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo").string().c_str(),     (test_dir / "foo.BAS").string().c_str());

    // Test when .bas, BAS and .Bas files present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo.bas").string().c_str(), ""));
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.bas").string().c_str(), (test_dir / "foo.bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.BAS").string().c_str(), (test_dir / "foo.BAS").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo.Bas").string().c_str(), (test_dir / "foo.Bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo").string().c_str(),     (test_dir / "foo.bas").string().c_str());
#endif // !defined(_WIN32)

    // Test when "foo" without extension is present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo").string().c_str(), ""));
    TEST_PROGRAM_GET_BAS_FILE((test_dir / "foo").string().c_str(),     (test_dir / "foo").string().c_str());
}

TEST_F(ProgramTest, GetBasFile_GivenRelativePath) {
    char out[STRINGSIZE] = { '\0' };
    MmResult result;

    // Test when no file present.
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", (cwd / "foo.bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", (cwd / "foo.BAS").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.Bas", (cwd / "foo.Bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo",     (cwd / "foo.bas").string().c_str());

    // Test when .Bas file present.
    ASSERT_EQ(kOk, file_mkfile("foo.Bas", ""));
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", (cwd / "foo.bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", (cwd / "foo.BAS").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.Bas", (cwd / "foo.Bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo",     (cwd / "foo.Bas").string().c_str());

#if !defined(_WIN32)  // Not valid on Windows because of case-insensitivity.
    // Test when .BAS and .Bas files present.
    ASSERT_EQ(kOk, file_mkfile("foo.BAS", ""));
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", (cwd / "foo.bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", (cwd / "foo.BAS").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.Bas", (cwd / "foo.Bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo",     (cwd / "foo.BAS").string().c_str());

    // Test when .bas, BAS and .Bas files present.
    ASSERT_EQ(kOk, file_mkfile("foo.bas", ""));
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", (cwd / "foo.bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", (cwd / "foo.BAS").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.Bas", (cwd / "foo.Bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo",     (cwd / "foo.bas").string().c_str());
#endif // #if !defined(_WIN32)

    // Test when "foo" without extension is present.
    ASSERT_EQ(kOk, file_mkfile("foo", ""));
    TEST_PROGRAM_GET_BAS_FILE("foo",     (cwd / "foo").string().c_str());
}

TEST_F(ProgramTest, GetBasFile_GivenRunningProgram_AndAbsolutePath) {
    char out[STRINGSIZE] = { '\0' };
    MmResult result;
    CurrentLinePtr = (char *) 1; // anything other than 0.
    strcpy(CurrentFile, (test_dir / "current.bas").string().c_str());

    TEST_PROGRAM_GET_BAS_FILE((test_dir / "bar" / "foo.bas").string().c_str(), (test_dir / "bar" / "foo.bas").string().c_str());
}

TEST_F(ProgramTest, GetBasFile_GivenRunningProgram_AndRelativePath) {
    char out[STRINGSIZE] = { '\0' };
    MmResult result;
    CurrentLinePtr = (char *) 1; // anything other than 0.
    strcpy(CurrentFile, (test_dir / "current.bas").string().c_str());

    // Contrary to my original belief the file should be is resolved relative
    // to CWD and not to the directory containing the currently running program.
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", (cwd / "foo.bas").string().c_str());
}

TEST_F(ProgramTest, GetBasFile_GivenOnlyInSearchPath) {
    char out[STRINGSIZE] = { '\0' };
    MmResult result;
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo.bas").string().c_str(), ""));

    // Given no SEARCH PATH,
    // expect to resolve to non-existent file in CWD.
    strcpy(mmb_options.search_path, "");
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", (cwd / "foo.bas").string().c_str());

    // Given matching file in SEARCH PATH,
    // expect to resolve to file in SEARCH PATH.
    strcpy(mmb_options.search_path, test_dir.string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", (test_dir / "foo.bas").string().c_str());

#if !defined(_WIN32) // Not valid on Windows because of case-insensitivity.
    // Given file in SEARCH PATH with non-matching extension,
    // expect to resolve to non-existent file in CWD.
    strcpy(mmb_options.search_path, test_dir.string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", (cwd / "foo.BAS").string().c_str());
#endif // !defined(_WIN32)

    // Given no extension and no matching file,
    // expect to resolve to file with extension in SEARCH PATH.
    strcpy(mmb_options.search_path, test_dir.string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo", (test_dir / "foo.bas").string().c_str());

    // Given no extension and matching file in SEARCH PATH,
    // expect to resolve to file in SEARCH PATH.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo").string().c_str(), ""));
    strcpy(mmb_options.search_path, test_dir.string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo", (test_dir / "foo").string().c_str());

    // Given no extension and matching file in CWD,
    // expect to resolve to file in CWD.
    ASSERT_EQ(kOk, file_mkfile("foo", ""));
    strcpy(mmb_options.search_path, test_dir.string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo", (cwd / "foo").string().c_str());

    // Given file in CWD and SEARCH PATH
    // expect to resolve to file in CWD.
    ASSERT_EQ(kOk, file_mkfile("foo.bas", ""));
    TEST_PROGRAM_GET_BAS_FILE("foo.bas", (cwd / "foo.bas").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo.BAS", (cwd / "foo.BAS").string().c_str());
    TEST_PROGRAM_GET_BAS_FILE("foo",     (cwd / "foo").string().c_str());
}

#define TEST_PROGRAM_GET_INC_FILE(filename, expected)                   \
    do {                                                                \
        result = program_get_inc_file(bas_file.c_str(), filename, out); \
        EXPECT_EQ(kOk, result);                                         \
        EXPECT_PATH_EQ(expected, out);                                  \
    } while (0)

TEST_F(ProgramTest, GetIncFile_GivenAbsolutePath) {
    std::string bas_file = (test_dir / "bar" / "myprog.bas").string();
    char out[STRINGSIZE] = { '\0' };
    MmResult result;

    // Test when no file present.
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.inc").string().c_str(), (test_dir / "foo.inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.INC").string().c_str(), (test_dir / "foo.INC").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.Inc").string().c_str(), (test_dir / "foo.Inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo").string().c_str(), (test_dir / "foo.inc").string().c_str());

    // Test when .Inc file present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo.Inc").string().c_str(), ""));
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.inc").string().c_str(), (test_dir / "foo.inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.INC").string().c_str(), (test_dir / "foo.INC").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.Inc").string().c_str(), (test_dir / "foo.Inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo").string().c_str(), (test_dir / "foo.Inc").string().c_str());

#if !defined(_WIN32)  // Not valid on Windows because of case-insensitivity.
    // Test when .INC and .inc files present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo.INC").string().c_str(), ""));
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.inc").string().c_str(), (test_dir / "foo.inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.INC").string().c_str(), (test_dir / "foo.INC").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.Inc").string().c_str(), (test_dir / "foo.Inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo").string().c_str(), (test_dir / "foo.INC").string().c_str());

    // Test when .inc, .INC and .Inc files present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo.inc").string().c_str(), ""));
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.inc").string().c_str(), (test_dir / "foo.inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.INC").string().c_str(), (test_dir / "foo.INC").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo.Inc").string().c_str(), (test_dir / "foo.Inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo").string().c_str(), (test_dir / "foo.inc").string().c_str());
#endif // !defined(_WIN32)

// Test when "foo" without extension is present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "foo").string().c_str(), ""));
    TEST_PROGRAM_GET_INC_FILE((test_dir / "foo").string().c_str(), (test_dir / "foo").string().c_str());
}

TEST_F(ProgramTest, GetIncFile_GivenRelativePath) {
    std::string bas_file = (test_dir / "bar" / "myprog.bas").string();
    char out[STRINGSIZE] = { '\0' };
    MmResult result;

    // Test when no file present.
    TEST_PROGRAM_GET_INC_FILE("foo.inc", (test_dir / "bar" / "foo.inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo.INC", (test_dir / "bar" / "foo.INC").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo.Inc", (test_dir / "bar" / "foo.Inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo",     (test_dir / "bar" / "foo.inc").string().c_str());

    // Test when .Inc file present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "bar" / "foo.Inc").string().c_str(), ""));
    TEST_PROGRAM_GET_INC_FILE("foo.inc", (test_dir / "bar" / "foo.inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo.INC", (test_dir / "bar" / "foo.INC").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo.Inc", (test_dir / "bar" / "foo.Inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo",     (test_dir / "bar" / "foo.Inc").string().c_str());

#if !defined(_WIN32)  // Not valid on Windows because of case-insensitivity.
    // Test when .INC and .inc files present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "bar" / "foo.INC").string().c_str(), ""));
    TEST_PROGRAM_GET_INC_FILE("foo.inc", (test_dir / "bar" / "foo.inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo.INC", (test_dir / "bar" / "foo.INC").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo.Inc", (test_dir / "bar" / "foo.Inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo",     (test_dir / "bar" / "foo.INC").string().c_str());

    // Test when .inc, .INC and .Inc files present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "bar" / "foo.inc").string().c_str(), ""));
    TEST_PROGRAM_GET_INC_FILE("foo.inc", (test_dir / "bar" / "foo.inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo.INC", (test_dir / "bar" / "foo.INC").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo.Inc", (test_dir / "bar" / "foo.Inc").string().c_str());
    TEST_PROGRAM_GET_INC_FILE("foo",     (test_dir / "bar" / "foo.inc").string().c_str());
#endif // !defined(_WIN32)

    // Test when "foo" without extension is present.
    ASSERT_EQ(kOk, file_mkfile((test_dir / "bar" / "foo").string().c_str(), ""));
    TEST_PROGRAM_GET_INC_FILE("foo",     (test_dir / "bar" / "foo").string().c_str());
}

TEST_F(ProgramTest, HardcodedTokenValuesAreCorrect) {
    EXPECT_STREQ(CMD_CSUB, commandtbl_encoded("CSub"));
    EXPECT_STREQ(CMD_DATA, commandtbl_encoded("Data"));
    EXPECT_STREQ(CMD_DEFINEFONT, commandtbl_encoded("DefineFont"));
    EXPECT_STREQ(CMD_DIM, commandtbl_encoded("Dim"));
    EXPECT_STREQ(CMD_END, commandtbl_encoded("End"));
    EXPECT_STREQ(CMD_LET, commandtbl_encoded("Let"));
    EXPECT_STREQ(CMD_MMDEBUG, commandtbl_encoded("MmDebug"));
    EXPECT_STREQ(CMD_PRINT, commandtbl_encoded("Print"));
    EXPECT_STREQ(OP_EQUALS, tokentbl_encoded("="));
}

TEST_F(ProgramTest, LoadFile_GivenFileNotFound) {
    std::string main_path = (test_dir / "main.bas").string();

    EXPECT_EQ(kFileNotFound, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
}

TEST_F(ProgramTest, LoadFile_GivenEmptyFile) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(), ""));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    char buf[PATH_MAX];
    EXPECT_EQ(kOk, file_normalize_separators(main_path.c_str(), buf + 1, sizeof(buf) - 1));
    buf[0] = '\'';
    e.appendLine(buf);
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenLineTooLong) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456789012345678901234567890123456789"
        "012345678901234567890"));  // 260 chars.

    EXPECT_EQ(kLineTooLong, program_load_file(main_path.c_str()));
    EXPECT_STREQ("Line too long", error_msg); // Because we hit the legacy error handling.
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenEmptyLine_StripsIt) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "\n"
        "Dim a = 1\n"
        "    "));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenExtraWhitespace_StripsIt) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(), "     Print   \"Hello World\"    "));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenRemCommandWithContent_StripsIt) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "REM foo bar\n"
        "Dim a = 1"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenRemCommandWithoutContent_StripsIt) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "  REM\n"
        "Dim a = 1"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenRemIsNotFirstCommandOnLine_StripsIt) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\" : REM foo bar\n"
        "Dim a = 1:REM"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    // TODO: Not quite what we want; the trailing ':' are transformed to '\0' in the output.
    //       We do not naively strip trailing ':' because they may terminate a label.
    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\" "); e.appendString("'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1");  e.appendString("'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenComment_StripsIt) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\" 'comment1\n"
        "  ' comment2\n"
        "Dim a = 1    'comment3"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenCommentContainingDoubleQuotes_StripsIt) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\" 'foo \"bar\n"
        "  ' \"wom bat\"\n"
        "Dim a = 1    'comment3"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenStringContainingSingleQuote_DoesNotTreatAsComment) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello 'World\"\n"
        "\n"
        "Dim a = 1"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello 'World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenUnclosedDoubleQuote_InsertsClosingQuote) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\n"
        "Dim a = 1"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenCommentContainingUnclosedDoubleQuote_StripsIt) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print '\"Hello World\n"
        "Dim a = 1"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenTabs_ReplacesWithSpaces) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print\t\"Hello\t\tWorld\t\n"
        "Dim\t\ta\t=\t1"));
    mmb_options.tab = 2;

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello    World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenData_DoesNotConvertToUpperCase) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\n"
        "Data abc, \"def\""));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DATA "abc, \"def\"'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenIncludeDirective_IncludesFiles) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Dim Main = 0\n"
        "#Include \"one.inc\"\n"
        "#Include \"two.inc\""));
    std::string one_path = (test_dir / "one.inc").string().c_str();
    ASSERT_EQ(kOk, file_mkfile(one_path.c_str(),
        "Dim One = 1\n"
        "#Include \"three.inc\""));
    std::string two_path = (test_dir / "two.inc").string();
    ASSERT_EQ(kOk, file_mkfile(two_path.c_str(), "Dim Two = 2"));
    std::string three_path = (test_dir / "three.inc").string();
    ASSERT_EQ(kOk, file_mkfile(three_path.c_str(), "Dim Three = 3"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_DIM "MAIN " OP_EQUALS " 0'|1");
    e.appendLine(CMD_DIM "ONE " OP_EQUALS " 1'|one.inc,1");
    e.appendLine(CMD_DIM "THREE " OP_EQUALS " 3'|three.inc,1");
    e.appendLine(CMD_DIM "TWO " OP_EQUALS " 2'|two.inc,1");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenIncludeDirective_AndFileNotFound) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(), "#Include \"not_found.inc\""));

    EXPECT_EQ(kFileNotFound, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(1, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenDefineDirective_AppliesReplacement) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#define \"foo\", \"bar\"\n"
        "Dim foo = 1\n"
        "FOO = 2")); // And is case-insensitive.

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "BAR " OP_EQUALS " 1'|3");
    e.appendLine(CMD_LET "BAR " OP_EQUALS " 2'|4");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenDefineDirectiveWithMissingComma) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#define \"foo\" \"bar\"\n"
        "Dim foo = 1\n"
        "FOO = 2")); // And is case-insensitive.

    EXPECT_EQ(kSyntax, program_load_file(main_path.c_str()));
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
    EXPECT_STREQ("", error_msg);
}

TEST_F(ProgramTest, LoadFile_GivenUnknownDirective_StripsIt) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#Unknown\n"
        "Dim a = 1"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenLabel_TokenisesLabel) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "my_label:\n"
        "Dim a = 1"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine("\x03\x08MY_LABEL'|2");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
}

TEST_F(ProgramTest, LoadFile_GivenMultilineComment_StripsIt) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello /*World*/\"\n"
        "Dim /*Strip this*/a = 1\n"
        "/* Strip this \n"
        "multi line comment */"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello /*World*/\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenMultilineCommentStartedButNotEnded) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "Dim /*Unterminated comment"));

    EXPECT_EQ(kUnterminatedComment, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenMultilineCommentEndedButNotStarted) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "Dim a = 1*/"));

    EXPECT_EQ(kNoCommentToTerminate, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenNestedMultilineComment) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "Dim a = 1 /* foo /* bar */ wombat */"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|2");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenCommentDirective) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#COMMENT START\n"
        "Dim a = 1\n"
        "#COMMENT END"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenCommentDirective_WithMissingSubDirective_Fails) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#COMMENT"));  // Missing comment sub-directive.

    EXPECT_EQ(kSyntax, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenCommentDirective_WithInvalidSubDirective_Fails) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#COMMENT foo"));  // Invalid comment sub-directive.

    EXPECT_EQ(kSyntax, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenNestedCommentDirective_WithMissingSubDirective_Fails) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#COMMENT START\n"
        "#COMMENT"));  // Missing comment sub-directive.

    EXPECT_EQ(kSyntax, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(3, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenNestedCommentDirective_WithInvalidSubDirective_Fails) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#COMMENT START\n"
        "#COMMENT foo"))    ;  // Invalid comment sub-directive.

    EXPECT_EQ(kSyntax, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(3, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenCommentStartDirective_WithTrailingText_Fails) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#COMMENT START foo\n"  // Invalid trailing text.
        "#COMMENT END"));

    EXPECT_EQ(kSyntax, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenNestedCommentStartDirective_WithTrailingText_Fails) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#COMMENT START\n"
        "#COMMENT START foo\n"  // Invalid trailing text in nested comment.
        "#COMMENT END\n"
        "#COMMENT END"));

    EXPECT_EQ(kSyntax, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(3, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugDirective) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "#MMDEBUG ON\n"
        "MmDebug \"Hello World\"\n"
        "MmDebug Break\n"
        "#MMDEBUG OFF\n"
        "MmDebug \"Goodbye World\"\n"
        "MmDebug Break\n"
        "#MMDEBUG ON\n"
        "MmDebug \"Hello Again World\"\n"
        "MmDebug Break"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_MMDEBUG "\"Hello World\"'|2");
    e.appendLine(CMD_MMDEBUG "BREAK'|3");
    e.appendLine(CMD_MMDEBUG "\"Hello Again World\"'|8");
    e.appendLine(CMD_MMDEBUG "BREAK'|9");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugOnDirective_WhileAlreadyOn_Succeeds) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "#MMDEBUG ON\n"
        "MmDebug \"Hello World\"\n"
        "MmDebug Break\n"
        "#MMDEBUG ON\n"
        "MmDebug \"Hello Again World\"\n"
        "MmDebug Break"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_MMDEBUG "\"Hello World\"'|2");
    e.appendLine(CMD_MMDEBUG "BREAK'|3");
    e.appendLine(CMD_MMDEBUG "\"Hello Again World\"'|5");
    e.appendLine(CMD_MMDEBUG "BREAK'|6");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugOffDirective_WhileAlreadyOff_Succeeds) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "#MMDEBUG ON\n"
        "MmDebug \"Hello World\"\n"
        "MmDebug Break\n"
        "#MMDEBUG OFF\n"
        "MmDebug \"Goodbye World\"\n"
        "MmDebug Break\n"
        "#MMDEBUG OFF\n"
        "MmDebug \"Hello Again World\"\n"
        "MmDebug Break"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_MMDEBUG "\"Hello World\"'|2");
    e.appendLine(CMD_MMDEBUG "BREAK'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugDirective_WithInvalidSubDirective_Fails) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#MMDEBUG foo"));

    EXPECT_EQ(kSyntax, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugDirective_WithMissingSubDirective_Fails) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#MMDEBUG"));

    EXPECT_EQ(kSyntax, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugOnDirective_WithTrailingText_Fails) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#MMDEBUG ON foo"));

    EXPECT_EQ(kSyntax, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenMmDebugOffDirective_WithTrailingText_Fails) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "#MMDEBUG OFF foo"));

    EXPECT_EQ(kSyntax, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(2, mmb_error_state_ptr->line);
    EXPECT_PATH_EQ(main_path.c_str(), mmb_error_state_ptr->file);
}

TEST_F(ProgramTest, LoadFile_GivenCSub) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "CSub my_csub(arg1, arg2)\n"
        "  12345678\n"
        "  01020304 05060708 090A0B0C 0D0E0F10 11121314 15161718\n"
        "End CSub\n"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    // Expect first 8 bytes of CFunctionFlash to point to the address of the CSUB token.
    void *pcsub = memchr(ProgMemory, CMD_CSUB[0], 256);
    EXPECT_EQ(*((uint64_t *) CFunctionFlash), (uintptr_t) pcsub);

    // Expect next 4 bytes to contain the length in bytes.
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 8)), 28);

    // Expect next 4 bytes to contain the "offset to main()".
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 12)), 0x12345678);

    // Expect 24 bytes of data.
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 16)), 0x01020304);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 20)), 0x05060708);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 24)), 0x090A0B0C);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 28)), 0x0D0E0F10);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 32)), 0x11121314);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 36)), 0x15161718);

    // Expect end marker
    EXPECT_EQ(*((uint64_t *) (CFunctionFlash + 40)), 0xFFFFFFFFFFFFFFFF);
}

TEST_F(ProgramTest, LoadFile_GivenCSub_WithBinaryDataOnFirstLine) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "CSub my_csub 12345678\n"
        "  01020304 05060708 090A0B0C 0D0E0F10 11121314 15161718\n"
        "End CSub\n"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    // Expect first 8 bytes of CFunctionFlash to point to the address of the CSUB token.
    void *pcsub = memchr(ProgMemory, CMD_CSUB[0], 256);
    EXPECT_EQ(*((uint64_t *) CFunctionFlash), (uintptr_t) pcsub);

    // Expect next 4 bytes to contain the length in bytes.
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 8)), 28);

    // Expect next 4 bytes to contain the "offset to main()".
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 12)), 0x12345678);

    // Expect 24 bytes of data.
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 16)), 0x01020304);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 20)), 0x05060708);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 24)), 0x090A0B0C);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 28)), 0x0D0E0F10);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 32)), 0x11121314);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 36)), 0x15161718);

    // Expect end marker
    EXPECT_EQ(*((uint64_t *) (CFunctionFlash + 40)), 0xFFFFFFFFFFFFFFFF);
}

TEST_F(ProgramTest, LoadFile_GivenCSub_WithLineNumbers) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "10 CSub my_csub(arg1, arg2)\n"
        "20  12345678\n"
        "30  01020304 05060708 090A0B0C 0D0E0F10 11121314 15161718\n"
        "40 End CSub\n"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    // Expect first 8 bytes of CFunctionFlash to point to the address of the CSUB token.
    void *pcsub = memchr(ProgMemory, CMD_CSUB[0], 256);
    EXPECT_EQ(*((uint64_t *) CFunctionFlash), (uintptr_t) pcsub);

    // Expect next 4 bytes to contain the length in bytes.
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 8)), 28);

    // Expect next 4 bytes to contain the "offset to main()".
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 12)), 0x12345678);

    // Expect 24 bytes of data.
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 16)), 0x01020304);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 20)), 0x05060708);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 24)), 0x090A0B0C);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 28)), 0x0D0E0F10);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 32)), 0x11121314);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 36)), 0x15161718);

    // Expect end marker
    EXPECT_EQ(*((uint64_t *) (CFunctionFlash + 40)), 0xFFFFFFFFFFFFFFFF);
}

TEST_F(ProgramTest, LoadFile_GivenMultipleCSub) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "CSub my_csub(arg1, arg2)\n"
        "  12345678\n"
        "  11111111 22222222\n"
        "  33333333\n"
        "End CSub\n"
        "CSub my_csub_2(arg3)\n"
        "  90ABCDEF\n"
        "  55555555 66666666\n"
        "End CSub\n"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    // my_csub()
    char *pcsub1;
    {
        // Expect first 8 bytes of CFunctionFlash to point to the address of the CSUB token.
        pcsub1 = (char *) memchr(ProgMemory, CMD_CSUB[0], 256);
        EXPECT_EQ(*((uint64_t *) CFunctionFlash), (uintptr_t) pcsub1);

        // Expect next 4 bytes to contain the length in bytes.
        EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 8)), 16);

        // Expect next 4 bytes to contain the "offset to main()".
        EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 12)), 0x12345678);

        // Expect 12 bytes of data.
        EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 16)), 0x11111111);
        EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 20)), 0x22222222);
        EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 24)), 0x33333333);

        // Expect zero padding to 64-bit boundary.
        EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 28)), 0x0);
    }

    // my_csub_2()
    char *pcsub2;
    {
        // Expect first 8 bytes of CFunctionFlash to point to the address of the CSUB token.
        pcsub2 = (char *) memchr(pcsub1 + 1, CMD_CSUB[0], 256);
        EXPECT_EQ(*((uint64_t *) (CFunctionFlash + 32)), (uintptr_t) pcsub2);

        // Expect next 4 bytes to contain the length in bytes.
        EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 40)), 12);

        // Expect next 4 bytes to contain the "offset to main()".
        EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 44)), 0x90ABCDEF);

        // Expect 8 bytes of data.
        EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 48)), 0x55555555);
        EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 52)), 0x66666666);
    }

    // Expect end marker
    EXPECT_EQ(*((uint64_t *) (CFunctionFlash + 56)), 0xFFFFFFFFFFFFFFFF);
}

TEST_F(ProgramTest, LoadFile_GivenDefineFont) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "DefineFont #9\n"
        "  5F200808\n"
        "  00000000 00000000 18181818 00180018 006C6C6C 00000000 367F3636 0036367F\n"
        "  3E683F0C 00187E0B 180C6660 00066630 386C6C38 003B666D 0030180C 00000000\n"
        "End DefineFont\n"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);

    // Expect first 8 bytes of CFunctionFlash to contain the font number - 1.
    EXPECT_EQ(*((uint64_t *) CFunctionFlash), 9 - 1);

    // Expect next 4 bytes to contain the length in bytes.
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 8)), 17 * 4);

    // Expect 17 * 4 bytes of data.
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 12)), 0x5F200808);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 16)), 0x00000000);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 20)), 0x00000000);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 24)), 0x18181818);
    // ...
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 72)), 0x0030180C);
    EXPECT_EQ(*((uint32_t *) (CFunctionFlash + 76)), 0x00000000);

    // Expect end marker
    EXPECT_EQ(*((uint64_t *) (CFunctionFlash + 80)), 0xFFFFFFFFFFFFFFFF);
}

TEST_F(ProgramTest, LoadFile_GivenSucceeds_RestoresErrorLineToOne) {
    std::string main_path = (test_dir / "main.bas").string();
    ASSERT_EQ(kOk, file_mkfile(main_path.c_str(),
        "Print \"Hello World\"\n"
        "Print \"Goodbye Wold\"\n"
        "Dim a = 1"));

    EXPECT_EQ(kOk, program_load_file(main_path.c_str()));
    EXPECT_STREQ("", error_msg);
    EXPECT_EQ(1, mmb_error_state_ptr->line);
    EXPECT_EQ(false, mmb_error_state_ptr->override_line);


    ExpectedProgram e;
    e.appendProgramPath(main_path.c_str());
    e.appendLine(CMD_PRINT "\"Hello World\"'|1");
    e.appendLine(CMD_PRINT "\"Goodbye Wold\"'|2");
    e.appendLine(CMD_DIM "A " OP_EQUALS " 1'|3");
    e.appendLine(CMD_END);
    e.end();
    EXPECT_PROGRAM_EQ(e);
}
