// Copyright (c) 2021 Thomas Hugo Williams

#if !defined(OPTION_H)
#define OPTION_H

enum option_console { BOTH, SCREEN, SERIAL };

enum option_resolution { CHARACTER, PIXEL };

struct option_s {
    char Tab;
    char Listcase;
    int  Height;
    int  Width;
    int  ProgFlashSize;
    int  Autorun;

    // Added for MMB4L
    enum option_console console;
    enum option_resolution resolution;
};

extern struct option_s Option;

void option_console_to_string(enum option_console console, char *buf);
void option_explicit_to_string(char explicit, char *buf);
void option_list_case_to_string(char list_case, char *buf);
void option_resolution_to_string(enum option_resolution resolution, char *buf);
void option_type_to_string(char type, char *buf);

#endif
