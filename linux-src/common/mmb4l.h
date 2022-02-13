#if !defined MMB4L_H
#define MMB4L_H

#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../../Version.h"
#include "../Configuration.h"
#include "../../MMBasic/MMBasic.h"
#include "../../MMBasic/VarTable.h"
#include "options.h"
#include "memory.h"

#define CONFIG_TITLE      0
#define CONFIG_LOWER      1
#define CONFIG_UPPER      2

#define PI_VALUE  3.14159265358979323

#define ClearSavedVars()    {}
#define TestStackOverflow()  {}

// Aliases for MMBasic's global variables, all prefixed with g_
#define g_break_key        BreakKey
#define g_current_var_idx  VarIndex
#define g_float_rtn        fret
#define g_integer_rtn      iret
#define g_rtn_type         targ
#define g_string_rtn       sret
#define g_subfun           subfun
#define g_var_tbl          vartbl

void CheckAbort(void);
void FlashWriteInit(char *p, int nbr);
char *GetIntAddress(char *p);
void ListProgram(char *p, int all);
char *llist(char *b, char *p);
int MMgetchar(void);
void cmd_dummy(void);
void op_equal(void);

#define JMP_BREAK  1
#define JMP_END    2
#define JMP_ERROR  3
#define JMP_NEW    4
#define JMP_QUIT   5
#define JMP_UNEXPECTED  999

extern uint8_t mmb_exit_code;
extern Options mmb_options;

extern char *CFunctionFlash;
extern char DimUsed;
extern char *errorstack[MAXGOSUB];
extern int gosubindex;
extern char *gosubstack[MAXGOSUB];
extern int IgnorePIN;
extern int MMCharPos;
extern int WatchdogSet;

#endif
