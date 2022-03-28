#include <gtest/gtest.h>

extern "C" {

#include "../cstring.h"
#include "../options.h"
#include "../utility.h"

}

static char options_test_buf[1024];

static void write_line_to_buf(const char *line) {
    cstring_cat(options_test_buf, line, sizeof(options_test_buf));
    cstring_cat(options_test_buf, "\n", sizeof(options_test_buf));
}

TEST(OptionsTest, Init) {
    Options options;
    options_init(&options);

    EXPECT_EQ(0, options.autorun);
    EXPECT_EQ(0, options.base);
    EXPECT_EQ(3, options.break_key);
    EXPECT_EQ(NULL, options.codepage);
    EXPECT_EQ(kSerial, options.console);
    EXPECT_EQ(0x1, options.default_type); // 0x1 = T_NBR
    EXPECT_STREQ("Nano", options.editor);
    EXPECT_EQ(false, options.explicit_type);
    EXPECT_EQ(0, options.height);
    EXPECT_EQ(0, options.list_case);
    EXPECT_EQ(PROG_FLASH_SIZE, options.prog_flash_size);
    EXPECT_EQ(kCharacter, options.resolution);
    EXPECT_STREQ("", options.search_path);
    EXPECT_EQ(4, options.tab);
    EXPECT_EQ(0, options.width);
    EXPECT_EQ(true, options.zboolean);
    EXPECT_EQ(2.71828, options.zfloat);
    EXPECT_EQ(1945, options.zinteger);
    EXPECT_STREQ("wombat", options.zstring);
}

static void expect_saved_content(const char *filename) {
    FILE *f = fopen(filename, "r");
    char line[256];
    EXPECT_STREQ(fgets(line, 256, f), "editor = Vi\n");
    EXPECT_STREQ(fgets(line, 256, f), "listcase = Lower\n");
    EXPECT_STREQ(fgets(line, 256, f), "tab = 8\n");
    EXPECT_STREQ(fgets(line, 256, f), "zboolean = false\n");
    EXPECT_STREQ(fgets(line, 256, f), "zfloat = 3.142\n");
    EXPECT_STREQ(fgets(line, 256, f), "zinteger = 42\n");
    EXPECT_STREQ(fgets(line, 256, f), "zstring = \"snafu\"\n");
    EXPECT_STREQ(fgets(line, 256, f), "search-path = \"/foo/bar\"\n");
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
    fprintf(f, "tab = 8  \n");                         // Trailing whitespace
    fprintf(f, "  zboolean = true\n");                      // Leading whitespace
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
