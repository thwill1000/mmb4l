/*-*****************************************************************************

MMBasic for Linux (MMB4L)

parse.c

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

#include "mmb4l.h"
#include "console.h"
#include "cstring.h"
#include "gpio.h"
#include "parse.h"
#include "path.h"
#include "sprite.h"
#include "utility.h"
#include "../core/tokentbl.h"

#include <stdlib.h>
#include <string.h>

bool parse_is_end(const char *p) {
    return *p == '\0' || *p == '\'';
}

const char *parse_check_string(const char *p, const char *tkn) {
    skipspace(p);  // skip leading spaces
    while(*tkn && (toupper(*tkn) == toupper(*p))) { tkn++; p++; }  // compare the strings
    if (*tkn == 0 && !isnamechar(*p)) {
        skipspace(p);
        return p;  // if successful return a pointer to the next non space character after the matched string
    }
    return NULL;  // or NULL if not
}

bool parse_bool(const char *p) {
    if (parse_check_string(p, "ON") || parse_check_string(p, "TRUE")) {
        return true;
    } else if (parse_check_string(p, "OFF") || parse_check_string(p, "FALSE")) {
        return false;
    } else {
        return getint(p, 0, 1) == 1;
    }
}

int parse_colour(const char *p, bool allow_bright) {
    const char *p2;
    if ((p2 = parse_check_string(p, "BLACK"))) {
        return BLACK;
    } else if ((p2 = parse_check_string(p, "BLUE"))) {
        return BLUE;
    } else if ((p2 = parse_check_string(p, "BRIGHT"))) {
        if (allow_bright) {
            int colour = parse_colour(p2, 0);
            if (colour == -1) return -1;
            return colour + BRIGHT_BLACK;
        } else {
            ERROR_NOT_ALLOWED("BRIGHT");
        }
    } else if ((p2 = parse_check_string(p, "CYAN"))) {
        return CYAN;
    } else if ((p2 = parse_check_string(p, "GREEN"))) {
        return GREEN;
    } else if ((p2 = parse_check_string(p, "GRAY"))) {
        if (allow_bright) {
            return BRIGHT_BLACK;
        } else {
            ERROR_NOT_ALLOWED("GRAY");
        }
    } else if ((p2 = parse_check_string(p, "GREY"))) {
        if (allow_bright) {
            return BRIGHT_BLACK;
        } else {
            ERROR_NOT_ALLOWED("GREY");
        }
    } else if ((p2 = parse_check_string(p, "MAGENTA"))) {
        return MAGENTA;
    } else if ((p2 = parse_check_string(p, "PURPLE"))) {
        return MAGENTA;
    } else if ((p2 = parse_check_string(p, "RED"))) {
        return RED;
    } else if ((p2 = parse_check_string(p, "WHITE"))) {
        return WHITE;
    } else if ((p2 = parse_check_string(p, "YELLOW"))) {
        return YELLOW;
    }

    int colour = getint(p, BLACK, BRIGHT_WHITE);
    if (!allow_bright && colour > WHITE) colour = -1;
    return colour;
}

int parse_file_number(const char *p, bool allow_zero) {
    skipspace(p); // Do we need this ?
    if (*p == '#') p++;
    int fnbr = getinteger(p);
    if (fnbr == 0 && !allow_zero) return -1;
    if (fnbr < 0 || fnbr > MAXOPENFILES) return -1;
    return fnbr;
}

bool parse_matches_longstring_pattern(const char *s) {
    char *p = (char *) s;
    skipspace(p);
    if (!isnamestart(*p)) return false;
    p++;
    while (isnamechar(*p)) p++;
    if (*p == '%') p++;
    skipspace(p);
    if (*p++ != '(') return false;
    skipspace(p);
    if (*p++ != ')') return false;
    skipspace(p);
    return *p == '\0' ? true : false;
}

MmResult parse_name(const char **p, char *name) {
    skipspace((*p)); // Double bracket is necessary for correct macro expansion.
    if (!isnamestart(**p)) return kInvalidName;
    size_t name_len = 0;
    *name++ = toupper(*((*p)++));
    name_len++;
    while (isnamechar(**p) && name_len < MAXVARLEN) {
        *name++ = toupper(*((*p)++));
        name_len++;
    }
    *name = '\0';
    if (isnamechar(**p)) return kNameTooLong;
    return kOk;
}

/**
 * @brief Transforms input beginning with * into a corresponding RUN command.
 *
 * e.g.
 *   *foo              =>  RUN "foo"
 *   *"foo bar"        =>  RUN "foo bar"
 *   *foo --wombat     =>  RUN "foo", "--wombat"
 *   *foo "wom"        =>  RUN "foo", Chr$(34) + "wom" + Chr$(34)
 *   *foo "wom" "bat"  =>  RUN "foo", Chr$(34) + "wom" + Chr$(34) + " " + Chr$(34) + "bat" + Chr$(34)
 *   *foo --wom="bat"  =>  RUN "foo", "--wom=" + Chr$(34) + "bat" + Chr$(34)
 */
