#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C" {

#include "../file.h"

int error_check() { return errno; }
int MMgetchar(void) { return -1; }
char console_putc(char c) { return c; }

}

TEST(FileTest, Exists) {
    EXPECT_EQ(file_exists("/bin/vi"), true);
    EXPECT_EQ(file_exists("/bin/does-not-exist"), false);
}

TEST(FileTest, IsEmpty) {
    EXPECT_EQ(file_is_empty("/bin/vi"), 0);

    char filename[] = "/tmp/is_empty_XXXXXX";
    int fd = mkstemp(filename);
    close(fd);
    EXPECT_EQ(file_exists(filename), true);
    EXPECT_EQ(file_is_empty(filename), true);
}

TEST(FileTest, IsRegular) {
    EXPECT_EQ(file_is_regular("/bin/vi"), true);
    EXPECT_EQ(file_is_regular("/bin"), false);
}

TEST(FileTest, HasExtension) {
    EXPECT_EQ(file_has_extension("foo.bas", ".bas", false), true);
    EXPECT_EQ(file_has_extension("foo.bas", ".BAS", false), false);
    EXPECT_EQ(file_has_extension("foo.bas", ".BAS", true), true);
    EXPECT_EQ(file_has_extension("foo.bas", ".inc", true), false);
}
