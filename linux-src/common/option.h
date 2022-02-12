// Copyright (c) 2021 Thomas Hugo Williams

#if !defined(OPTION_H)
#define OPTION_H

#include <stdbool.h>

#include "../Configuration.h"

#define OPTIONS_FILE_NAME  "~/.mmbasic/mmbasic.options"

typedef enum {
    kOk = 0,
    kFileNotFound,
    kInvalidFormat,
    kUnknownOption,
    kInvalidBool,
    kInvalidEnum,
    kInvalidFloat,
    kInvalidInt,
    kInvalidString,
    kOtherIoError
} OptionsResult;

enum option_console { BOTH, SCREEN, SERIAL };

enum option_resolution { CHARACTER, PIXEL };

typedef struct {
    char tab;
    char list_case;
    int  height;
    int  width;
    int  prog_flash_size;
    int  autorun;

    // Added for MMB4L
    enum option_console console;
    enum option_resolution resolution;

#if defined OPTION_TESTS
    bool    persistent_bool;
    MMFLOAT persistent_float;
    char    persistent_string[32];
#endif
} Options;

extern Options mmb_options;

extern void (*options_load_error_callback)(const char *);

/** Initialises the options. */
void options_init(Options *options);

/** Loads persistent options from a file. */
OptionsResult options_load(Options *options, const char *filename);

/** Saves persistent options to a file. */
OptionsResult options_save(const Options *options, const char *filename);

void option_console_to_string(enum option_console console, char *buf);
void option_explicit_to_string(char explicit_type, char *buf);
void option_list_case_to_string(char list_case, char *buf);
void option_resolution_to_string(enum option_resolution resolution, char *buf);
void option_type_to_string(char type, char *buf);

#endif
