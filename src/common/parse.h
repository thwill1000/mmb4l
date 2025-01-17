/*-*****************************************************************************

MMBasic for Linux (MMB4L)

parse.h

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

#if !defined(MMB4L_PARSE_H)
#define MMB4L_PARSE_H

#include "graphics.h"
#include "mmresult.h"
#include "../core/commandtbl.h"

#include <stdbool.h>

#define  MAX_PARAMETERS  32

typedef struct {
    uint8_t name_offset;
    uint8_t name_len;
    uint8_t type;
    bool    array;
} ParameterSignature;

typedef struct {
    const char *addr;
    CommandToken token;
    uint8_t name_offset;
    uint8_t name_len;
    uint8_t type;
    uint8_t num_params;
    ParameterSignature params[MAX_PARAMETERS];
} FunctionSignature;

bool parse_is_end(const char *p);

/**
 * @brief Does the next text in an element (a basic statement) correspond to an alpha string.
 *
 * Leading whitespace is skipped and the string must be terminated by a non-name character.
 *
 * @param[in] p  Parse from this pointer.
 * @return       If found, then pointer to the next non-space character after the match,
 *               otherwise NULL.
 */
const char *parse_check_string(const char *p, const char *tkn);

bool parse_bool(const char *p);
int parse_colour(const char *p, bool allow_bright);
int parse_file_number(const char *p, bool allow_zero);

/**
 * @brief Does the string match the pattern for a LONGSTRING, i.e. name%() or name().
 */
bool parse_matches_longstring_pattern(const char *s);

/**
 * @brief Parses a name/identifier ignoring any leading space characters.
 *
 * @param[in,out] p     Parse from this pointer.
 *                      On exit points at the character following the name.
 * @param[out]    name  Buffer to store the UPPER-CASE name in, should be
 *                      at least MAXVARLEN + 1 chars.
 * @return              kOk on success.
 *                      kSyntax if the first non-space character is not valid to
 *                      start a name.
 *                      kNameTooLong if the name is more than MAXVARLEN chars.
 */
MmResult parse_name(const char **p, char *name);

/**
 * @brief Transforms star '*' and bang '!' commands in the input buffer
 *        to standard MMBasic commands.
 *
 * @param[in,out] The buffer to transform, usually called with the global 'inpbuf'.
 */
MmResult parse_transform_input_buffer(char *input);

/**
 * @brief Parses a SUB/FUNCTION signature from tokenised code ignoring any leading space
 *        characters.
 *
 * @param[in,out] p          Parse from this pointer.
 *                           On exit points at the character following the signature.
 * @param[out]    signature  Structure to store the signature in.
 * @return                   kOk on success.
 */
MmResult parse_fn_sig(const char **p, FunctionSignature *signature);

/**
 * @brief Parses a literal GPnn specification from tokenised code ignoring any leading space
 *        characters.
 *
 * @param[in,out] p       Parse from this pointer.
 *                        On exit points at the character following the signature.
 * @param[out]    pin_gp  The parsed GPnn number.
 * @return                kOk on success.
 *                        kSyntax if a badly formed GPnn specification is parsed.
 *                        kNotParsed if a GPnn specification is not parsed.
 */
MmResult parse_gp_pin(const char **p, uint8_t *pin_gp);

/**
 * @brief Parses a pin number, either literal GPnn or integer from tokenised code ignoring any
 *        leading space characters.
 *
 * @param[in,out] p        Parse from this pointer.
 *                         On exit points at the character following the signature.
 * @param[out]    pin_num  The parsed hardware pin number. If parsed from a GPnn specification it
                           will have been converted to a hardware pin number.
 * @param[out]    is_gp    True if pin number parsed from GPnn specification, otherwise false.
 * @return                 kOk on success.
 *                         kSyntax if a badly formed GPnn specification is parsed.
 *                         kNotParsed if a GPnn specification is not parsed.
 */
MmResult parse_pin_num(const char **p, uint8_t *pin_num, bool *gp);

/**
 * Parses a page/surface ID.
 *
 * @param[in]   p        Parse from this pointer.
 * @param[out]  page_id  The page/surface ID.
 * @return               kGraphicsInvalidSurface if the page/surface does not exist.
 */
MmResult parse_page(const char *p, MmSurfaceId *page_id);

/**
 * Parses a READ page/surface ID.
 *
 * @param[in]   p        Parse from this pointer.
 * @param[out]  page_id  The page/surface ID.
 * @return               kGraphicsInvalidReadSurface if the page/surface does not exist.
 */
MmResult parse_read_page(const char *p, MmSurfaceId *page_id);

/**
 * Parses a WRITE page/surface ID.
 *
 * @param[in]   p        Parse from this pointer.
 * @param[out]  page_id  The page/surface ID.
 * @return               kGraphicsInvalidWriteSurface if the page/surface does not exist.
 */
MmResult parse_write_page(const char *p, MmSurfaceId *page_id);

/**
 * Parses a surface ID for use as the 'this' argument in a BLIT command.
 *
 * @param[in]   p         Parse from this pointer.
 * @param[in]   existing  Return an error if surface does not exist.
 * @param[out]  blit_id   On exit, the surface ID.
 * @return                kGraphicsSurfaceNotFound if the surface does not exist
 *                        and existing == true.
 */
MmResult parse_blit_id(const char *p, bool existing, MmSurfaceId *blit_id);

typedef enum {
   kParseSpriteIdMustExist = 0x1, // Error if {In}active sprite with given ID does not exist.
   kParseSpriteIdAllowZero = 0x2  // Not an error if ID == 0.
} ParseSpriteIdFlags;

/**
 * Parses a surface ID for use as the 'this' argument in a SPRITE command/function.
 *
 * @param[in]   p           Parse from this pointer.
 * @param[in]   flags       Bitwise OR of ParseSpriteIdFlags
 * @param[out]  sprite_id   On exit, the surface ID.
 * @return                  kGraphicsSurfaceNotFound if the surface does not exist and
 *                          existing == true.
 *                          kInvalidSprite if the surface does exist but is not a sprite.
 */
MmResult parse_sprite_id(const char *p, uint64_t flags, MmSurfaceId *sprite_id);

/**
 * Parses a file path/name.
 *
 * @param[in]   p       Parse from this pointer.
 * @param[out]  out     Buffer which on exit will contain the parsed file path/name.
 *                      This will already have been munged; see path_munge().
 * @param[in]   out_sz  Size of the \p out buffer.
 */
MmResult parse_filename(const char *p, char *out, size_t out_sz);

#endif
