/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#if !defined(MMB4L_TEST_CONFIG_H)
#define MMB4L_TEST_CONFIG_H

#if defined(__ANDROID__)

#define TERMUX_ROOT       "/data/data/com.termux"
#define TERMUX_FILES      TERMUX_ROOT  "/files"
#define BIN_DIR           TERMUX_FILES "/usr/bin"
#define HOME_DIR          TERMUX_FILES "/home"
#define TMP_DIR           TERMUX_FILES "/usr/tmp"

#else

#define BIN_DIR           "/bin"
#define HOME_DIR          "/home/thwill"
#define TMP_DIR           "/tmp"

#endif

#endif
