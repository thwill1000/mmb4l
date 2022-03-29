#include <gtest/gtest.h>

extern "C" {

#include "../codepage.h"
#include "../cstring.h"
#include "../options.h"
#include "../utility.h"

}

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

TEST(OptionsTest, Init) {
    Options options;
    options_init(&options);

    expect_options_have_defaults(&options);
}

static void expect_saved_content(const char *filename) {
    FILE *f = fopen(filename, "r");
    char line[256];
    EXPECT_STREQ(fgets(line, 256, f), "editor = Vi\n");
    EXPECT_STREQ(fgets(line, 256, f), "f1 = \"FILES\\r\\n\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "f2 = \"RUN\\r\\n\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "f3 = \"LIST\\r\\n\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "f4 = \"EDIT\\r\\n\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "f5 = \"AUTOSAVE \\\"\\\"\\202\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "f6 = \"XMODEM RECEIVE \\\"\\\"\\202\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "f7 = \"XMODEM SEND \\\"\\\"\\202\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "f8 = \"EDIT \\\"\\\"\\202\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "f9 = \"LIST \\\"\\\"\\202\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "f10 = \"RUN \\\"\\\"\\202\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "f11 = \"\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "f12 = \"\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "listcase = Lower\n");
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

TEST(OptionsTest, Save) {
    Options options;
    options_init(&options);
    strcpy(options.editor, "Vi");
    options.list_case = kLower;
    strcpy(options.search_path, "/foo/bar");
    options.tab = 8;
    options.zboolean = false;
    options.zfloat = 3.142;
    options.zinteger = 42;
    strcpy(options.zstring, "snafu");

    const char *filename = "/tmp/options_test_save";
    EXPECT_EQ(options_save(&options, filename), 0);
    expect_saved_content(filename);
}

TEST(OptionsTest, Save_GivenDirectoryDoesNotExist) {
    Options options;
    options_init(&options);
    strcpy(options.editor, "Vi");
    options.list_case = kLower;
    strcpy(options.search_path, "/foo/bar");
    options.tab = 8;
    options.zboolean = false;
    options.zfloat = 3.142;
    options.zinteger = 42;
    strcpy(options.zstring, "snafu");

    const char *filename = "/tmp/options_test_save_dir/myfile.options";
    const char *directory = "/tmp/options_test_save_dir";
    remove(filename);
    remove(directory);

    EXPECT_EQ(0, options_save(&options, filename));
    expect_saved_content(filename);

    if (FAILED(remove(filename))) {
        perror("Save_GivenDirectoryDoesNotExist");
    }
    if (FAILED(remove(directory))) {
        perror("Save_GivenDirectoryDoesNotExist");
    }
}

TEST(OptionsTest, Load) {
    const char *filename = "/tmp/options_test_load";
    FILE *f = fopen(filename, "w");
    fprintf(f, "editor = Vi\n");
    fprintf(f, "listcase = Upper\n");
    fprintf(f, "tab = 8\n");
    fprintf(f, "zboolean = true\n");
    fprintf(f, "zfloat = 3.142\n");
    fprintf(f, "zstring = \"foo bar\"\n");
    fclose(f);

    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_load(&options, filename, NULL));
    EXPECT_STREQ(options.editor, "Vi");
    EXPECT_EQ(2, options.list_case);
    EXPECT_EQ(8, options.tab);
    EXPECT_EQ(options.zboolean, true);
    EXPECT_DOUBLE_EQ(options.zfloat, 3.142);
    EXPECT_STREQ(options.zstring, "foo bar");
}

TEST(OptionsTest, Load_GivenAdditionalWhitespace) {
    const char *filename = "/tmp/options_test_load";
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

TEST(OptionsTest, Load_GivenEmptyAndWhitespaceOnlyLines) {
    const char *filename = "/tmp/options_test_load";
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

TEST(OptionsTest, Load_GivenHashComments) {
    const char *filename = "/tmp/options_test_load";
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

TEST(OptionsTest, Load_GivenSemicolonComments) {
    const char *filename = "/tmp/options_test_load";
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

TEST(OptionsTest, Load_GivenWarnings_AndCallbackProvided) {
    const char *filename = "/tmp/options_test_load";
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

    EXPECT_EQ(0, options_load(&options, filename, &write_line_to_buf));
    EXPECT_STREQ(
            "line 1: unknown option 'foo'.\n"
            "line 2: invalid boolean value for option 'zboolean'.\n"
            "line 4: invalid integer value for option 'tab'.\n"
            "line 5: invalid float value for option 'zfloat'.\n"
            "line 6: invalid string value for option 'zstring'.\n"
            "line 7: file or directory not found for option 'search-path'.\n"
            "line 8: invalid option format.\n",
            options_test_buf);
}

TEST(OptionsTest, Load_GivenWarnings_AndCallbackNotProvided) {
    const char *filename = "/tmp/options_test_load";
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

    EXPECT_EQ(0, options_load(&options, filename, NULL));
    EXPECT_STREQ("", options_test_buf);
}

TEST(OptionsTest, Load_GivenLoadSaveRoundtrip) {
    const char *filename = "/tmp/options_test_load_save_roundtrip";
    options_test_buf[0] = '\0';
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_save(&options, filename));
    EXPECT_EQ(kOk, options_load(&options, filename, &write_line_to_buf));
    EXPECT_STREQ("", options_test_buf);

    expect_options_have_defaults(&options);
}

TEST(OptionsTest, EditorToString) {
    char buf[STRINGSIZE];
    options_editor_to_string("Default", buf);
    EXPECT_STREQ("Default", buf);

    options_editor_to_string("Geany", buf);
    EXPECT_STREQ("Geany", buf);

    options_editor_to_string("Gedit", buf);
    EXPECT_STREQ("Gedit", buf);

    options_editor_to_string("Leafpad", buf);
    EXPECT_STREQ("Leafpad", buf);

    options_editor_to_string("Nano", buf);
    EXPECT_STREQ("Nano", buf);

    options_editor_to_string("Vi", buf);
    EXPECT_STREQ("Vi", buf);

    options_editor_to_string("Vim", buf);
    EXPECT_STREQ("Vim", buf);

    options_editor_to_string("VSCode", buf);
    EXPECT_STREQ("VSCode", buf);

    options_editor_to_string("Xed", buf);
    EXPECT_STREQ("Xed", buf);

    options_editor_to_string("\"code -g ${file}:${line}\"", buf);
    EXPECT_STREQ("\"code -g ${file}:${line}\"", buf);

    // This is actually an invalid value, it should be quoted, but
    // options_editor_to_string() doesn't care.
    options_editor_to_string("code -g ${file}:${line}", buf);
    EXPECT_STREQ("code -g ${file}:${line}", buf);
}

TEST(OptionsTest, Set_GivenEditor) {
    Options options;
    options_init(&options);
    strcpy(options.editor, "");

    EXPECT_EQ(kOk, options_set(&options, "editor", "atom"));
    EXPECT_STREQ("Atom", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "code"));
    EXPECT_STREQ("VSCode", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "default"));
    EXPECT_STREQ("Nano", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "GEaNY"));
    EXPECT_STREQ("Geany", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "GEDIT"));
    EXPECT_STREQ("Gedit", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "leafPAD"));
    EXPECT_STREQ("Leafpad", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "nano"));
    EXPECT_STREQ("Nano", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "SUBlime"));
    EXPECT_STREQ("Sublime", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "vi"));
    EXPECT_STREQ("Vi", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "vim"));
    EXPECT_STREQ("Vim", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "VSCODE"));
    EXPECT_STREQ("VSCode", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "Xed"));
    EXPECT_STREQ("Xed", options.editor);

    EXPECT_EQ(kOk, options_set(&options, "editor", "\"code -g ${file}:${line}\""));
    EXPECT_STREQ("\"code -g ${file}:${line}\"", options.editor);

    // Invalid because the value isn't quoted.
    strcpy(options.editor, "");
    EXPECT_EQ(kInvalidValue, options_set(&options, "editor", "code -g ${file}:${line}"));
    EXPECT_STREQ("", options.editor);
}

