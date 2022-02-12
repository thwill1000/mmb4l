#include <gtest/gtest.h>

extern "C" {

#include "../option.h"
#include "../utility.h"

}

static char option_test_buf[1024];

static void write_line_to_buf(const char *line) {
    strcat(option_test_buf, line);
    strcat(option_test_buf, "\n");
}

TEST(OptionTest, Init) {
    Options options;
    options_init(&options);

    EXPECT_EQ(4, options.tab);
    EXPECT_EQ(0, options.list_case);
    EXPECT_EQ(0, options.height);
    EXPECT_EQ(0, options.width);
    EXPECT_EQ(PROG_FLASH_SIZE, options.prog_flash_size);
    EXPECT_EQ(SERIAL, options.console);
    EXPECT_EQ(CHARACTER, options.resolution);
    EXPECT_EQ(false, options.persistent_bool);
    EXPECT_EQ(0.0, options.persistent_float);
    EXPECT_STREQ("", options.persistent_string);
}

TEST(OptionTest, Save) {
    Options options;
    options_init(&options);
    options.persistent_bool = true;
    options.list_case = 1;
    options.tab = 8;
    options.persistent_float = 3.142;
    strcpy(options.persistent_string, "foo bar");
    const char *filename = "/tmp/option_test_save";

    EXPECT_EQ(options_save(&options, filename), 0);

    FILE *f = fopen(filename, "r");
    char line[256];
    EXPECT_STREQ(fgets(line, 256, f), "listcase = Lower\n");
    EXPECT_STREQ(fgets(line, 256, f), "tab = 8\n");
    EXPECT_STREQ(fgets(line, 256, f), "persistent-bool = true\n");
    EXPECT_STREQ(fgets(line, 256, f), "persistent-float = 3.142\n");
    EXPECT_STREQ(fgets(line, 256, f), "persistent-string = \"foo bar\"\n");
    EXPECT_FALSE(fgets(line, 256, f));
    EXPECT_TRUE(feof(f));
    fclose(f);
}

TEST(OptionTest, Save_GivenDirectoryDoesNotExist) {
    Options options;
    options_init(&options);
    options.persistent_bool = true;
    options.list_case = 1;
    options.tab = 8;
    options.persistent_float = 3.142;
    strcpy(options.persistent_string, "foo bar");

    const char *filename = "/tmp/option_test_save_dir/myfile.options";
    const char *directory = "/tmp/option_test_save_dir";
    remove(filename);
    remove(directory);

    EXPECT_EQ(0, options_save(&options, filename));

    FILE *f = fopen(filename, "r");
    char line[256];
    EXPECT_STREQ(fgets(line, 256, f), "listcase = Lower\n");
    EXPECT_STREQ(fgets(line, 256, f), "tab = 8\n");
    EXPECT_STREQ(fgets(line, 256, f), "persistent-bool = true\n");
    EXPECT_STREQ(fgets(line, 256, f), "persistent-float = 3.142\n");
    EXPECT_STREQ(fgets(line, 256, f), "persistent-string = \"foo bar\"\n");
    EXPECT_FALSE(fgets(line, 256, f));
    EXPECT_TRUE(feof(f));
    fclose(f);

    if (FAILED(remove(filename))) {
        perror("Save_GivenDirectoryDoesNotExist");
    }
    if (FAILED(remove(directory))) {
        perror("Save_GivenDirectoryDoesNotExist");
    }
}

TEST(OptionTest, Load) {
    const char *filename = "/tmp/option_test_load";
    FILE *f = fopen(filename, "w");
    fprintf(f, "listcase = Upper\n");
    fprintf(f, "tab = 8\n");
    fprintf(f, "persistent-bool = true\n");
    fprintf(f, "persistent-float = 3.142\n");
    fprintf(f, "persistent-string = \"foo bar\"\n");
    fclose(f);

    Options options;
    options_init(&options);

    EXPECT_EQ(kOk, options_load(&options, filename));
    EXPECT_EQ(2, options.list_case);
    EXPECT_EQ(8, options.tab);
    EXPECT_EQ(options.persistent_bool, true);
    EXPECT_DOUBLE_EQ(options.persistent_float, 3.142);
    EXPECT_STREQ(options.persistent_string, "foo bar");
}