static MmResult parse_transform_star_command(char *input) {
    char *src = input;
    while (isspace(*src)) src++; // Skip leading whitespace.
    if (*src != '*') return kInternalFault;
    src++;

    // Trim any trailing whitespace from the input.
    char *end = input + strlen(input) - 1;
    while (isspace(*end)) *end-- = '\0';

    // Allocate extra space to avoid string overrun.
    char *tmp = (char *) GetTempMemory(INPBUF_SIZE + 32);
    strcpy(tmp, "RUN");
    char *dst = tmp + 3;

    if (*src == '"') {
        // Everything before the second quote is the name of the file to RUN.
        *dst++ = ' ';
        *dst++ = *src++; // Leading quote.
        while (*src && *src != '"') *dst++ = *src++;
        if (*src == '"') *dst++ = *src++; // Trailing quote.
    } else {
        // Everything before the first space is the name of the file to RUN.
        int count = 0;
        while (*src && !isspace(*src)) {
            if (++count == 1) {
                *dst++ = ' ';
                *dst++ = '\"';
            }
            *dst++ = *src++;
        }
        if (count) *dst++ = '\"';
    }

    while (isspace(*src)) src++; // Skip whitespace.

    // Anything else is arguments.
    if (*src) {
        *dst++ = ',';
        *dst++ = ' ';

        // If 'src' starts with double-quote then replace with: Chr$(34) +
        if (*src == '"') {
            memcpy(dst, "Chr$(34) + ", 11);
            dst += 11;
            src++;
        }

        *dst++ = '\"';

        // Copy from 'src' to 'dst'.
        while (*src) {
            if (*src == '"') {
                // Close current set of quotes to insert a Chr$(34)
                memcpy(dst, "\" + Chr$(34)", 12);
                dst += 12;

                // Open another set of quotes unless this was the last character.
                if (*(src + 1)) {
                    memcpy(dst, " + \"", 4);
                    dst += 4;
                }
                src++;
            } else {
                *dst++ = *src++;
            }
            if (dst - tmp >= INPBUF_SIZE) {
                ClearSpecificTempMemory(tmp);
                return kStringTooLong;
            }
        }

        // End with a double quote unless 'src' ended with one.
        if (*(src - 1) != '"') *dst++ = '\"';

        *dst = '\0';
    }

    if (dst - tmp >= INPBUF_SIZE) {
        ClearSpecificTempMemory(tmp);
        return kStringTooLong;
    }

    // Copy transformed string back into the input buffer.
    cstring_cpy(input, tmp, INPBUF_SIZE);

    ClearSpecificTempMemory(tmp);
    return kOk;
}

static MmResult parse_transform_bang_cd_command(char *input, char *src) {
    while (isspace(*src)) src++;

    if (!*src) {
        // Special handling for 'cd' on its own.
        strcpy(input, "CHDIR \"~\"");
        return kOk;
    }

    // Allocate extra space to avoid string overrun.
    char *tmp = (char *) GetTempMemory(INPBUF_SIZE + 32);
    strcpy(tmp, "CHDIR ");
    char *dst = tmp + 6;

    // Start the command with a double quote unless the input started with one.
    if (*src != '\"') *dst++ = '\"';

    // Copy from src to dst.
    while (*src) {
        *dst++ = *src++;
        if (dst > tmp + INPBUF_SIZE - 1) {
            ClearSpecificTempMemory(tmp);
            return kStringTooLong;
        }
    }

    // End the command with a double quote unless the input ended with one.
    if (*(src - 1) != '\"') *dst++ = '\"';

    *dst = '\0';

    // Copy transformed string back into the input buffer.
    cstring_cpy(input, tmp, INPBUF_SIZE);

    ClearSpecificTempMemory(tmp);
    return kOk;
}

/**
 * @brief Transforms input beginning with ! into a corresponding SYSTEM command.
 */