TEST(OptionsTest, Set_GivenFnKey) {
    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_set(&options, "f1", "\"foo\""));
    EXPECT_STREQ("foo", options.fn_keys[0]);

    EXPECT_EQ(kOk, options_set(&options, "F2", "\"\"foo\"\r\n\""));
    EXPECT_STREQ("\"foo\"\r\n", options.fn_keys[1]);

    EXPECT_EQ(kUnknownOption, options_set(&options, "f0", "\"foo\""));
    EXPECT_EQ(kUnknownOption, options_set(&options, "f13", "\"foo\""));
}

TEST(OptionsTest, FnKeyToString) {
    Options options;
    options_init(&options);
    char buf[STRINGSIZE];

    options_fn_key_to_string(options.fn_keys[0], buf);
    EXPECT_STREQ("FILES\\r\\n", buf);

    options_fn_key_to_string(options.fn_keys[4], buf);
    EXPECT_STREQ("AUTOSAVE \\\"\\\"\\202", buf);
}

TEST(OptionsTest, EncodeString) {
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

TEST(OptionsTest, EncodeString_GivenEncodedStringTooLong) {
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

TEST(OptionsTest, DecodeString) {
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

TEST(OptionsTest, DecodeString_GivenDecodedStringTooLong) {
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

TEST(OptionsTest, GetDefinition) {
    OptionsDefinition *lookup;
    for (const OptionsDefinition *def = options_definitions; def->name; ++def) {
        EXPECT_EQ(kOk, options_get_definition(def->name, &lookup));
        EXPECT_EQ(def, lookup);
    }

    EXPECT_EQ(kOk, options_get_definition("seaRCH paTH", &lookup));
    EXPECT_EQ(kOptionSearchPath, lookup->id);

    EXPECT_EQ(kUnknownOption, options_get_definition("unknown", &lookup));
}

TEST(OptionsTest, GetFloatValue_ForZFloat) {
    Options options;
    options_init(&options);
    MMFLOAT fvalue = 0.0;

    EXPECT_EQ(kOk, options_get_float_value(&options, kOptionZFloat, &fvalue));
    EXPECT_EQ(2.71828, fvalue);
}

TEST(OptionsTest, GetFloatValue_ForNonFloat) {
    Options options;
    options_init(&options);
    MMFLOAT fvalue = 0.0;

    EXPECT_EQ(kInternalFault, options_get_float_value(&options, kOptionZInteger, &fvalue));
    EXPECT_EQ(0.0, fvalue);
    EXPECT_EQ(kInternalFault, options_get_float_value(&options, kOptionZString, &fvalue));
    EXPECT_EQ(0.0, fvalue);
}

TEST(OptionsTest, GetIntegerValue_ForBase) {
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

TEST(OptionsTest, GetIntegerValue_ForBreakKey) {
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

TEST(OptionsTest, GetIntegerValue_ForTab) {
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

TEST(OptionsTest, GetIntegerValue_ForZBoolean) {
    Options options;
    options_init(&options);
    MMINTEGER ivalue = 0;

    EXPECT_EQ(kOk, options_get_integer_value(&options, kOptionZBoolean, &ivalue));
    EXPECT_EQ(true, (bool) ivalue);
}

TEST(OptionsTest, GetIntegerValue_ForZInteger) {
    Options options;
    options_init(&options);
    MMINTEGER ivalue = 0;

    EXPECT_EQ(kOk, options_get_integer_value(&options, kOptionZInteger, &ivalue));
    EXPECT_EQ(1945, ivalue);
}

TEST(OptionsTest, GetIntegerValue_ForNonInteger) {
    Options options;
    options_init(&options);
    MMINTEGER ivalue = 0;

    EXPECT_EQ(kInternalFault, options_get_integer_value(&options, kOptionZFloat, &ivalue));
    EXPECT_EQ(0, ivalue);
    EXPECT_EQ(kInternalFault, options_get_integer_value(&options, kOptionZString, &ivalue));
    EXPECT_EQ(0, ivalue);
}

TEST(OptionsTest, GetStringValue_ForBase) {
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

TEST(OptionsTest, GetStringValue_ForBreakKey) {
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

TEST(OptionsTest, GetStringValue_ForCodePage) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    for (char **codepage_name = (char **) CODEPAGE_NAMES; *codepage_name != NULL; ++codepage_name) {
        codepage_set(&options, *codepage_name);
        EXPECT_EQ(kOk, options_get_string_value(&options, kOptionCodePage, svalue));
        if (strcasecmp(*codepage_name, "NONE") == 0) {
            EXPECT_STREQ("None", svalue);
        } else {
            EXPECT_STREQ(*codepage_name, svalue);
        }
    }

    char tmp[] = "foo";
    options.codepage = tmp; // Invalid value.
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionCodePage, svalue));
    EXPECT_STREQ("???", svalue);
}

TEST(OptionsTest, GetStringValue_ForConsole) {
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

TEST(OptionsTest, GetStringValue_ForDefaultType) {
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

TEST(OptionsTest, GetStringValue_ForEditor) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    for (OptionsEditor *editor = options_editors; editor->id; ++editor) {
        strcpy(options.editor, editor->value);
        EXPECT_EQ(kOk, options_get_string_value(&options, kOptionEditor, svalue));
        EXPECT_STREQ(editor->value, svalue);
    }

    strcpy(options.editor, "myeditor ${file}:${line}");
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionEditor, svalue));
    EXPECT_STREQ("myeditor ${file}:${line}", svalue);

    strcpy(options.editor, "");
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionEditor, svalue));
    EXPECT_STREQ("", svalue);
}

TEST(OptionsTest, GetStringValue_ForExplicitType) {
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

TEST(OptionsTest, GetStringValue_ForFunctionKeys) {
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

TEST(OptionsTest, GetStringValue_ForListCase) {
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

TEST(OptionsTest, GetStringValue_ForResolution) {
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

TEST(OptionsTest, GetStringValue_ForSearchPath) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE];

    strcpy(options.search_path, "");
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionSearchPath, svalue));
    EXPECT_STREQ("", svalue);

    strcpy(options.search_path, "/home/thwill/foo");
    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionSearchPath, svalue));
    EXPECT_STREQ("/home/thwill/foo", svalue);
}

TEST(OptionsTest, GetStringValue_ForTab) {
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

TEST(OptionsTest, GetStringValue_ForZBoolean) {
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

TEST(OptionsTest, GetStringValue_ForZFloat) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE] = { 0 };

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionZFloat, svalue));
    EXPECT_STREQ("2.71828", svalue);
}

TEST(OptionsTest, GetStringValue_ForZInteger) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE] = { 0 };

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionZInteger, svalue));
    EXPECT_STREQ("1945", svalue);
}

TEST(OptionsTest, GetStringValue_ForZString) {
    Options options;
    options_init(&options);
    char svalue[STRINGSIZE] = { 0 };

    EXPECT_EQ(kOk, options_get_string_value(&options, kOptionZString, svalue));
    EXPECT_STREQ("wombat", svalue);
}
