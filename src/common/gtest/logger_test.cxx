/*
 * Copyright (c) 2026-2026 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../logger.h"
#include "../cstring.h"

}

class LoggerTest : public ::testing::Test {

protected:

    void SetUp() override {
        // Every test starts from a known state regardless of execution order
        // or what a previous test left behind.
        logger_min_level = kLoggerLevelNone;
    }

    void TearDown() override {
        logger_min_level = kLoggerLevelNone;
    }
};

// ---------------------------------------------------------------------------
// logger_level_from_string()
// ---------------------------------------------------------------------------

TEST_F(LoggerTest, LevelFromString_GivenValidNames) {
    EXPECT_EQ(kLoggerLevelUninitialised, logger_level_from_string("Uninitialised"));
    EXPECT_EQ(kLoggerLevelDebug, logger_level_from_string("Debug"));
    EXPECT_EQ(kLoggerLevelInfo, logger_level_from_string("Info"));
    EXPECT_EQ(kLoggerLevelWarning, logger_level_from_string("Warning"));
    EXPECT_EQ(kLoggerLevelError, logger_level_from_string("Error"));
    EXPECT_EQ(kLoggerLevelFatal, logger_level_from_string("Fatal"));
    EXPECT_EQ(kLoggerLevelNone, logger_level_from_string("None"));
}

TEST_F(LoggerTest, LevelFromString_IsCaseInsensitive) {
    EXPECT_EQ(kLoggerLevelDebug, logger_level_from_string("debug"));
    EXPECT_EQ(kLoggerLevelDebug, logger_level_from_string("DEBUG"));
    EXPECT_EQ(kLoggerLevelDebug, logger_level_from_string("DeBuG"));
    EXPECT_EQ(kLoggerLevelWarning, logger_level_from_string("wArNiNg"));
    EXPECT_EQ(kLoggerLevelNone, logger_level_from_string("none"));
}

TEST_F(LoggerTest, LevelFromString_GivenUnknownName_ReturnsUninitialised) {
    EXPECT_EQ(kLoggerLevelUninitialised, logger_level_from_string("Trace"));
    EXPECT_EQ(kLoggerLevelUninitialised, logger_level_from_string("verbose"));
    EXPECT_EQ(kLoggerLevelUninitialised, logger_level_from_string(""));
    EXPECT_EQ(kLoggerLevelUninitialised, logger_level_from_string("Debugg"));
    EXPECT_EQ(kLoggerLevelUninitialised, logger_level_from_string(" Debug"));
}

// ---------------------------------------------------------------------------
// logger_level_as_string()
// ---------------------------------------------------------------------------

TEST_F(LoggerTest, LevelAsString_GivenValidLevels) {
    char buf[32];

    EXPECT_EQ(kOk, logger_level_as_string(kLoggerLevelUninitialised, buf, sizeof(buf)));
    EXPECT_STREQ("Uninitialised", buf);

    EXPECT_EQ(kOk, logger_level_as_string(kLoggerLevelDebug, buf, sizeof(buf)));
    EXPECT_STREQ("Debug", buf);

    EXPECT_EQ(kOk, logger_level_as_string(kLoggerLevelInfo, buf, sizeof(buf)));
    EXPECT_STREQ("Info", buf);

    EXPECT_EQ(kOk, logger_level_as_string(kLoggerLevelWarning, buf, sizeof(buf)));
    EXPECT_STREQ("Warning", buf);

    EXPECT_EQ(kOk, logger_level_as_string(kLoggerLevelError, buf, sizeof(buf)));
    EXPECT_STREQ("Error", buf);

    EXPECT_EQ(kOk, logger_level_as_string(kLoggerLevelFatal, buf, sizeof(buf)));
    EXPECT_STREQ("Fatal", buf);

    EXPECT_EQ(kOk, logger_level_as_string(kLoggerLevelNone, buf, sizeof(buf)));
    EXPECT_STREQ("None", buf);
}

TEST_F(LoggerTest, LevelAsString_GivenOutOfRangeLevel_Fails) {
    char buf[32];

    EXPECT_EQ(kInvalidValue,
              logger_level_as_string((LoggerLevel) (kLoggerLevelUninitialised - 1), buf,
                                     sizeof(buf)));
    EXPECT_EQ(kInvalidValue,
              logger_level_as_string((LoggerLevel) (kLoggerLevelNone + 1), buf, sizeof(buf)));
}

TEST_F(LoggerTest, LevelAsString_GivenBufferTooSmall_Fails) {
    char buf[4]; // Too small for "Uninitialised", "Warning", etc.

    EXPECT_EQ(kStringTooLong, logger_level_as_string(kLoggerLevelUninitialised, buf, sizeof(buf)));
    EXPECT_EQ(kStringTooLong, logger_level_as_string(kLoggerLevelWarning, buf, sizeof(buf)));

    // "None" is 4 chars + NUL, so still too long for a 4-byte buffer.
    EXPECT_EQ(kStringTooLong, logger_level_as_string(kLoggerLevelNone, buf, sizeof(buf)));
}

TEST_F(LoggerTest, LevelAsString_RoundTripsWithLevelFromString) {
    char buf[32];
    const LoggerLevel levels[] = {
        kLoggerLevelUninitialised, kLoggerLevelDebug,   kLoggerLevelInfo, kLoggerLevelWarning,
        kLoggerLevelError,         kLoggerLevelFatal,   kLoggerLevelNone,
    };

    for (LoggerLevel level : levels) {
        ASSERT_EQ(kOk, logger_level_as_string(level, buf, sizeof(buf)));
        EXPECT_EQ(level, logger_level_from_string(buf));
    }
}

// ---------------------------------------------------------------------------
// logger_set_min_level()
// ---------------------------------------------------------------------------

TEST_F(LoggerTest, SetMinLevel_GivenValidLevels_Succeeds) {
    EXPECT_EQ(kOk, logger_set_min_level(kLoggerLevelDebug));
    EXPECT_EQ(kLoggerLevelDebug, logger_min_level);

    EXPECT_EQ(kOk, logger_set_min_level(kLoggerLevelInfo));
    EXPECT_EQ(kLoggerLevelInfo, logger_min_level);

    EXPECT_EQ(kOk, logger_set_min_level(kLoggerLevelWarning));
    EXPECT_EQ(kLoggerLevelWarning, logger_min_level);

    EXPECT_EQ(kOk, logger_set_min_level(kLoggerLevelError));
    EXPECT_EQ(kLoggerLevelError, logger_min_level);

    EXPECT_EQ(kOk, logger_set_min_level(kLoggerLevelFatal));
    EXPECT_EQ(kLoggerLevelFatal, logger_min_level);

    EXPECT_EQ(kOk, logger_set_min_level(kLoggerLevelNone));
    EXPECT_EQ(kLoggerLevelNone, logger_min_level);
}

TEST_F(LoggerTest, SetMinLevel_GivenUninitialised_Fails) {
    logger_min_level = kLoggerLevelWarning; // Sentinel to prove it's unchanged.

    EXPECT_EQ(kInvalidValue, logger_set_min_level(kLoggerLevelUninitialised));
    EXPECT_EQ(kLoggerLevelWarning, logger_min_level);
}

TEST_F(LoggerTest, SetMinLevel_GivenOutOfRangeLevel_Fails) {
    logger_min_level = kLoggerLevelWarning;

    EXPECT_EQ(kInvalidValue, logger_set_min_level((LoggerLevel) (kLoggerLevelNone + 1)));
    EXPECT_EQ(kLoggerLevelWarning, logger_min_level);

    EXPECT_EQ(kInvalidValue, logger_set_min_level((LoggerLevel) -1));
    EXPECT_EQ(kLoggerLevelWarning, logger_min_level);
}

TEST_F(LoggerTest, SetMinLevel_ThenLoggerWillLog_ReflectsNewLevel) {
    ASSERT_EQ(kOk, logger_set_min_level(kLoggerLevelWarning));

    EXPECT_FALSE(logger_will_log(kLoggerLevelDebug));
    EXPECT_FALSE(logger_will_log(kLoggerLevelInfo));
    EXPECT_TRUE(logger_will_log(kLoggerLevelWarning));
    EXPECT_TRUE(logger_will_log(kLoggerLevelError));
    EXPECT_TRUE(logger_will_log(kLoggerLevelFatal));
}

