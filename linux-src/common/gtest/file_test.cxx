#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C" {

#include "../file.h"

int error_check() { return errno; }
int MMgetchar(void) { return -1; }
char console_putc(char c) { return c; }

}

TEST(FileTest, Exists) {
    EXPECT_EQ(file_exists("/bin/vi"), 1);
    EXPECT_EQ(file_exists("/bin/does-not-exist"), 0);
}

TEST(FileTest, IsEmpty) {
    EXPECT_EQ(file_is_empty("/bin/vi"), 0);

    char filename[] = "/tmp/is_empty_XXXXXX";
    int fd = mkstemp(filename);
    close(fd);
    EXPECT_EQ(file_exists(filename), 1);
    EXPECT_EQ(file_is_empty(filename), 1);
}

TEST(FileTest, IsRegular) {
    EXPECT_EQ(file_is_regular("/bin/vi"), 1);
    EXPECT_EQ(file_is_regular("/bin"), 0);
}

TEST(FileTest, HasExtension) {
    EXPECT_EQ(file_has_extension("foo.bas", ".bas", 0), 1);
    EXPECT_EQ(file_has_extension("foo.bas", ".BAS", 0), 0);
    EXPECT_EQ(file_has_extension("foo.bas", ".BAS", 1), 1);
    EXPECT_EQ(file_has_extension("foo.bas", ".inc", 1), 0);
}
