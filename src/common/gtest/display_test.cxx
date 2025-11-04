/*
 * Copyright (c) 2025 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#include <gtest/gtest.h>

extern "C" {

#include "../display.h"
#include "../graphics.h"
#include "../options.h"

#define FONT_1_WIDTH   8
#define FONT_1_HEIGHT  12

// Defined in "common/gtest/stubs/console_stubs.c"
extern MmResult (*mock_console_get_cursor_pos)(int *, int *);
extern MmResult (*mock_console_get_size)(int *, int *);
extern void (*mock_console_set_cursor_pos)(int, int);

int console_cursor_x;
int console_cursor_y;
int console_width;
int console_height;

// Defined in "common/graphics.c"
MmSurface *graphics_current = NULL;
MmGraphicsColour graphics_bcolour;
MmGraphicsColour graphics_fcolour;
uint32_t graphics_font = (1 << 4) + 1; // Font 1, Scale 1.
MmResult graphics_cls(MmSurface *surface, MmGraphicsColour colour) { return kOk; }
MmResult graphics_draw_box(MmSurface *surface, int x1, int y1, int x2, int y2, int width,
                           MmGraphicsColour colour, MmGraphicsColour fill) { return kOk; }
MmResult graphics_draw_char(MmSurface *surface,  int *x, int *y, uint32_t font,
                            MmGraphicsColour fcolour, MmGraphicsColour bcolour, char c,
                            TextOrientation orientation) { return kOk; }
MmResult graphics_draw_line(MmSurface *surface, int x1, int y1, int x2, int y2, int width,
                            MmGraphicsColour colour) { return kOk; }
MmResult graphics_scroll(MmSurface *surface, int x, int y, MmGraphicsColour fill) { return kOk; }

// Defined in "main.c"
Options mmb_options;

MmSurface graphics_display;

} // extern "C"

class DisplayTest : public ::testing::Test {

protected:

    void SetUp() override {
        mmb_options.console = kConsoleNone;
        console_cursor_x = 30;
        console_cursor_y = 60;
        console_width = 80;
        console_height = 40;
        mock_console_get_cursor_pos = [](int *x, int *y) {
            *x = console_cursor_x;
            *y = console_cursor_y;
            return (MmResult) kOk;
        };
        mock_console_get_size = [](int *width, int *height) {
            *width = console_width;
            *height = console_height;
            return (MmResult) kOk;
        };
        mock_console_set_cursor_pos = [](int x, int y) {
            console_cursor_x = x;
            console_cursor_y = y;
        };
        graphics_display.cursor_x = 300;
        graphics_display.cursor_y = 400;
        graphics_display.height = 480;
        graphics_display.width = 640;
    }

    void TearDown() override { }
};

void GivenGraphicsDisplay() {
    mmb_options.console = kScreen;
    graphics_current = &graphics_display;
}

void GivenConsoleDisplay() {
    mmb_options.console = kSerial;
    graphics_current = NULL;
}

void GivenConsoleGetCursorPosFails() {
    mock_console_get_cursor_pos = [](int *x, int *y) { return (MmResult) kError; };
}

void GivenConsoleGetSizeFails() {
    mock_console_get_size = [](int *width, int *height) { return (MmResult) kError; };
}

TEST_F(DisplayTest, GetCursorPos_InPixels_GivenGraphicsDisplay_Succeeds) {
    GivenGraphicsDisplay();
    int x, y;
    EXPECT_EQ(kOk, display_get_cursor_pos(true, &x, &y));
    EXPECT_EQ(300, x);
    EXPECT_EQ(400, y);
}

TEST_F(DisplayTest, GetCursorPos_InPixels_GivenConsoleDisplay_Succeeds) {
    GivenConsoleDisplay();
    int x, y;
    EXPECT_EQ(kOk, display_get_cursor_pos(true, &x, &y));
    EXPECT_EQ(30 * FONT_1_WIDTH, x);
    EXPECT_EQ(60 * FONT_1_HEIGHT, y);
};

TEST_F(DisplayTest, GetCursorPos_InCharacters_GivenGraphicsDisplay_Succeeds) {
    GivenGraphicsDisplay();
    int x, y;
    EXPECT_EQ(kOk, display_get_cursor_pos(false, &x, &y));
    EXPECT_EQ(300 / FONT_1_WIDTH, x);
    EXPECT_EQ(400 / FONT_1_HEIGHT, y);
}

TEST_F(DisplayTest, GetCursorPos_InCharacters_GivenConsoleDisplay_Succeeds) {
    GivenConsoleDisplay();
    int x, y;
    EXPECT_EQ(kOk, display_get_cursor_pos(false, &x, &y));
    EXPECT_EQ(30, x);
    EXPECT_EQ(60, y);
}

TEST_F(DisplayTest, GetCursorPos_GivenConsoleDisplay_AndConsoleGetCursorPosFails_Fails) {
    GivenConsoleDisplay();
    GivenConsoleGetCursorPosFails();
    int x, y;
    EXPECT_EQ(kError, display_get_cursor_pos(false, &x, &y));
}

TEST_F(DisplayTest, GetSize_InPixels_GivenGraphicsDisplay_Succeeds) {
    GivenGraphicsDisplay();
    int w, h;
    EXPECT_EQ(kOk, display_get_size(true, &w, &h));
    EXPECT_EQ(640, w);
    EXPECT_EQ(480, h);
}

TEST_F(DisplayTest, GetSize_InPixels_GivenConsoleDisplay_Succeeds) {
    GivenConsoleDisplay();
    int w, h;
    EXPECT_EQ(kOk, display_get_size(true, &w, &h));
    EXPECT_EQ(80 * FONT_1_WIDTH, w);
    EXPECT_EQ(40 * FONT_1_HEIGHT, h);
}

TEST_F(DisplayTest, GetSize_InCharacters_GivenGraphicsDisplay_Succeeds) {
    GivenGraphicsDisplay();
    int w, h;
    EXPECT_EQ(kOk, display_get_size(false, &w, &h));
    EXPECT_EQ(640 / FONT_1_WIDTH, w);
    EXPECT_EQ(480 / FONT_1_HEIGHT, h);
}

TEST_F(DisplayTest, GetSize_InCharacters_GivenConsoleDisplay_Succeeds) {
    GivenConsoleDisplay();
    int w, h;
    EXPECT_EQ(kOk, display_get_size(false, &w, &h));
    EXPECT_EQ(80, w);
    EXPECT_EQ(40, h);
}

TEST_F(DisplayTest, GetSize_GivenConsoleDisplay_AndConsoleGetSizeFails_Fails) {
    GivenConsoleDisplay();
    GivenConsoleGetSizeFails();
    int w, h;
    EXPECT_EQ(kError, display_get_size(false, &w, &h));
}

TEST_F(DisplayTest, SetCursorPos_InPixels_GivenGraphicsDisplay_Succeeds) {
    GivenGraphicsDisplay();
    mmb_options.console = kBoth;
    EXPECT_EQ(kOk, display_set_cursor_pos(true, 100, 200));
    EXPECT_EQ(100, graphics_current->cursor_x);
    EXPECT_EQ(200, graphics_current->cursor_y);
}

TEST_F(DisplayTest, SetCursorPos_InPixels_GivenConsoleDisplay_Succeeds) {
    GivenConsoleDisplay();
    EXPECT_EQ(kOk, display_set_cursor_pos(true, 100, 200));
    EXPECT_EQ((int) (100 / FONT_1_WIDTH), console_cursor_x);
    EXPECT_EQ((int) (200 / FONT_1_HEIGHT), console_cursor_y);
}

TEST_F(DisplayTest, SetCursorPos_InCharacters_GivenGraphicsDisplay_FailsWithUnimplementedError) {
    GivenGraphicsDisplay();
    mmb_options.console = kBoth;
    EXPECT_EQ(kOk, display_set_cursor_pos(false, 10, 20));
    EXPECT_EQ(10 * FONT_1_WIDTH, graphics_current->cursor_x);
    EXPECT_EQ(20 * FONT_1_HEIGHT, graphics_current->cursor_y);
}

TEST_F(DisplayTest, SetCursorPos_InCharacters_GivenConsoleDisplay_Succeeds) {
    GivenConsoleDisplay();
    EXPECT_EQ(kOk, display_set_cursor_pos(false, 10, 20));
    EXPECT_EQ(10, console_cursor_x);
    EXPECT_EQ(20, console_cursor_y);
}
