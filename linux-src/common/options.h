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

typedef enum {
    kOptionBase = 0,
    kOptionBreakKey,
    kOptionListCase, // Alphabetically ordered as if it were kOptionCase.
    kOptionCodePage,
    kOptionConsole,
    kOptionDefaultType,
    kOptionEditor,
    kOptionExplicitType,
    kOptionF1,
    kOptionF2,
    kOptionF3,
    kOptionF4,
    kOptionF5,
    kOptionF6,
    kOptionF7,
    kOptionF8,
    kOptionF9,
    kOptionF10,
    kOptionF11,
    kOptionF12,
    kOptionResolution,
    kOptionSearchPath,
    kOptionTab,
#if defined(OPTION_TESTS)
    kOptionZBoolean,
    kOptionZFloat,
    kOptionZInteger,
    kOptionZString
#endif
} OptionsId;

typedef enum {
    kOptionTypeFloat   = 0x01, // Same as T_NBR
    kOptionTypeString  = 0x02, // Same as T_STR
    kOptionTypeInteger = 0x04, // Same as T_INT
    kOptionTypeBoolean = 0x40
} OptionsType;

typedef struct {
    const char *name;
    int ordinal;
} NameOrdinalPair;

typedef struct {
    const char *name;
    OptionsId id;
    OptionsType type;
    bool saved;
    const char *default_value;
    const NameOrdinalPair *enum_map;
} OptionsDefinition;

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
extern OptionsDefinition options_definitions[];
extern OptionsEditor options_editors[];

/** @brief Initialises the options. */
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

/** @brief Gets the OptionsDefinition for a named option. */
MmResult options_get_definition(const char *name, OptionsDefinition **definition);

/** @brief Gets the MMFLOAT value for the given option. */
MmResult options_get_float_value(const Options *options, OptionsId id, MMFLOAT *fvalue);

/** @brief Gets the MMINTEGER value for the given option. */
MmResult options_get_integer_value(const Options *options, OptionsId id, MMINTEGER *ivalue);

/** @brief Gets the C-string value for the given option. */
MmResult options_get_string_value(const Options *options, OptionsId id, char *svalue);

/** Loads persistent options from a file. */
MmResult options_load(Options *options, const char *filename, OPTIONS_WARNING_CB warning_cb);

/** Saves persistent options to a file. */
MmResult options_save(const Options *options, const char *filename);

/** @brief Sets the value for the given option from an MMFLOAT. */
MmResult options_set_float_value(Options *options, OptionsId id, MMFLOAT fvalue);

/** @brief Sets the value for the given option from an MMINTEGER. */
MmResult options_set_integer_value(Options *options, OptionsId id, MMINTEGER ivalue);

/** @brief Sets the value for the given option from a C-string. */
MmResult options_set_string_value(Options *options, OptionsId id, const char *svalue);

MmResult options_set(Options *options, const char *name, const char *value);

void options_console_to_string(OptionsConsole console, char *buf);
void options_editor_to_string(const char *editor, char *buf);
void options_explicit_to_string(char explicit_type, char *buf);
void options_fn_key_to_string(const char *fn_key, char *buf);
void options_list_case_to_string(OptionsListCase list_case, char *buf);
void options_resolution_to_string(OptionsResolution resolution, char *buf);
void options_type_to_string(char type, char *buf);

#endif