TEST(OptionTest, Load_GivenAdditionalWhitespace) {
    const char *filename = "/tmp/option_test_load";
    FILE *f = fopen(filename, "w");
    fprintf(f, "tab = 8  \n");                         // Trailing whitespace
    fprintf(f, "  persistent-bool = true\n");                      // Leading whitespace
    fprintf(f, "persistent-float   =   3.142    \n");              // Whitespace around equals
    fprintf(f, "\tpersistent-string\t \t=\t \t\"foo bar\"\t\n\t"); // tab characters everywhere
    fclose(f);

    Options options;
    options_init(&options);

    EXPECT_EQ(options_load(&options, filename), 0);
    EXPECT_EQ(8, options.tab);
    EXPECT_EQ(options.persistent_bool, true);
    EXPECT_DOUBLE_EQ(options.persistent_float, 3.142);
    EXPECT_STREQ(options.persistent_string, "foo bar");
}

TEST(OptionTest, Load_GivenEmptyAndWhitespaceOnlyLines) {
    const char *filename = "/tmp/option_test_load";
    FILE *f = fopen(filename, "w");
    fprintf(f, "\n");
    fprintf(f, "persistent-bool = true\n");
    fprintf(f, "    \n");
    fprintf(f, "tab = 8\n");
    fprintf(f, "\r\n");
    fprintf(f, "persistent-float = 3.142\n");
    fprintf(f, "  \t  \t \n");
    fprintf(f, "persistent-string = \"foo bar\"\n");
    fclose(f);

    Options options;
    options_init(&options);

    EXPECT_EQ(options_load(&options, filename), 0);
    EXPECT_EQ(options.persistent_bool, true);
    EXPECT_EQ(8, options.tab);
    EXPECT_DOUBLE_EQ(options.persistent_float, 3.142);
    EXPECT_STREQ(options.persistent_string, "foo bar");
}

TEST(OptionTest, Load_GivenHashComments) {
    const char *filename = "/tmp/option_test_load";
    FILE *f = fopen(filename, "w");
    fprintf(f, "  # Hello World\n");
    fprintf(f, "persistent-bool = true # Trailing comment\n");
    fprintf(f, "# Leading comment persistent-bool = false\n");
    fprintf(f, "tab = 8\n");
    fprintf(f, "persistent-float = 3.142\n");
    fprintf(f, "persistent-string = \"foo bar\"\n");
    fclose(f);

    Options options;
    options_init(&options);

    EXPECT_EQ(options_load(&options, filename), 0);
    EXPECT_EQ(options.persistent_bool, true);
    EXPECT_EQ(8, options.tab);
    EXPECT_DOUBLE_EQ(options.persistent_float, 3.142);
    EXPECT_STREQ(options.persistent_string, "foo bar");
}

TEST(OptionTest, Load_GivenSemicolonComments) {
    const char *filename = "/tmp/option_test_load";
    FILE *f = fopen(filename, "w");
    fprintf(f, "  ; Hello World\n");
    fprintf(f, "persistent-bool = true ; Trailing comment\n");
    fprintf(f, "; Leading comment persistent-bool = false\n");
    fprintf(f, "tab = 8\n");
    fprintf(f, "persistent-float = 3.142\n");
    fprintf(f, "persistent-string = \"foo bar\"\n");
    fclose(f);

    Options options;
    options_init(&options);

    EXPECT_EQ(options_load(&options, filename), 0);
    EXPECT_EQ(options.persistent_bool, true);
    EXPECT_EQ(8, options.tab);
    EXPECT_DOUBLE_EQ(options.persistent_float, 3.142);
    EXPECT_STREQ(options.persistent_string, "foo bar");
}

TEST(OptionTest, Load_GivenErrors) {
    const char *filename = "/tmp/option_test_load";
    FILE *f = fopen(filename, "w");
    fprintf(f, "foo = true\n");
    fprintf(f, "persistent-bool = 42\n");
    fprintf(f, "tab = \"Hello World\"\n");
    fprintf(f, "persistent-float = true\n");
    fprintf(f, "persistent-string = 3.142\n");
    fclose(f);

    Options options;
    options_init(&options);

    option_test_buf[0] = '\0';
    options_load_error_callback = &write_line_to_buf;
    EXPECT_EQ(options_load(&options, filename), 0);

    EXPECT_STREQ(
            "line 1: unknown option 'foo'.\n"
            "line 2: invalid boolean value for option 'persistent-bool'.\n"
            "line 3: invalid integer value for option 'tab'.\n"
            "line 4: invalid float value for option 'persistent-float'.\n"
            "line 5: invalid string value for option 'persistent-string'.\n",
            option_test_buf);
}
