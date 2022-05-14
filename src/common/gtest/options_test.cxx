/*
 * Copyright (c) 2021-2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

#include "test_config.h"

extern "C" {

#include "../codepage.h"
#include "../cstring.h"
#include "../options.h"
#include "../utility.h"

}

#define OPTIONS_TEST_DIR  TMP_DIR "/OptionsTest"

class OptionsTest : public ::testing::Test {

protected:

    void SetUp() override {
        struct stat st = { 0 };
        if (stat(OPTIONS_TEST_DIR, &st) == -1) {
            mkdir(OPTIONS_TEST_DIR, 0775);
        }
    }

    void TearDown() override {
        (void)! system("rm -rf " OPTIONS_TEST_DIR);
    }

};

static char options_test_buf[1024];

static void write_line_to_buf(const char *line) {
    cstring_cat(options_test_buf, line, sizeof(options_test_buf));
    cstring_cat(options_test_buf, "\n", sizeof(options_test_buf));
}

static void expect_options_have_defaults(Options *options) {
    EXPECT_EQ(0, options->autorun);
    EXPECT_EQ(0, options->base);
    EXPECT_EQ(3, options->break_key);
    EXPECT_EQ(NULL, options->codepage);
    EXPECT_EQ(kSerial, options->console);
    EXPECT_EQ(0x1, options->default_type); // 0x1 = T_NBR
    EXPECT_STREQ("Nano", options->editor);
    EXPECT_EQ(false, options->explicit_type);
    EXPECT_STREQ("FILES\r\n", options->fn_keys[0]);
    EXPECT_STREQ("RUN\r\n", options->fn_keys[1]);
    EXPECT_STREQ("LIST\r\n", options->fn_keys[2]);
    EXPECT_STREQ("EDIT\r\n", options->fn_keys[3]);
    EXPECT_STREQ("AUTOSAVE \"\"\202", options->fn_keys[4]);
    EXPECT_STREQ("XMODEM RECEIVE \"\"\202", options->fn_keys[5]);
    EXPECT_STREQ("XMODEM SEND \"\"\202", options->fn_keys[6]);
    EXPECT_STREQ("EDIT \"\"\202", options->fn_keys[7]);
    EXPECT_STREQ("LIST \"\"\202", options->fn_keys[8]);
    EXPECT_STREQ("RUN \"\"\202", options->fn_keys[9]);
    EXPECT_STREQ("", options->fn_keys[10]);
    EXPECT_STREQ("", options->fn_keys[11]);
    EXPECT_EQ(0, options->height);
    EXPECT_EQ(kTitle, options->list_case);
    EXPECT_EQ(PROG_FLASH_SIZE, options->prog_flash_size);
    EXPECT_EQ(kCharacter, options->resolution);
    EXPECT_STREQ("", options->search_path);
    EXPECT_EQ(4, options->tab);
    EXPECT_EQ(0, options->width);
    EXPECT_EQ(true, options->zboolean);
    EXPECT_EQ(2.71828, options->zfloat);
    EXPECT_EQ(1945, options->zinteger);
    EXPECT_STREQ("wombat", options->zstring);
}

TEST_F(OptionsTest, Init) {
    Options options;
    options_init(&options);

    expect_options_have_defaults(&options);
}

static void given_non_default_options(Options *options) {
    strcpy(options->editor, "Vi");
    options->list_case = kLower;
    strcpy(options->search_path, "/foo/bar");
    options->tab = 8;
    options->zboolean = false;
    options->zfloat = 3.142;
    options->zinteger = 42;
    strcpy(options->zstring, "snafu");
}

TEST_F(OptionsTest, HasDefaultValue) {
    Options options;
    options_init(&options);

    for (const OptionsDefinition *def = options_definitions; def->name; ++def) {
        EXPECT_EQ(true, options_has_default_value(&options, def->id));
    }

    given_non_default_options(&options);
    for (const OptionsDefinition *def = options_definitions; def->name; ++def) {
        switch (def->id) {
            case kOptionListCase:
            case kOptionEditor:
            case kOptionSearchPath:
            case kOptionTab:
            case kOptionZBoolean:
            case kOptionZFloat:
            case kOptionZInteger:
            case kOptionZString:
                EXPECT_EQ(false, options_has_default_value(&options, def->id));
                break;
            default:
                EXPECT_EQ(true, options_has_default_value(&options, def->id));
                break;
        }
    }
}

TEST_F(OptionsTest, Save_GivenAllOptionsAtDefaults) {
    Options options;
    options_init(&options);

    const char *filename = OPTIONS_TEST_DIR "/save_give_all_options_at_defaults";
    EXPECT_EQ(kOk, options_save(&options, filename));

    // Expect an empty file.
    FILE *f = fopen(filename, "r");
    char line[256];
    EXPECT_TRUE(f);
    EXPECT_STREQ(NULL, fgets(line, 256, f));
    EXPECT_TRUE(feof(f));
    fclose(f);
}

// Note only non-default values are saved.
static void expect_saved_content_for_non_default_options(const char *filename) {
    FILE *f = fopen(filename, "r");
    char line[256];
    EXPECT_STREQ(fgets(line, 256, f), "case = \"Lower\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "editor = \"Vi\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "search-path = \"/foo/bar\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "tab = 8\n");
    EXPECT_STREQ(fgets(line, 256, f), "zboolean = false\n");
    EXPECT_STREQ(fgets(line, 256, f), "zfloat = 3.142\n");
    EXPECT_STREQ(fgets(line, 256, f), "zinteger = 42\n");
    EXPECT_STREQ(fgets(line, 256, f), "zstring = \"snafu\"\n");
    EXPECT_FALSE(fgets(line, 256, f));
    EXPECT_TRUE(feof(f));
    fclose(f);
}

TEST_F(OptionsTest, Save_GivenNonDefaultOptions) {
    Options options;
    options_init(&options);
    given_non_default_options(&options);

    const char *filename = OPTIONS_TEST_DIR "/save_given_non_default_options";
    EXPECT_EQ(options_save(&options, filename), 0);
    expect_saved_content_for_non_default_options(filename);
}

TEST_F(OptionsTest, Save_GivenNonDefaultOptions_GivenDirectoryDoesNotExist) {
    Options options;
    options_init(&options);
    given_non_default_options(&options);

    const char *filename = OPTIONS_TEST_DIR "/save_given_dir/myfile.options";
    EXPECT_EQ(kFileNotFound, options_save(&options, filename));
}

TEST_F(OptionsTest, Save_GivenPathIsDirectory) {
    Options options;
    options_init(&options);
    given_non_default_options(&options);
    (void)! system("mkdir " OPTIONS_TEST_DIR "/save_given_path_is_directory");

    const char *filename = OPTIONS_TEST_DIR "/save_given_path_is_directory";
    EXPECT_EQ(kIsADirectory, options_save(&options, filename));
}

TEST_F(OptionsTest, Save_GivenInvalidPath) {
    Options options;
    options_init(&options);
    given_non_default_options(&options);

    const char *filename = OPTIONS_TEST_DIR "/subdir/save_given_path_is_directory";
    EXPECT_EQ(kFileNotFound, options_save(&options, filename));
}

#define SAVE_GIVEN_PATH_IS_EXISTING_READ_ONLY_FILE  OPTIONS_TEST_DIR "/save_given_path_is_existing_read_only_file"

TEST_F(OptionsTest, Save_GivenPathIsExistingReadOnlyFile) {
    Options options;
    options_init(&options);
    given_non_default_options(&options);
    (void)! system("touch " SAVE_GIVEN_PATH_IS_EXISTING_READ_ONLY_FILE);
    (void)! system("chmod 444 " SAVE_GIVEN_PATH_IS_EXISTING_READ_ONLY_FILE);

    const char *filename = SAVE_GIVEN_PATH_IS_EXISTING_READ_ONLY_FILE;
    EXPECT_EQ(kPermissionDenied, options_save(&options, filename));

    (void)! system("chmod 644 " SAVE_GIVEN_PATH_IS_EXISTING_READ_ONLY_FILE);
}

TEST_F(OptionsTest, Load) {
    const char *filename = OPTIONS_TEST_DIR "/load";
    FILE *f = fopen(filename, "w");
    fprintf(f, "case = Upper\n");
    fprintf(f, "editor = Vi\n");
    fprintf(f, "tab = 8\n");
    fprintf(f, "zboolean = true\n");
    fprintf(f, "zfloat = 3.142\n");
    fprintf(f, "zstring = \"foo bar\"\n");
    fclose(f);

    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_load(&options, filename, NULL));
    EXPECT_STREQ(options.editor, "Vi");
    EXPECT_EQ(kUpper, options.list_case);
    EXPECT_EQ(8, options.tab);
    EXPECT_EQ(options.zboolean, true);
    EXPECT_DOUBLE_EQ(options.zfloat, 3.142);
    EXPECT_STREQ(options.zstring, "foo bar");
}

TEST_F(OptionsTest, Load_GivenAdditionalWhitespace) {
    const char *filename = OPTIONS_TEST_DIR "/load_given_additional_whitespace";
    FILE *f = fopen(filename, "w");
    fprintf(f, "tab = 8  \n");                           // Trailing whitespace
    fprintf(f, "  zboolean = true\n");                   // Leading whitespace
    fprintf(f, "zfloat   =   3.142    \n");              // Whitespace around equals
    fprintf(f, "\tzstring\t \t=\t \t\"foo bar\"\t\n\t"); // tab characters everywhere
    fclose(f);

    Options options;
    options_init(&options);

    EXPECT_EQ(options_load(&options, filename, NULL), 0);
    EXPECT_EQ(8, options.tab);
    EXPECT_EQ(options.zboolean, true);
    EXPECT_DOUBLE_EQ(options.zfloat, 3.142);
    EXPECT_STREQ(options.zstring, "foo bar");
}

TEST_F(OptionsTest, Load_GivenEmptyAndWhitespaceOnlyLines) {
    const char *filename = OPTIONS_TEST_DIR "/load_given_empty_and_whitespace_only_lines";
    FILE *f = fopen(filename, "w");
    fprintf(f, "\n");
    fprintf(f, "zboolean = true\n");
    fprintf(f, "    \n");
    fprintf(f, "tab = 8\n");
    fprintf(f, "\r\n");
    fprintf(f, "zfloat = 3.142\n");
    fprintf(f, "  \t  \t \n");
    fprintf(f, "zstring = \"foo bar\"\n");
    fclose(f);

    Options options;
    options_init(&options);

    EXPECT_EQ(options_load(&options, filename, NULL), 0);
    EXPECT_EQ(options.zboolean, true);
    EXPECT_EQ(8, options.tab);
    EXPECT_DOUBLE_EQ(options.zfloat, 3.142);
    EXPECT_STREQ(options.zstring, "foo bar");
}

TEST_F(OptionsTest, Load_GivenHashComments) {
    const char *filename = OPTIONS_TEST_DIR "/load_given_hash_comments";
    FILE *f = fopen(filename, "w");
    fprintf(f, "  # Hello World\n");
    fprintf(f, "zboolean = true # Trailing comment\n");
    fprintf(f, "# Leading comment zboolean = false\n");
    fprintf(f, "tab = 8\n");
    fprintf(f, "zfloat = 3.142\n");
    fprintf(f, "zstring = \"foo bar\"\n");
    fclose(f);

    Options options;
    options_init(&options);

    EXPECT_EQ(options_load(&options, filename, NULL), 0);
    EXPECT_EQ(options.zboolean, true);
    EXPECT_EQ(8, options.tab);
    EXPECT_DOUBLE_EQ(options.zfloat, 3.142);
    EXPECT_STREQ(options.zstring, "foo bar");
}

TEST_F(OptionsTest, Load_GivenSemicolonComments) {
    const char *filename = OPTIONS_TEST_DIR "/load_given_semicolon_comments";
    FILE *f = fopen(filename, "w");
    fprintf(f, "  ; Hello World\n");
    fprintf(f, "zboolean = true ; Trailing comment\n");
    fprintf(f, "; Leading comment zboolean = false\n");
    fprintf(f, "tab = 8\n");
    fprintf(f, "zfloat = 3.142\n");
    fprintf(f, "zstring = \"foo bar\"\n");
    fclose(f);

    Options options;
    options_init(&options);

    EXPECT_EQ(options_load(&options, filename, NULL), 0);
    EXPECT_EQ(options.zboolean, true);
    EXPECT_EQ(8, options.tab);
    EXPECT_DOUBLE_EQ(options.zfloat, 3.142);
    EXPECT_STREQ(options.zstring, "foo bar");
}

TEST_F(OptionsTest, Load_GivenWarnings_AndCallbackProvided) {
    const char *filename = OPTIONS_TEST_DIR "/load_given_warnings_and_callback_provided";
    FILE *f = fopen(filename, "w");
    fprintf(f,
            "foo = true\n"
            "zboolean = 42\n"
            "# comment\n"
            "tab = \"Hello World\"\n"
            "zfloat = true\n"
            "zstring = 3.142\n"
            "search-path = \"/does/not/exist\"\n"
            "bar");
    fclose(f);

    options_test_buf[0] = '\0';
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_load(&options, filename, &write_line_to_buf));
    EXPECT_STREQ(
            "line 1: Unknown option 'foo'.\n"
            "line 2: Invalid value for option 'zboolean'.\n"
            "line 4: Invalid value for option 'tab'.\n"
            "line 5: Invalid value for option 'zfloat'.\n"
            "line 7: No such file or directory for option 'search path'.\n"
            "line 8: Invalid format for option.\n",
            options_test_buf);
}

TEST_F(OptionsTest, Load_GivenWarnings_AndCallbackNotProvided) {
    const char *filename = OPTIONS_TEST_DIR "/load_given_warnings_and_callback_not_provided";
    FILE *f = fopen(filename, "w");
    fprintf(f,
            "foo = true\n"
            "zboolean = 42\n"
            "# comment"
            "tab = \"Hello World\"\n"
            "zfloat = true\n"
            "zstring = 3.142\n"
            "search-path = \"/does/not/exist\"\n"
            "bar");
    fclose(f);

    options_test_buf[0] = '\0';
    Options options;
    options_init(&options);

    EXPECT_EQ(kUnknownOption, options_load(&options, filename, NULL));
    EXPECT_STREQ("", options_test_buf);
}

TEST_F(OptionsTest, LoadSaveRoundtrip) {
    const char *filename = OPTIONS_TEST_DIR "/load_save_roundtrip";
    options_test_buf[0] = '\0';
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_save(&options, filename));
    EXPECT_EQ(kOk, options_load(&options, filename, &write_line_to_buf));
    EXPECT_STREQ("", options_test_buf);

    expect_options_have_defaults(&options);
}

TEST_F(OptionsTest, Load_GivenFileDoesNotExist) {
    const char *filename = OPTIONS_TEST_DIR "/load_given_file_does_not_exist";
    options_test_buf[0] = '\0';
    Options options;
    options_init(&options);

    EXPECT_EQ(kFileNotFound, options_load(&options, filename, &write_line_to_buf));
    EXPECT_STREQ("", options_test_buf);

    expect_options_have_defaults(&options);
}

TEST_F(OptionsTest, Load_GivenDirectory) {
    const char *filename = BIN_DIR;
    options_test_buf[0] = '\0';
    Options options;
    options_init(&options);

    EXPECT_EQ(kIsADirectory, options_load(&options, filename, &write_line_to_buf));
    EXPECT_STREQ("", options_test_buf);

    expect_options_have_defaults(&options);
}

TEST_F(OptionsTest, EncodeString) {
    char encoded[STRINGSIZE];

    EXPECT_EQ(kOk, options_encode_string("Hello World", encoded));
    EXPECT_STREQ("Hello World", encoded);

    EXPECT_EQ(kOk, options_encode_string("\\", encoded));
    EXPECT_STREQ("\\\\", encoded);

    EXPECT_EQ(kOk, options_encode_string("\"", encoded));
    EXPECT_STREQ("\\\"", encoded);

    EXPECT_EQ(kOk, options_encode_string("\r", encoded));
    EXPECT_STREQ("\\r", encoded);

    EXPECT_EQ(kOk, options_encode_string("\n", encoded));
    EXPECT_STREQ("\\n", encoded);

    EXPECT_EQ(kOk, options_encode_string("\x01", encoded));
    EXPECT_STREQ("\\001", encoded);

    EXPECT_EQ(kOk, options_encode_string("\x1f", encoded));
    EXPECT_STREQ("\\037", encoded);

    EXPECT_EQ(kOk, options_encode_string("\x20", encoded));
    EXPECT_STREQ(" ", encoded);

    EXPECT_EQ(kOk, options_encode_string("\x7E", encoded));
    EXPECT_STREQ("~", encoded);

    EXPECT_EQ(kOk, options_encode_string("\x7F", encoded));
    EXPECT_STREQ("\\177", encoded);

    // Unencoded string is STRINGSIZE.
    {
        char encoded[STRINGSIZE] = { 0 };
        char unencoded[STRINGSIZE] = { 0 };
        memset(unencoded, '*', STRINGSIZE - 1);

        EXPECT_EQ(kOk, options_encode_string(unencoded, encoded));
        char expected[STRINGSIZE] = { 0 };
        memset(expected, '*', STRINGSIZE - 1);
        EXPECT_STREQ(expected, encoded);
    }
}

TEST_F(OptionsTest, EncodeString_GivenEncodedStringTooLong) {
    // Unencoded string is longer that STRINGSIZE.
    {
        char encoded[STRINGSIZE] = { 0 };
        char unencoded[STRINGSIZE + 1] = { 0 };
        memset(unencoded, '*', STRINGSIZE);

        EXPECT_EQ(kStringTooLong, options_encode_string(unencoded, encoded));
        char expected[STRINGSIZE] = { 0 };
        memset(expected, '*', STRINGSIZE - 1);
        EXPECT_STREQ(expected, encoded);
    }

    // Unencoded string is STRINGSIZE
    // but contains 1 character that will encode to 2 characters.
    char replace_with_2[] = { '\\', '\"', '\r', '\n', '\0' };
    for (char *p = replace_with_2; *p  != '\0'; ++p) {
        char encoded[STRINGSIZE] = { 0 };
        char unencoded[STRINGSIZE] = { 0 };
        memset(unencoded, '*', STRINGSIZE - 2);
        unencoded[STRINGSIZE - 2] = *p;

        EXPECT_EQ(kStringTooLong, options_encode_string(unencoded, encoded));
        char expected[STRINGSIZE] = { 0 };
        memset(expected, '*', STRINGSIZE - 2);
        EXPECT_STREQ(expected, encoded);
    }

    // Unencoded string is 3 characters shorter than STRINGSIZE,
    // but contains 1 character that will encode to 4 characters.
    char replace_with_4[] = { '\x01', '\x1F', '\x7F', '\xFF', '\0' };
    for (char *p = replace_with_4; *p  != '\0'; ++p) {
        char encoded[STRINGSIZE] = { 0 };
        char unencoded[STRINGSIZE] = { 0 };
        memset(unencoded, '*', STRINGSIZE - 4);
        unencoded[STRINGSIZE - 4] = *p;

        EXPECT_EQ(kStringTooLong, options_encode_string(unencoded, encoded));
        char expected[STRINGSIZE] = { 0 };
        memset(expected, '*', STRINGSIZE - 4);
        EXPECT_STREQ(expected, encoded);
    }
}

TEST_F(OptionsTest, DecodeString) {
    char decoded[STRINGSIZE];

    EXPECT_EQ(kOk, options_decode_string("Hello World", decoded));
    EXPECT_STREQ("Hello World", decoded);

    EXPECT_EQ(kOk, options_decode_string("\\\\", decoded));
    EXPECT_STREQ("\\", decoded);

    EXPECT_EQ(kOk, options_decode_string("\\\"", decoded));
    EXPECT_STREQ("\"", decoded);

    EXPECT_EQ(kOk, options_decode_string("\\r", decoded));
    EXPECT_STREQ("\r", decoded);

    EXPECT_EQ(kOk, options_decode_string("\\n", decoded));
    EXPECT_STREQ("\n", decoded);

    EXPECT_EQ(kOk, options_decode_string("\\001", decoded));
    EXPECT_STREQ("\x01", decoded);

    EXPECT_EQ(kOk, options_decode_string("\\037", decoded));
    EXPECT_STREQ("\x1f", decoded);

    EXPECT_EQ(kOk, options_decode_string(" ", decoded));
    EXPECT_STREQ("\x20", decoded);

    EXPECT_EQ(kOk, options_decode_string("~", decoded));
    EXPECT_STREQ("\x7E", decoded);

    EXPECT_EQ(kOk, options_decode_string("\\177", decoded));
    EXPECT_STREQ("\x7F", decoded);

    // Encoded string is STRINGSIZE.
    {
        char encoded[STRINGSIZE] = { 0 };
        memset(encoded, '*', STRINGSIZE - 1);
        char decoded[STRINGSIZE] = { 0 };

        EXPECT_EQ(kOk, options_decode_string(encoded, decoded));
        char expected[STRINGSIZE] = { 0 };
        memset(expected, '*', STRINGSIZE - 1);
        EXPECT_STREQ(expected, decoded);
    }
}

TEST_F(OptionsTest, DecodeString_GivenDecodedStringTooLong) {
    // Encoded string is longer than STRINGSIZE.
    {
        char encoded[STRINGSIZE + 1] = { 0 };
        memset(encoded, '*', STRINGSIZE);
        char decoded[STRINGSIZE] = { 0 };

        EXPECT_EQ(kStringTooLong, options_decode_string(encoded, decoded));
        char expected[STRINGSIZE] = { 0 };
        memset(expected, '*', STRINGSIZE - 1);
        EXPECT_STREQ(expected, decoded);
    }

    // Encoded string is 1 character longer than STRINGSIZE,
    // but contains 2 characters that will decode to 1 character.
    {
        char encoded[STRINGSIZE + 1] = { 0 };
        memset(encoded, '*', STRINGSIZE - 1); // 0..253 = 254 stars
        encoded[STRINGSIZE - 2] = '\\';       // 254    = backslash
        encoded[STRINGSIZE - 1] = '\\';       // 255    = backslash
        char decoded[STRINGSIZE] = { 0 };

        EXPECT_EQ(kOk, options_decode_string(encoded, decoded));
        char expected[STRINGSIZE] = { 0 };
        memset(expected, '*', STRINGSIZE - 2); // 0..253 = 254 stars
        expected[STRINGSIZE - 2] = '\\';       // 254    = backslash
        EXPECT_STREQ(expected, decoded);
    }

    // Encoded string is 3 characters longer than STRINGSIZE,
    // but contains 4 characters that will decode to 1 character.
    {
        char encoded[STRINGSIZE + 4] = { 0 };
        memset(encoded, '*', STRINGSIZE - 1); // 0..253 = 254 stars
        encoded[STRINGSIZE - 2] = '\\';       // 254    = backslash
        encoded[STRINGSIZE - 1] = '0';        // 255    = backslash
        encoded[STRINGSIZE + 0] = '0';        // 256    = backslash
        encoded[STRINGSIZE + 1] = '1';        // 257    = backslash
        char decoded[STRINGSIZE] = { 0 };

        EXPECT_EQ(kOk, options_decode_string(encoded, decoded));
        char expected[STRINGSIZE] = { 0 };
        memset(expected, '*', STRINGSIZE - 2); // 0..253 = 254 stars
        expected[STRINGSIZE - 2] = '\001';     // 254    = 0x01
        EXPECT_STREQ(expected, decoded);
    }
}

TEST_F(OptionsTest, GetDefinition) {
    OptionsDefinition *lookup;
    for (const OptionsDefinition *def = options_definitions; def->name; ++def) {
        EXPECT_EQ(kOk, options_get_definition(def->name, &lookup));
        EXPECT_EQ(def, lookup);
    }

    EXPECT_EQ(kOk, options_get_definition("seaRCH paTH", &lookup));
    EXPECT_EQ(kOptionSearchPath, lookup->id);

    EXPECT_EQ(kUnknownOption, options_get_definition("unknown", &lookup));
}

TEST_F(OptionsTest, GetDisplayValue) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionBase, svalue));
    EXPECT_STREQ("0", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionBreakKey, svalue));
    EXPECT_STREQ("3", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionListCase, svalue));
    EXPECT_STREQ("Title", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionCodePage, svalue));
    EXPECT_STREQ("None", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionConsole, svalue));
    EXPECT_STREQ("Serial", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionDefaultType, svalue));
    EXPECT_STREQ("Float", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionEditor, svalue));
    EXPECT_STREQ("Nano", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionExplicitType, svalue));
    EXPECT_STREQ("Off", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF1, svalue));
    EXPECT_STREQ("FILES<crlf>", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF5, svalue));
    EXPECT_STREQ("AUTOSAVE \"\"<left>", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF11, svalue));
    EXPECT_STREQ("<unset>", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionResolution, svalue));
    EXPECT_STREQ("Character", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionSearchPath, svalue));
    EXPECT_STREQ("<unset>", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionTab, svalue));
    EXPECT_STREQ("4", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionZBoolean, svalue));
    EXPECT_STREQ("On", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionZFloat, svalue));
    EXPECT_STREQ("2.71828", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionZInteger, svalue));
    EXPECT_STREQ("1945", svalue);

    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionZString, svalue));
    EXPECT_STREQ("wombat", svalue);
}

TEST_F(OptionsTest, GetDisplayValue_GivenNonAscii) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    strcpy(options.fn_keys[10], "foo\r\n");
    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF11, svalue));
    EXPECT_STREQ("foo<crlf>", svalue);

    strcpy(options.fn_keys[10], "foo\r");
    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF11, svalue));
    EXPECT_STREQ("foo<cr>", svalue);

    strcpy(options.fn_keys[10], "foo\n");
    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF11, svalue));
    EXPECT_STREQ("foo<lf>", svalue);

    strcpy(options.fn_keys[10], "foo\x82 bar");
    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF11, svalue));
    EXPECT_STREQ("foo<left> bar", svalue);

    strcpy(options.fn_keys[10], "  foo  bar  ");
    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF11, svalue));
    EXPECT_STREQ("  foo  bar<20><20>", svalue);

    strcpy(options.fn_keys[10], "\x10 foo\xFD");
    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF11, svalue));
    EXPECT_STREQ("<10> foo<fd>", svalue);
}

TEST_F(OptionsTest, GetDisplayValue_GivenTooLong) {
    Options options;
    options_init(&options);
    char expected[STRINGSIZE];
    char in[STRINGSIZE];
    char out[STRINGSIZE];

    memset(in, '\0', STRINGSIZE);
    memset(in, '*', STRINGSIZE - 1);
    strcpy(options.fn_keys[10], in);
    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF11, out));
    EXPECT_EQ(STRINGSIZE - 1, strlen(out));
    strcpy(expected, in);
    EXPECT_STREQ(expected, out);

    memset(in, '\0', STRINGSIZE);
    memset(in, '*', STRINGSIZE - 5);
    strcat(in + STRINGSIZE - 5, "\r");
    strcpy(options.fn_keys[10], in);
    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF11, out));
    EXPECT_EQ(STRINGSIZE - 1, strlen(out));
    memset(expected, '\0', STRINGSIZE);
    memset(expected, '*', STRINGSIZE - 5);
    strcpy(expected + STRINGSIZE - 5, "<cr>");
    EXPECT_STREQ(expected, out);

    memset(in, '\0', STRINGSIZE);
    memset(in, '*', STRINGSIZE - 4);
    strcat(in + STRINGSIZE - 4, "\r");
    strcpy(options.fn_keys[10], in);
    EXPECT_EQ(kOk, options_get_display_value(&options, kOptionF11, out));
    EXPECT_EQ(STRINGSIZE - 4, strlen(out));
    memset(expected, '\0', STRINGSIZE);
    memset(expected, '*', STRINGSIZE - 4);
    EXPECT_STREQ(expected, out);
}

TEST_F(OptionsTest, GetFloatValue_ForZFloat) {
    Options options;
    options_init(&options);
    MMFLOAT fvalue = 0.0;

    EXPECT_EQ(kOk, options_get_float_value(&options, kOptionZFloat, &fvalue));
    EXPECT_EQ(2.71828, fvalue);
}

TEST_F(OptionsTest, GetFloatValue_ForNonFloat) {
    Options options;
    options_init(&options);
    MMFLOAT fvalue = 0.0;

    EXPECT_EQ(kInternalFault, options_get_float_value(&options, kOptionZInteger, &fvalue));
    EXPECT_EQ(0.0, fvalue);
    EXPECT_EQ(kInternalFault, options_get_float_value(&options, kOptionZString, &fvalue));
    EXPECT_EQ(0.0, fvalue);
}

TEST_F(OptionsTest, GetIntegerValue_ForBase) {
    Options options;
    options_init(&options);
    MMINTEGER ivalue = 0;

    options.base = 0;
    EXPECT_EQ(kOk, options_get_integer_value(&options, kOptionBase, &ivalue));
    EXPECT_EQ(0, ivalue);

    options.base = 1;
    EXPECT_EQ(kOk, options_get_integer_value(&options, kOptionBase, &ivalue));
    EXPECT_EQ(1, ivalue);
}

TEST_F(OptionsTest, GetIntegerValue_ForBreakKey) {
    Options options;
    options_init(&options);
    MMINTEGER ivalue = 0;

    options.break_key = 3;
    EXPECT_EQ(kOk, options_get_integer_value(&options, kOptionBreakKey, &ivalue));
    EXPECT_EQ(3, ivalue);

    options.break_key = 4;
    EXPECT_EQ(kOk, options_get_integer_value(&options, kOptionBreakKey, &ivalue));
    EXPECT_EQ(4, ivalue);
}

TEST_F(OptionsTest, GetIntegerValue_ForTab) {
    Options options;
    options_init(&options);
    MMINTEGER ivalue = 0;

    options.tab = 2;
    EXPECT_EQ(kOk, options_get_integer_value(&options, kOptionTab, &ivalue));
    EXPECT_EQ(2, ivalue);

    options.tab = 8;
    EXPECT_EQ(kOk, options_get_integer_value(&options, kOptionTab, &ivalue));
    EXPECT_EQ(8, ivalue);
}

TEST_F(OptionsTest, GetIntegerValue_ForZBoolean) {
    Options options;
    options_init(&options);
    MMINTEGER ivalue = 0;

    EXPECT_EQ(kOk, options_get_integer_value(&options, kOptionZBoolean, &ivalue));
    EXPECT_EQ(true, (bool) ivalue);
}

TEST_F(OptionsTest, GetIntegerValue_ForZInteger) {
    Options options;
    options_init(&options);
    MMINTEGER ivalue = 0;

    EXPECT_EQ(kOk, options_get_integer_value(&options, kOptionZInteger, &ivalue));
    EXPECT_EQ(1945, ivalue);
}

TEST_F(OptionsTest, GetIntegerValue_ForNonInteger) {
    Options options;
    options_init(&options);
    MMINTEGER ivalue = 0;

    EXPECT_EQ(kInternalFault, options_get_integer_value(&options, kOptionZFloat, &ivalue));
    EXPECT_EQ(0, ivalue);
    EXPECT_EQ(kInternalFault, options_get_integer_value(&options, kOptionZString, &ivalue));
    EXPECT_EQ(0, ivalue);
}

TEST_F(OptionsTest, GetStringValue_ForBase) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE] = { 0 };

    options.base = 0;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionBase, svalue));
    EXPECT_STREQ("0", svalue);

    options.base = 1;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionBase, svalue));
    EXPECT_STREQ("1", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForBreakKey) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE] = { 0 };

    options.break_key = 3;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionBreakKey, svalue));
    EXPECT_STREQ("3", svalue);

    options.break_key = 4;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionBreakKey, svalue));
    EXPECT_STREQ("4", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForCodePage) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    for (const NameOrdinalPair *entry = codepage_data_to_ordinal_map; entry->name; ++entry) {
        options.codepage = entry->name;
        EXPECT_EQ(kOk, options_get_string_value(&options, kOptionCodePage, svalue));
        EXPECT_STREQ(codepage_name_to_ordinal_map[entry->ordinal].name, svalue);
    }

    char tmp[] = "foo";
    options.codepage = tmp; // Invalid value.
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionCodePage, svalue));
    EXPECT_STREQ("???", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForConsole) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    options.console = kBoth;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionConsole, svalue));
    EXPECT_STREQ("Both", svalue);

    options.console = kScreen;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionConsole, svalue));
    EXPECT_STREQ("Screen", svalue);

    options.console = kSerial;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionConsole, svalue));
    EXPECT_STREQ("Serial", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForDefaultType) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    options.default_type = 0x00; // T_NOTYPE;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionDefaultType, svalue));
    EXPECT_STREQ("None", svalue);

    options.default_type = 0x01; // T_NBR
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionDefaultType, svalue));
    EXPECT_STREQ("Float", svalue);

    options.default_type = 0x02; // T_STR;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionDefaultType, svalue));
    EXPECT_STREQ("String", svalue);

    options.default_type = 0x04; // T_INT
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionDefaultType, svalue));
    EXPECT_STREQ("Integer", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForEditor) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    for (OptionsEditor *editor = options_editors; editor->name; ++editor) {
        strcpy(options.editor, editor->name);
        EXPECT_EQ(kOk, options_get_string_value(&options, kOptionEditor, svalue));
        EXPECT_STREQ(editor->name, svalue);
    }

    strcpy(options.editor, "myeditor ${file}:${line}");
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionEditor, svalue));
    EXPECT_STREQ("myeditor ${file}:${line}", svalue);

    strcpy(options.editor, "");
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionEditor, svalue));
    EXPECT_STREQ("", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForExplicitType) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    options.explicit_type = 0;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionExplicitType, svalue));
    EXPECT_STREQ("Off", svalue);

    options.explicit_type = 1;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionExplicitType, svalue));
    EXPECT_STREQ("On", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForFunctionKeys) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE] = { 0 };

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF1, svalue));
    EXPECT_STREQ("FILES\r\n", svalue);

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF2, svalue));
    EXPECT_STREQ("RUN\r\n", svalue);

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF3, svalue));
    EXPECT_STREQ("LIST\r\n", svalue);

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF4, svalue));
    EXPECT_STREQ("EDIT\r\n", svalue);

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF5, svalue));
    EXPECT_STREQ("AUTOSAVE \"\"\202", svalue);

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF6, svalue));
    EXPECT_STREQ("XMODEM RECEIVE \"\"\202", svalue);

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF7, svalue));
    EXPECT_STREQ("XMODEM SEND \"\"\202", svalue);

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF8, svalue));
    EXPECT_STREQ("EDIT \"\"\202", svalue);

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF9, svalue));
    EXPECT_STREQ("LIST \"\"\202", svalue);

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF10, svalue));
    EXPECT_STREQ("RUN \"\"\202", svalue);

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF11, svalue));
    EXPECT_STREQ("", svalue);

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionF12, svalue));
    EXPECT_STREQ("", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForListCase) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    options.list_case = kTitle;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionListCase, svalue));
    EXPECT_STREQ("Title", svalue);

    options.list_case = kLower;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionListCase, svalue));
    EXPECT_STREQ("Lower", svalue);

    options.list_case = kUpper;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionListCase, svalue));
    EXPECT_STREQ("Upper", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForResolution) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    options.resolution = kCharacter;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionResolution, svalue));
    EXPECT_STREQ("Character", svalue);

    options.resolution = kPixel;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionResolution, svalue));
    EXPECT_STREQ("Pixel", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForSearchPath) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    strcpy(options.search_path, "");
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionSearchPath, svalue));
    EXPECT_STREQ("", svalue);

    strcpy(options.search_path, HOME_DIR "/foo");
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionSearchPath, svalue));
    EXPECT_STREQ(HOME_DIR "/foo", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForTab) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE] = { 0 };

    options.tab = 2;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionTab, svalue));
    EXPECT_STREQ("2", svalue);

    options.tab = 8;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionTab, svalue));
    EXPECT_STREQ("8", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForZBoolean) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE] = { 0 };

    options.zboolean = true;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionZBoolean, svalue));
    EXPECT_STREQ("On", svalue);

    options.zboolean = false;
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionZBoolean, svalue));
    EXPECT_STREQ("Off", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForZFloat) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE] = { 0 };

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionZFloat, svalue));
    EXPECT_STREQ("2.71828", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForZInteger) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE] = { 0 };

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionZInteger, svalue));
    EXPECT_STREQ("1945", svalue);
}

TEST_F(OptionsTest, GetStringValue_ForZString) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE] = { 0 };

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionZString, svalue));
    EXPECT_STREQ("wombat", svalue);
}

TEST_F(OptionsTest, SetFloatValue_ForZFloat) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_float_value(&options, kOptionZFloat, 1.2345));
    EXPECT_EQ(1.2345, options.zfloat);
}

TEST_F(OptionsTest, SetFloatValue_ForNonFloat) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kInternalFault, options_set_float_value(&options, kOptionZInteger, 1.2345));
    EXPECT_EQ(kInternalFault, options_set_float_value(&options, kOptionZString, 1.2345));
}

TEST_F(OptionsTest, SetIntegerValue_ForBase) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_integer_value(&options, kOptionBase, 0));
    EXPECT_EQ(0, options.base);

    EXPECT_EQ(kOk, options_set_integer_value(&options, kOptionBase, 1));
    EXPECT_EQ(1, options.base);

    EXPECT_EQ(kInvalidValue, options_set_integer_value(&options, kOptionBase, 2));
}

TEST_F(OptionsTest, SetIntegerValue_ForBreakKey) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_integer_value(&options, kOptionBreakKey, 5));
    EXPECT_EQ(5, options.break_key);

    EXPECT_EQ(kInvalidValue, options_set_integer_value(&options, kOptionBreakKey, 0));
    EXPECT_EQ(kInvalidValue, options_set_integer_value(&options, kOptionBreakKey, 256));
}

TEST_F(OptionsTest, SetIntegerValue_ForTab) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_integer_value(&options, kOptionTab, 2));
    EXPECT_EQ(2, options.tab);

    EXPECT_EQ(kOk, options_set_integer_value(&options, kOptionTab, 8));
    EXPECT_EQ(8, options.tab);

    EXPECT_EQ(kInvalidValue, options_set_integer_value(&options, kOptionTab, 3));
}

TEST_F(OptionsTest, SetIntegerValue_ForZBoolean) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_integer_value(&options, kOptionZBoolean, 0));
    EXPECT_EQ(false, options.zboolean);

    EXPECT_EQ(kOk, options_set_integer_value(&options, kOptionZBoolean, 1));
    EXPECT_EQ(true, options.zboolean);

    EXPECT_EQ(kInvalidValue, options_set_integer_value(&options, kOptionZBoolean, 2));
}


TEST_F(OptionsTest, SetIntegerValue_ForZInteger) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_integer_value(&options, kOptionZInteger, 43));
    EXPECT_EQ(43, options.zinteger);
}

TEST_F(OptionsTest, SetIntegerValue_ForNonInteger) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kInternalFault, options_set_integer_value(&options, kOptionZFloat, 42));
    EXPECT_EQ(kInternalFault, options_set_integer_value(&options, kOptionZString, 42));
}

TEST_F(OptionsTest, SetStringValue_ForBase) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionBase, "0"));
    EXPECT_EQ(0, options.base);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionBase, "1"));
    EXPECT_EQ(1, options.base);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionBase, "2"));
    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionBase, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForBreakKey) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionBreakKey, "42"));
    EXPECT_EQ(42, options.break_key);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionBreakKey, "0"));
    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionBreakKey, "256"));
    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionBreakKey, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForCodePage) {
    Options options;
    options_init(&options);

    for (const NameOrdinalPair *entry = codepage_name_to_ordinal_map; entry->name; ++entry) {
        EXPECT_EQ(kOk, options_set_string_value(&options, kOptionCodePage, entry->name));
        EXPECT_EQ(codepage_data_to_ordinal_map[entry->ordinal].name, options.codepage);
    }

    // Test case-insensitivity.
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionCodePage, "mMb4L"));
    EXPECT_EQ(codepage_data_to_ordinal_map[kCodepageMMB4L].name, options.codepage);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionCodePage, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForConsole) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionConsole, "Both"));
    EXPECT_EQ(kBoth, options.console);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionConsole, "Screen"));
    EXPECT_EQ(kScreen, options.console);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionConsole, "Serial"));
    EXPECT_EQ(kSerial, options.console);

    // Test case-insensitivity.
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionConsole, "boTH"));
    EXPECT_EQ(kBoth, options.console);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionConsole, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForDefaultType) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionDefaultType, "None"));
    EXPECT_EQ(0x00, options.default_type);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionDefaultType, "Float"));
    EXPECT_EQ(0x01, options.default_type);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionDefaultType, "String"));
    EXPECT_EQ(0x02, options.default_type);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionDefaultType, "Integer"));
    EXPECT_EQ(0x04, options.default_type);

    // Test case-insensitivity.
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionDefaultType, "STRing"));
    EXPECT_EQ(0x02, options.default_type);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionDefaultType, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForEditor) {
    Options options;
    options_init(&options);

    // Test standard editors.
    for (OptionsEditor *editor = options_editors; editor->name; ++editor) {
        EXPECT_EQ(kOk, options_set_string_value(&options, kOptionEditor, editor->name));
        EXPECT_STREQ(editor->name, options.editor);
    }

    // Test custom editor.
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionEditor, "myeditor ${file}:${line}"));
    EXPECT_STREQ("myeditor ${file}:${line}", options.editor);

    // Test case-insensitivity.
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionEditor, "vsCODE"));
    EXPECT_STREQ("VSCode", options.editor);

    // Empty string.
    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionEditor, ""));

    // String at the size limit.
    char svalue[STRINGSIZE + 1] = { 0 };
    memset(svalue, '*', STRINGSIZE - 1);
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionEditor, svalue));
    EXPECT_STREQ(svalue, options.editor);

    // String just beyond the size limit.
    memset(svalue, '*', STRINGSIZE);
    EXPECT_EQ(kStringTooLong, options_set_string_value(&options, kOptionEditor, svalue));
}

TEST_F(OptionsTest, SetStringValue_ForExplicitType) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionExplicitType, "0"));
    EXPECT_EQ(false, options.explicit_type);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionExplicitType, "1"));
    EXPECT_EQ(true, options.explicit_type);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionExplicitType, "false"));
    EXPECT_EQ(false, options.explicit_type);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionExplicitType, "true"));
    EXPECT_EQ(true, options.explicit_type);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionExplicitType, "Off"));
    EXPECT_EQ(false, options.explicit_type);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionExplicitType, "On"));
    EXPECT_EQ(true, options.explicit_type);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionExplicitType, "OFF"));
    EXPECT_EQ(false, options.explicit_type);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionExplicitType, "TRUE"));
    EXPECT_EQ(true, options.explicit_type);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionExplicitType, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForFunctionKeys) {
    Options options;
    options_init(&options);

    for (int f = 1; f <= 12; ++f) {
        EXPECT_EQ(kOk, options_set_string_value(&options, (OptionsId) (kOptionF1 + f - 1), "wombat"));
        EXPECT_STREQ("wombat", options.fn_keys[f - 1]);
    }

    // Empty string.
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionF1, ""));
    EXPECT_STREQ("", options.fn_keys[0]);

    // String at the size limit.
    char svalue[STRINGSIZE + 1] = { 0 };
    memset(svalue, '*', STRINGSIZE - 1);
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionF1, svalue));
    EXPECT_STREQ(svalue, options.fn_keys[0]);

    // String just beyond the size limit.
    memset(svalue, '*', STRINGSIZE);
    EXPECT_EQ(kStringTooLong, options_set_string_value(&options, kOptionF1, svalue));
}

TEST_F(OptionsTest, SetStringValue_ForListCase) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionListCase, "Title"));
    EXPECT_EQ(kTitle, options.list_case);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionListCase, "Lower"));
    EXPECT_EQ(kLower, options.list_case);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionListCase, "Upper"));
    EXPECT_EQ(kUpper, options.list_case);

    // Test case-insensitivity.
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionListCase, "LOWer"));
    EXPECT_EQ(kLower, options.list_case);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionListCase, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForResolution) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionResolution, "Character"));
    EXPECT_EQ(kCharacter, options.resolution);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionResolution, "Pixel"));
    EXPECT_EQ(kPixel, options.resolution);

    // Test case-insensitivity.
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionResolution, "CHARacter"));
    EXPECT_EQ(kCharacter, options.resolution);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionResolution, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForSearchPath) {
    Options options;
    options_init(&options);

    // Empty path.
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionSearchPath, ""));
    EXPECT_STREQ("", options.search_path);

    // Path to a directory that exists.
    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionSearchPath, BIN_DIR));
    EXPECT_STREQ(BIN_DIR, options.search_path);

    // Path to a file that exists.
    EXPECT_EQ(
            kNotADirectory,
            options_set_string_value(&options, kOptionSearchPath, BIN_DIR "/cp"));

    // Path that does not exist.
    EXPECT_EQ(
            kFileNotFound,
            options_set_string_value(&options, kOptionSearchPath, "/does/not/exist"));

    // Path that is too long.
    char svalue[STRINGSIZE + 1] = { 0 };
    svalue[0] = '/';
    memset(svalue + 1, 'a', STRINGSIZE - 1);
    EXPECT_EQ(
            kFilenameTooLong,
            options_set_string_value(&options, kOptionSearchPath, svalue));
}

TEST_F(OptionsTest, SetStringValue_ForTab) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionTab, "2"));
    EXPECT_EQ(2, options.tab);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionTab, "4"));
    EXPECT_EQ(4, options.tab);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionTab, "8"));
    EXPECT_EQ(8, options.tab);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionTab, "3"));
    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionTab, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForZBoolean) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZBoolean, "false"));
    EXPECT_EQ(false, options.zboolean);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZBoolean, "true"));
    EXPECT_EQ(true, options.zboolean);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZBoolean, "Off"));
    EXPECT_EQ(false, options.zboolean);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZBoolean, "On"));
    EXPECT_EQ(true, options.zboolean);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZBoolean, "0"));
    EXPECT_EQ(false, options.zboolean);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZBoolean, "1"));
    EXPECT_EQ(true, options.zboolean);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZBoolean, "FALSE"));
    EXPECT_EQ(false, options.zboolean);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZBoolean, "ON"));
    EXPECT_EQ(true, options.zboolean);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionZBoolean, "2"));
    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionZBoolean, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForZFloat) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZFloat, "1.234"));
    EXPECT_EQ(1.234, options.zfloat);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZFloat, "-1.51e17"));
    EXPECT_EQ(-1.51e17, options.zfloat);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZFloat, "42"));
    EXPECT_EQ(42.0, options.zfloat);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionZFloat, "true"));
    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionZFloat, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForZInteger) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZInteger, "99"));
    EXPECT_EQ(99, options.zinteger);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZInteger, "-123456"));
    EXPECT_EQ(-123456, options.zinteger);

    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionZInteger, "1.234"));
    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionZInteger, "true"));
    EXPECT_EQ(kInvalidValue, options_set_string_value(&options, kOptionZInteger, "wombat"));
}

TEST_F(OptionsTest, SetStringValue_ForZString) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZString, ""));
    EXPECT_STREQ("", options.zstring);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZString, "wombat"));
    EXPECT_STREQ("wombat", options.zstring);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZString, "\"foo\""));
    EXPECT_STREQ("\"foo\"", options.zstring);

    EXPECT_EQ(kOk, options_set_string_value(&options, kOptionZString, "\r\n"));
    EXPECT_STREQ("\r\n", options.zstring);
}