static MmResult parse_transform_bang_command(char *input) {
    char *src = input;
    while (isspace(*src)) src++; // Skip whitespace.
    if (*src != '!') return kInternalFault;
    src++;

    // Trim any whitespace after the bang.
    while (isspace(*src)) src++;

    // Trim any trailing whitespace from the input.
    char *end = input + strlen(input) - 1;
    while (isspace(*end)) *end-- = '\0';

    if (!*src) {
        // Special case when it's just bang; this will be reported as a syntax error.
        strcpy(input, "SYSTEM");
        return kOk;
    }

    if (memcmp(src, "cd", 2) == 0
            && (*(src + 2) == '\0' || isblank(*(src + 2)))
            && !strstr(src, ";")
            && !strstr(src, "&&")
            && !strstr(src, "||")) {
        // Special case for the 'cd' command unless there are multiple commands.
        return parse_transform_bang_cd_command(input, src + 2);
    }

    // Allocate extra space to avoid string overrun.
    char *tmp = (char *) GetTempMemory(INPBUF_SIZE + 32);
    strcpy(tmp, "SYSTEM ");
    char *dst = tmp + 7;

    // Start the command with a double quote unless the input started with one.
    if (*src != '\"') *dst++ = '\"';

    // Copy from src to dst replacing any quotes with Chr$(34).
    while (*src) {
        if (*src == '"') {
            if (dst > tmp + 7) {
                memcpy(dst, "\" + ", 4);
                dst += 4;
            }
            memcpy(dst, "Chr$(34)", 8);
            dst += 8;
            if (*(src + 1)) {
                memcpy(dst, " + \"", 4);
                dst += 4;
            }
            src++;
        } else {
            *dst++ = *src++;
        }
        if (dst > tmp + INPBUF_SIZE - 2) {
            ClearSpecificTempMemory(tmp);
            return kStringTooLong;
        }
    }

    // End the command with a double quote unless the input ended with one.
    if (memcmp(dst - 8, "Chr$(34)", 8) != 0) *dst++ = '\"';

    *dst = '\0';

    // Copy transformed string back into the input buffer.
    cstring_cpy(input, tmp, INPBUF_SIZE);

    ClearSpecificTempMemory(tmp);
    return kOk;
}

MmResult parse_transform_input_buffer(char *input) {
    char *p = input;
    while (isspace(*p)) p++; // Skip whitespace.
    switch (*p) {
        case '*':
            return parse_transform_star_command(input);
        case '!':
            return parse_transform_bang_command(input);
        default:
            return kOk;
    }
}

static MmResult parse_skip_name(const char **p, uint8_t *len) {
    if (!isnamestart(**p)) return kInvalidName;
    *len = 1;
    (*p)++;
    while (isnamechar(**p)) {
        (*p)++;
        (*len)++;
    }
    return *len > MAXVARLEN ? kNameTooLong : kOk;
}

MmResult parse_implied_type(const char **p, uint8_t *type) {
    const char *tp = NULL;
    if ((tp = checkstring(*p, "INTEGER")) != NULL) {
        *type = T_INT | T_IMPLIED;
    } else if ((tp = checkstring(*p, "STRING")) != NULL) {
        *type = T_STR | T_IMPLIED;
    } else if ((tp = checkstring(*p, "FLOAT")) != NULL) {
        *type = T_NBR | T_IMPLIED;
    } else {
        return kMissingType;
    }
    if (tp) *p = tp;
    return kOk;
}

MmResult parse_explicit_type(const char **p, uint8_t *type) {
    switch (**p) {
        case '%':
            *type = T_INT;
            (*p)++;
            break;
        case '$':
            *type = T_STR;
            (*p)++;
            break;
        case '!':
            *type = T_NBR;
            (*p)++;
            break;
        default:
            *type = T_NOTYPE;
            break;
    }
    return kOk;
}

