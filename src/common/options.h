/*-*****************************************************************************

MMBasic for Linux (MMB4L)

options.h

Copyright 2021-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#if !defined(MMB4L_OPTIONS_H)
#define MMB4L_OPTIONS_H

#include <stdbool.h>

#include "../Configuration.h"
#include "mmresult.h"

#define OPTIONS_FILE_NAME       "~/.mmbasic/mmbasic.options"

/** Number of programmable function keys. */
#define OPTIONS_NUM_FN_KEYS     12

/** Maximum length of string that can be assigned to a programmable function key. */
#define OPTIONS_MAX_FN_KEY_LEN  64

typedef enum {
    kOptionAngle = 0,
    kOptionBase,
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
    kOptionSimulate,
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
    const char *name;
    const char *command; // Command to run for the given editor.
    bool blocking;       // Does the editor command block.
} OptionsEditor;

typedef enum { kRadians, kDegrees } OptionsAngle;

typedef enum { kBoth, kScreen, kSerial } OptionsConsole;

typedef enum {
    kSimulateMmb4l,
    kSimulateCmm2,
    kSimulatePicoMiteVga,
    kSimulateGameMite
} OptionsSimulate;

typedef enum { kTitle, kLower, kUpper } OptionsListCase;

typedef enum { kCharacter, kPixel } OptionsResolution;

typedef struct {
    OptionsAngle angle;
    int autorun;
    int base;
    char break_key;
    const char *codepage; // Pointer to one of the arrays/maps declared in 'codepage.h'
    OptionsConsole console;
    char default_type;
    char editor[STRINGSIZE];  // TODO: should probably be shorter
    char explicit_type;
    char fn_keys[OPTIONS_NUM_FN_KEYS][OPTIONS_MAX_FN_KEY_LEN + 1];
    int  height;
    OptionsListCase list_case;
    OptionsResolution resolution;
    char search_path[STRINGSIZE];
    OptionsSimulate simulate;
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

/** @brief Gets the C-string value to be displayed by OPTION LIST for the given option. */
MmResult options_get_display_value(const Options *options, OptionsId id, char *svalue);

/** @brief Gets the MMFLOAT value for the given option. */
MmResult options_get_float_value(const Options *options, OptionsId id, MMFLOAT *fvalue);

/** @brief Gets the MMINTEGER value for the given option. */
MmResult options_get_integer_value(const Options *options, OptionsId id, MMINTEGER *ivalue);

/** @brief Gets the C-string value for the given option. */
MmResult options_get_string_value(const Options *options, OptionsId id, char *svalue);

/** @brief Does the given option have its default value? */
bool options_has_default_value(const Options *options, OptionsId id);

/** @brief Loads persistent options from a file. */
MmResult options_load(Options *options, const char *filename, OPTIONS_WARNING_CB warning_cb);

/** @brief Saves persistent options to a file. */
MmResult options_save(const Options *options, const char *filename);

/** @brief Sets the value for the given option from an MMFLOAT. */
MmResult options_set_float_value(Options *options, OptionsId id, MMFLOAT fvalue);

/** @brief Sets the value for the given option from an MMINTEGER. */
MmResult options_set_integer_value(Options *options, OptionsId id, MMINTEGER ivalue);

/** @brief Sets the value for the given option from a C-string. */
MmResult options_set_string_value(Options *options, OptionsId id, const char *svalue);

#endif // #if !defined(MMB4L_OPTIONS_H)
