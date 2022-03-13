// Copyright (c) 2021-2022 Thomas Hugo Williams

#if !defined(MMB4L_OPTION_H)
#define MMB4L_OPTION_H

#include <stdbool.h>

#include "../Configuration.h"
#include "mmresult.h"

#define OPTIONS_FILE_NAME       "~/.mmbasic/mmbasic.options"

/** Number of programmable function keys. */
#define OPTIONS_NUM_FN_KEYS     12

/** Maximum length of string that can be assigned to a programmable function key. */
#define OPTIONS_MAX_FN_KEY_LEN  64

typedef struct {
    char *id;      // Users specify these with OPTION EDITOR.
    char *value;   // Internally we use this.
    char *command; // Command to run for the given editor.
    bool blocking; // Does the editor command block.
} OptionsEditor;

typedef enum { kBoth, kScreen, kSerial } OptionsConsole;

typedef enum { kTitle, kLower, kUpper } OptionsListCase;

typedef enum { kCharacter, kPixel } OptionsResolution;

typedef struct {
    int autorun;
    int base;
    char break_key;
    char *codepage; // Pointer to one of the arrays/maps declared in 'codepage.c'
    OptionsConsole console;
    char default_type;
    char editor[STRINGSIZE];  // TODO: should probably be shorter
    char explicit_type;
    char fn_keys[OPTIONS_NUM_FN_KEYS][OPTIONS_MAX_FN_KEY_LEN + 1];
    int  height;
    OptionsListCase list_case;
    int  prog_flash_size;
    OptionsResolution resolution;
    char search_path[STRINGSIZE];
    char tab;
    int  width;

#if defined OPTION_TESTS
    bool    zboolean;
    MMFLOAT zfloat;
    int     zinteger;
    char    zstring[32];
#endif
} Options;

typedef void (*OPTIONS_WARNING_CB) (const char *);

extern Options mmb_options;
extern OptionsEditor options_editors[];

/** Initialises the options. */
void options_init(Options *options);

/** @brief Decodes a C-string that has been encoded using options_encode_string(). */
MmResult options_decode_string(const char *encoded, char *decoded);

/**
 * @brief Encodes a C-string so that any unprintable characters are escaped.
 *
 * 0x0a => \n
 * 0x0d => \r
 * "    => \"
 * \    => \\
 * Other ASCII codes < 0x20 or > 0x7E         => \###
 * where ### is the 3-digit octal ASCII code
 */
MmResult options_encode_string(const char *unencoded, char *encoded);

/** Loads persistent options from a file. */
MmResult options_load(Options *options, const char *filename, OPTIONS_WARNING_CB warning_cb);

/** Saves persistent options to a file. */
MmResult options_save(const Options *options, const char *filename);

MmResult options_set(Options *options, const char *name, const char *value);

void options_console_to_string(OptionsConsole console, char *buf);
void options_editor_to_string(const char *editor, char *buf);
void options_explicit_to_string(char explicit_type, char *buf);
void options_fn_key_to_string(const char *fn_key, char *buf);
void options_list_case_to_string(OptionsListCase list_case, char *buf);
void options_resolution_to_string(OptionsResolution resolution, char *buf);
void options_type_to_string(char type, char *buf);

#endif