MmResult parse_fn_sig(const char **p, FunctionSignature *signature) {
    int bracket_count = 0;
    MmResult result = kOk;

    // Start from a blank slate.
    memset(signature, 0, sizeof(FunctionSignature));

    // Parse FUNCTION/SUB command token.
    skipspace((*p)); // Double bracket is necessary for correct macro expansion.
    signature->addr = *p;
    signature->token = commandtbl_decode(*p);
    if (signature->token != cmdSUB && signature->token != cmdFUN) return kInternalFault;
    *p += sizeof(CommandToken); // Jump over the command token.

    // Parse FUNCTION/SUB name.
    skipspace((*p));
    signature->name_offset = *p - signature->addr;
    result = parse_skip_name(p, &signature->name_len);
    if (FAILED(result)) return result;

    // Parse %, $, ! explicit type.
    result = parse_explicit_type(p, &signature->type);
    if (FAILED(result)) return result;
    if (signature->token == cmdSUB && signature->type) return kInvalidSubDefinition;
    if ((**p != '\0') && (**p != '\'') && (**p != ' ') && (**p != '(')) {
        return signature->token == cmdSUB ? kInvalidSubDefinition : kInvalidFunctionDefinition;
    }

    // Parse open bracket, obligatory for FUNCTION, optional for SUB.
    skipspace((*p));
    if (**p == '(') {
        bracket_count++;
        (*p)++; // Jump over the open bracket.
    } else if (signature->token == cmdFUN) {
        return kMissingOpenBracket;
    }

    // Parse comma separated parameter list.
    skipspace((*p));
    if ((**p != '\0') && (**p != '\'') && (**p != ')')) {
        for (;;) {
            if (signature->num_params == MAX_PARAMETERS) return kTooManyParameters;

            ParameterSignature *param = &signature->params[signature->num_params];

            // Parse parameter name.
            skipspace((*p));
            param->name_offset = *p - signature->addr;
            result = parse_skip_name(p, &param->name_len);
            if (FAILED(result)) return result;

            // Parse %, $, ! explicit type.
            result = parse_explicit_type(p, &(param->type));
            if (FAILED(result)) return result;

            // Parse array parameter.
            skipspace((*p));
            if (**p == '(') {
                (*p)++; // Jump over the open bracket.
                skipspace((*p));
                if (**p != ')') return kInvalidArrayParameter;
                (*p)++; // Jump over the closing bracket.
                skipspace((*p));
                param->array = true;
            }

            // Parse optional trailing AS FLOAT|INTEGER|STRING.
            if (**p == tokenAS) {
                (*p)++; // Jump over the AS token.
                if (param->type) return kTypeSpecifiedTwice;
                skipspace((*p));
                result = parse_implied_type(p, &(param->type));
                if (FAILED(result)) return result;
            }

            // If no parameter type has been specified then use the default type.
            if (param->type == T_NOTYPE) {
                if (mmb_options.default_type == T_NOTYPE) {
                    return kMissingType;
                } else {
                    param->type = mmb_options.default_type;
                }
            }

            signature->num_params++;

            // Are there any more parameters?
            skipspace((*p));
            if (**p == ',') {
                (*p)++; // Jump over the comma.
            } else {
                break;
            }
        }
    }

    // Parse optional trailing AS FLOAT|INTEGER|STRING.
    if (**p == ')') {
        if (bracket_count == 0) return kUnexpectedCloseBracket;
        bracket_count--;
        (*p)++; // Jump over the closing bracket.
        skipspace((*p));
        if (**p == tokenAS) {
            if (signature->token == cmdSUB) return kInvalidSubDefinition;
            if (signature->type) return kTypeSpecifiedTwice;
            (*p)++; // Jump over the AS token.
            skipspace((*p));
            result = parse_implied_type(p, &signature->type);
            if (FAILED(result)) return result;
        }
    }

    if (bracket_count != 0) return kMissingCloseBracket;

    skipspace((*p));
    if (**p != '\0' && **p != '\'') return kUnexpectedText;

    // If no function type has been specified then use the default type.
    if (signature->token == cmdFUN && signature->type == T_NOTYPE) {
        if (mmb_options.default_type == T_NOTYPE) {
            return kMissingType;
        } else {
            signature->type = mmb_options.default_type;
        }
    }

    return result;
}

MmResult parse_gp_pin(const char **p, uint8_t *gp) {
    const char *tp = *p;
    *gp = 0;
    skipspace((*p));
    if (*tp != 'g' && *tp != 'G') return kNotParsed;
    tp++;
    if (*tp != 'p' && *tp != 'P') return kNotParsed;
    tp++;
    if (!isdigit(*tp)) return kError;
    *gp = (*tp) - '0';
    tp++;
    if (isdigit(*tp)) {
        if (*gp == 0) return kSyntax; // Leading zero.
        *gp *= 10;
        *gp += (*tp) - '0';
        tp++;
    }
    if (isdigit(*tp)) { // Too many digits.
        *gp = 0;
        return kSyntax;
    }
    if (*tp != '\0' && *tp != ' ' && *tp != ')' && *tp !='\'') return kNotParsed;
    *p = tp;
    return kOk;
}

