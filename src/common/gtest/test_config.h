/*
 * Copyright (c) 2022 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#if !defined(MMB4L_TEST_CONFIG_H)
#define MMB4L_TEST_CONFIG_H

#define BIN_DIR           "/usr/bin"
#define TMP_DIR           "/tmp"
#define HOME_DIR          "/home/thwill"

#if defined(__ANDROID__)

#define TERMUX_ROOT       "/data/data/com.termux"
#define TERMUX_FILES      TERMUX_ROOT  "/files"
#undef  BIN_DIR
#define BIN_DIR           TERMUX_FILES "/usr/bin"
#undef  TMP_DIR
#define TMP_DIR           TERMUX_FILES "/usr/tmp"
#undef  HOME_DIR
#define HOME_DIR          TERMUX_FILES "/home"

#elif defined(__arm__)

#undef  HOME_DIR
#define HOME_DIR          "/home/pi"

#endif

#endif // MMB4L_TEST_CONFIG_H
