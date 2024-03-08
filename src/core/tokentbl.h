/*-*****************************************************************************

MMBasic for Linux (MMB4L)

MMBasic.c

Copyright 2011-2024 Geoff Graham, Peter Mather and Thomas Hugo Williams.

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

#if !defined(TOKENTBL_H)
#define TOKENTBL_H

#include "commandtbl.h"

#define GetTokenValue(s)  tokentbl_get(s)
#define TokenTableSize    tokentbl_size

/** Gets the type of a token. */
#define tokentype(i)  ((i >= C_BASETOKEN && i < TokenTableSize - 1 + C_BASETOKEN) ? (tokentbl[i - C_BASETOKEN].type) : 0)

/** Gets the function pointer of a token. */
#define tokenfunction(i)  ((i >= C_BASETOKEN && i < TokenTableSize - 1 + C_BASETOKEN) ? (tokentbl[i - C_BASETOKEN].fptr) : (tokentbl[0].fptr))

/** Gets the name of a token. */
#define tokenname(i)  ((i >= C_BASETOKEN && i < TokenTableSize - 1 + C_BASETOKEN) ? (tokentbl[i - C_BASETOKEN].name) : "")

void fun_abs(void);
void fun_acos(void);
void fun_asc(void);
void fun_asin(void);
void fun_atn(void);
void fun_at(void);
void fun_bin2str(void);
void fun_bin(void);
void fun_bound(void);
void fun_call(void);
void fun_choice(void);
void fun_chr(void);
void fun_cint(void);
void fun_cos(void);
void fun_cwd(void);
void fun_datetime(void);
void fun_date(void);
void fun_day(void);
void fun_deg(void);
void fun_dir(void);
void fun_eof(void);
void fun_epoch(void);
void fun_errmsg(void);
void fun_errno(void);
void fun_eval(void);
void fun_exp(void);
void fun_field(void);
void fun_fix(void);
void fun_format(void);
void fun_hex(void);
void fun_hres(void);
void fun_inkey(void);
void fun_inputstr(void);
void fun_instr(void);
void fun_int(void);
void fun_json(void);
void fun_lcase(void);
void fun_lcompare(void);
void fun_left(void);
void fun_len(void);
void fun_lgetbyte(void);
void fun_lgetstr(void);
void fun_linstr(void);
void fun_llen(void);
void fun_loc(void);
void fun_lof(void);
void fun_log(void);
void fun_math(void);
void fun_max(void);
void fun_mid(void);
void fun_min(void);
void fun_mmcmdline(void);
void fun_mmdevice(void);
void fun_mminfo(void);
void fun_oct(void);
void fun_peek(void);
void fun_pi(void);
void fun_pos(void);
void fun_rad(void);
void fun_rgb(void);
void fun_right(void);
void fun_rnd(void);
void fun_sgn(void);
void fun_sin(void);
void fun_space(void);
void fun_sqr(void);
void fun_str2bin(void);
void fun_string(void);
void fun_str(void);
void fun_tab(void);
void fun_tan(void);
void fun_timer(void);
void fun_time(void);
void fun_ucase(void);
void fun_val(void);
void fun_version(void);
void fun_vres(void);

void op_add(void);
void op_and(void);
void op_divint(void);
void op_div(void);
void op_equal(void);
void op_exp(void);
void op_gte(void);
void op_gt(void);
void op_invalid(void);
void op_inv(void);
void op_lte(void);
void op_lt(void);
void op_mod(void);
void op_mul(void);
void op_ne(void);
void op_not(void);
void op_or(void);
void op_shiftleft(void);
void op_shiftright(void);
void op_subtract(void);
void op_xor(void);

void tokentbl_init();
int tokentbl_get(const char *s);

extern const struct s_tokentbl tokentbl[];
extern int tokentbl_size;

// Store commonly used tokens for faster token checking.
extern char tokenTHEN, tokenELSE, tokenGOTO, tokenEQUAL, tokenTO, tokenSTEP;
extern char tokenWHILE, tokenUNTIL, tokenGOSUB, tokenAS, tokenFOR;

#endif
