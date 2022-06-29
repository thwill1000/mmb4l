/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#if !defined(MMB4L_TEST_CONFIG_H)
#define MMB4L_TEST_CONFIG_H

#define BIN_DIR           "/usr/bin"
#define TMP_DIR           "/tmp"

#if defined(__ANDROID__)

#define TERMUX_ROOT       "/data/data/com.termux"
#define TERMUX_FILES      TERMUX_ROOT  "/files"
#undef  BIN_DIR
#define BIN_DIR           TERMUX_FILES "/usr/bin"
#undef  TMP_DIR
#define TMP_DIR           TERMUX_FILES "/usr/tmp"

#elif defined(__arm__)

#undef  BIN_DIR
#define BIN_DIR           "/bin"

#endif

// Used like a macro so named like a macro.
static void SYSTEM_CALL(const char *cmd) {
    errno = 0;
    int exit_status = system(cmd);
    if (exit_status != 0) FAIL() << "system(\"" << cmd << "\" failed: " << exit_status;
}

#define MAKE_FILE(path)  SYSTEM_CALL("touch " path)
#define MKDIR(path)      SYSTEM_CALL("mkdir " path)

#define CHDIR(path) \
    errno = 0; \
    ASSERT_TRUE(SUCCEEDED(chdir(path))) << "chdir(" << path << ") failed: " << errno

#endif // MMB4L_TEST_CONFIG_H
