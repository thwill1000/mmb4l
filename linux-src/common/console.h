// Copyright (c) 2021 Thomas Hugo Williams

#if !defined(CONSOLE_H)
#define CONSOLE_H

// the values returned by the standard control keys
#define TAB 0x9
#define BKSP 0x8
#define ENTER 0xd
#define ESC 0x1b

// the values returned by the function keys
#define F1 0x91
#define F2 0x92
#define F3 0x93
#define F4 0x94
#define F5 0x95
#define F6 0x96
#define F7 0x97
#define F8 0x98
#define F9 0x99
#define F10 0x9a
#define F11 0x9b
#define F12 0x9c

// the values returned by special control keys
#define UP 0x80
#define DOWN 0x81
#define LEFT 0x82
#define RIGHT 0x83
#define INSERT 0x84
#define DEL 0x7f
#define HOME 0x86
#define END 0x87
#define PUP 0x88
#define PDOWN 0x89
#define NUM_ENT ENTER
#define SLOCK 0x8c
#define ALT 0x8b

// Ordinals match those used by MMBasic for DOS and original CMM.
#define BLACK           0
#define BLUE            1
#define GREEN           2
#define CYAN            3
#define RED             4
#define MAGENTA         5
#define YELLOW          6
#define WHITE           7
#define BRIGHT_BLACK    8
#define GREY            8
#define GRAY            8
#define BRIGHT_BLUE     9
#define BRIGHT_GREEN    10
#define BRIGHT_CYAN     11
#define BRIGHT_RED      12
#define BRIGHT_MAGENTA  13
#define BRIGHT_YELLOW   14
#define BRIGHT_WHITE    15

void console_init();
void console_background(int colour);
void console_bell();
void console_pump_input(void);
void console_clear(void);
void console_disable_raw_mode(void);
void console_enable_raw_mode(void);
void console_foreground(int colour);

/**
 * Gets a character from the console without blocking.
 *
 * @return  -1  if no character.
 */
int console_getc(void);

/**
 * Gets the cursor position.
 *
 * @param   x           on return holds the x-position.
 * @param   y           on return holds the y-position.
 * @param   timeout_ms  how long (in milliseconds) to wait for a response
 *                      from the terminal before reporting a failure.
 * @return              1 on success, otherwise 0.
 */
int console_get_cursor_pos(int *x, int *y, int timeout_ms);

/**
 * Gets the console size.
 *
 * @param   width   on return holds the width in characters.
 * @param   height  on return holds the height in characters.
 * @return          1 on success, otherwise 0.
 */
int console_get_size(int *width, int *height);

void console_home_cursor(void);
void console_invert(int invert);

/** Gets the number of characters waiting in the console input queue. */
int console_kbhit(void);

void console_reset(void);

/**
 * Sets the cursor position.
 *
 * @param  x  the new x-position.
 * @param  y  the new y-position.
 */
void console_set_cursor_pos(int x, int y);

void console_set_title(const char *title);
void console_show_cursor(int show);

#endif
