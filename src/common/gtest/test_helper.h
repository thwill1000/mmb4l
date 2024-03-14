/*
 * Copyright (c) 2024 Thomas Hugo Williams
 * License MIT <https://opensource.org/licenses/MIT>
 */

#if !defined(HELPER_H)
#define HELPER_H

extern char error_msg[256];

void clear_prog_memory();

// TODO: can I just use program_tokenise() instead ?
void tokenise_and_append(const char* untokenised);

#endif
