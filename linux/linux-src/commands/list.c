#include "../common/version.h"

/* qsort C-string comparison function */ 
static int cstring_cmp(const void *a, const void *b)  {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcasecmp(*ia, *ib);
	/* strcmp functions works exactly as expected from
	comparison function */
}

static void list_tokens(const char *title, const struct s_tokentbl *primary, int num_primary, const char **secondary) {
    int num_secondary = 0;
    const char **ptr = secondary;
    while (*ptr++) num_secondary++;

    const int total = num_primary + num_secondary;

    char **tbl = (char **) GetTempMemory(
            total * sizeof(char *) // Memory for char pointers.
            + total * 20);         // Memory for 20 character strings.

    // Initialise char pointers.
    for (int i = 0; i < total; i++) {
        tbl[i] = (char *) (tbl + total + i * 20);
    }

    // Copy primary items.
    for (int i = 0; i < num_primary; i++) {
        strcpy(tbl[i], primary[i].name);
    }

    // Copy secondary items.
    char buf[256];
    for (int i = num_primary; i < total; i++) {
        sprintf(buf, "%s (*)", secondary[i - num_primary]);
        strcpy(tbl[i], buf);
    }

    // Sort the table.
    qsort(tbl, total, sizeof(char *), cstring_cmp);

    int step = 5;
    for (int i = 0; i < total; i += step) {
        for (int k = 0; k < step; k++) {
            if (i + k < total) {
                MMPrintString(tbl[i + k]);
                if (k != (step - 1))
                    for (int j = strlen(tbl[i + k]); j < 19; j++) MMPrintString(" "); //putConsole(' ');
            }
        }
        MMPrintString("\r\n");
    }
    sprintf(buf, "Total of %d %s using %d slots\r\n", total, title, num_primary);
    MMPrintString(buf);
}

static void list_commands(char *p) {
    const char *secondary_commands[] = {
            "foo",
            "bar",
            NULL };
    list_tokens("commands", commandtbl, CommandTableSize - 1, secondary_commands);
}

static void list_functions(char *p) {
    const char *secondary_functions[] = {
            "foo",
            "bar",
            NULL };
    list_tokens("functions", tokentbl, TokenTableSize - 1, secondary_functions);
}

static void list_program(char *p, int all) {
    ListProgram(ProgMemory, all);
}

void cmd_list(void) {
    char *p;
    if (p = checkstring(cmdline, "ALL")) {
        list_program(p, true);
    } else if (p = checkstring(cmdline, "COMMANDS")) {
        list_commands(p);
    } else if (p = checkstring(cmdline, "FUNCTIONS")) {
        list_functions(p);
    } else {
        list_program(cmdline, false);
        p = cmdline;
    }
    checkend(p);
}