MmResult parse_pin_num(const char **p, uint8_t *pin_num, bool *is_gp) {
    *is_gp = false;

    // First try parsing arg as literal GPnn.
    uint8_t pin_gp = 0;
    MmResult result = parse_gp_pin(p, &pin_gp);
    if (SUCCEEDED(result)) {
        *is_gp = true;
        result = gpio_translate_from_pin_gp(pin_gp, pin_num);
    } else if (result != kSyntax) {
        // If it is not in the format GPnn then treat it as an integer instead.
        // TODO: Currently getint() will longjmp on an error rather than returing an MmResult.
        *pin_num = (uint8_t) getint(*p, 1, GPIO_MAX_PIN_NUM);
        *p = skipexpression(*p);
        result = kOk;
    }
    return result;
}

static inline MmResult parse_picomite_page(const char *p, MmSurfaceId *page_id) {
    *page_id = -1;
    const char *tp;
    if ((tp = checkstring(p, "N"))) {
        *page_id = GRAPHICS_SURFACE_N;
    } else if ((tp = checkstring(p, "F"))) {
        *page_id = GRAPHICS_SURFACE_F;
    } else if ((tp = checkstring(p, "L"))) {
        *page_id = GRAPHICS_SURFACE_L;
    } else { // Allow string expression.
        const char *s = getCstring(p);
        if (strcasecmp(s, "N") == 0) {
            *page_id = GRAPHICS_SURFACE_N;
        } else if (strcasecmp(s, "F") == 0) {
            *page_id = GRAPHICS_SURFACE_F;
        } else if (strcasecmp(s, "L") == 0) {
            *page_id = GRAPHICS_SURFACE_L;
        } else {
            return kSyntax;
        }
    }
    return kOk;
}

MmResult parse_page(const char *p, MmSurfaceId *page_id) {
    MmResult result = kOk;
    switch (mmb_options.simulate) {
        case kSimulateGameMite:
        case kSimulatePicoMiteVga:
            result = parse_picomite_page(p, page_id);
            break;
        default:
            *page_id = getint(p, 0, GRAPHICS_MAX_ID);
            result = kOk;
            break;
    }
    if (SUCCEEDED(result) && !graphics_surface_exists(*page_id)) {
        return kGraphicsInvalidSurface;
    } else {
        return result;
    }
}

MmResult parse_read_page(const char *p, MmSurfaceId *page_id) {
    MmResult result = parse_page(p, page_id);
    return (result == kGraphicsInvalidSurface) ? kGraphicsInvalidReadSurface : result;
}

MmResult parse_write_page(const char *p, MmSurfaceId *page_id) {
    MmResult result = parse_page(p, page_id);
    return (result == kGraphicsInvalidSurface) ? kGraphicsInvalidWriteSurface : result;
}

MmResult parse_blit_id(const char *p, bool existing, MmSurfaceId *blit_id) {
    skipspace(p);
    if (*p == '#') p++;
    if (!*p) return kSyntax;
    if (mmb_options.simulate == kSimulateMmb4l) {
        *blit_id = getint(p, 0, GRAPHICS_MAX_ID);
    } else {
        *blit_id = getint(p, 1, CMM2_BLIT_COUNT) + CMM2_BLIT_BASE;
    }
    if (existing && graphics_surfaces[*blit_id].type == kGraphicsNone) {
        return kGraphicsInvalidSurface;
    }
    return kOk;
}

MmResult parse_sprite_id(const char *p, uint64_t flags, MmSurfaceId *sprite_id) {
    skipspace(p);
    if (*p == '#') p++;
    if (!*p) return kSyntax;
    *sprite_id = sprite_id_to_surface_id(
            getint(p, flags & kParseSpriteIdAllowZero ? 0 : 1, sprite_max_id()));

    // If allowed then 0 does not mean surface 0;
    // it is a special value used by some of the SPRITE() functions.
    if (*sprite_id == 0) return kOk;

    switch (graphics_surfaces[*sprite_id].type) {
        case kGraphicsNone:
            if (flags & kParseSpriteIdMustExist) {
                MMRESULT_RETURN_EX(kGraphicsInvalidSprite, "Invalid sprite: %d", *sprite_id);
            }
            break;
        case kGraphicsInactiveSprite:
        case kGraphicsSprite:
            break;
        default:
            MMRESULT_RETURN_EX(kGraphicsInvalidSprite, "Invalid sprite: %d", *sprite_id);
    }

    return kOk;
}

MmResult parse_filename(const char *p, char *out, size_t out_sz) {
    char *f = getCstring(p);
    MmResult result = path_munge(getCstring(p), out, out_sz);
    ClearSpecificTempMemory(f);
    return result;
}
