// Copyright (c) 2021 Thomas Hugo Williams

#if !defined(CONSOLE_H)
#define CONSOLE_H

void console_clear(void);
void console_disable_raw_mode(void);
void console_enable_raw_mode(void);

/**
 * Gets a character from the console without blocking.
 *
 * @return  -1  if no character.
 */
int console_getc(void);

void console_get_size(void);
void console_set_title(const char *title);

#endif
