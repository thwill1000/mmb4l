#include "../common/console.h"
#include "../common/error.h"
#include "../common/parse.h"
#include "../common/program.h"
#include "../common/utility.h"
#include "../common/version.h"

void option_list(char *p); // cmd_option.c

/* qsort C-string comparison function */
static int cstring_cmp(const void *a, const void *b)  {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcasecmp(*ia, *ib);
}

static void list_tokens(const char *title, const struct s_tokentbl *primary, const char **secondary) {
    int num_primary = 0;
    struct s_tokentbl *ptok = (struct s_tokentbl *) primary;
    while (ptok->name[0] != '\0') {
        if (ptok->fptr != cmd_dummy) num_primary++;
        ptok++;
    }

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
    ptok = (struct s_tokentbl *) primary;
    for (int i = 0; ptok->name[0] != '\0'; ) {
        if (ptok->fptr != cmd_dummy) strcpy(tbl[i++], ptok->name);
        ptok++;
    }

    // Copy secondary items.
    char buf[STRINGSIZE];
    for (int i = num_primary; i < total; i++) {
        sprintf(buf, "%s (*)", secondary[i - num_primary]);
        strcpy(tbl[i], buf);
    }

    // Sort the table.
    qsort(tbl, total, sizeof(char *), cstring_cmp);

    int step = Option.Width / 20;
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
    sprintf(buf, "Total of %d %s using %d slots\r\n\r\n", total, title, num_primary);
    MMPrintString(buf);
}

static void list_commands() {
    const char *secondary_commands[] = {
            // "foo",
            // "bar",
            (char *) NULL };
    list_tokens("commands", commandtbl, secondary_commands);
}

static void list_functions() {
    const char *secondary_functions[] = {
            // "foo",
            // "bar",
            (char *) NULL };
    list_tokens("functions", tokentbl, secondary_functions);
}

static void list_file(const char *filename, int all) {
    if (!filename && CurrentFile[0] == '\0') {
        error("Nothing to list");
        return;
    }

    char file_path[STRINGSIZE];
    munge_path(filename ? filename : CurrentFile, file_path, STRINGSIZE);
    error_check();

    char line_buffer[STRINGSIZE];
    int list_count = 1;
    int file_num = FindFreeFileNbr();
    MMfopen(file_path, "rb", file_num);
    while (!MMfeof(file_num)) {
        memset(line_buffer, 0, STRINGSIZE);
        MMgetline(file_num, line_buffer);
        for (size_t i = 0; i < strlen(line_buffer); i++) {
            if (line_buffer[i] == TAB) line_buffer[i] = ' ';
        }
        MMPrintString(line_buffer);
        list_count += strlen(line_buffer) / Option.Width;
        ListNewLine(&list_count, all);
    }
    MMfclose(file_num);

    // Ensure listing is followed by an empty line.
    if (strcmp(line_buffer, "") != 0) MMPrintString("\r\n");
}

static void list_flash(int all) {
    if (CurrentFile[0] == '\0') {
        error("Nothing to list");
        return;
    }

    // Make sure we are looking at the latest (on disk) version of the program.
    if (!program_load_file(CurrentFile)) return;

    ListProgram(ProgMemory, all);

    MMPrintString("\r\n");
}

static void list_csubs(int all) {
    if (CurrentFile[0] == '\0') {
        error("Nothing to list");
        return;
    }

    // Make sure we are looking at the latest (on disk) version of the program.
    if (!program_load_file(CurrentFile)) return;

    program_list_csubs(all);
}

static void list_options() {
    option_list("");
}

void cmd_list(void) {
    char *p;
    skipspace(cmdline);

    // Use the current console dimensions for the output of the LIST command.
    console_get_size(&Option.Width, &Option.Height);

    if (parse_is_end(cmdline)) {
        list_file(NULL, false);
    } else if ((p = checkstring(cmdline, "COMMANDS"))) {
        if (!parse_is_end(p)) ERROR_SYNTAX;
        list_commands();
    } else if ((p = checkstring(cmdline, "CSUB")) || (p = checkstring(cmdline, "CSUBS"))) {
        if (parse_is_end(p)) {
            list_csubs(false);
        } else if ((p = checkstring(p, "ALL"))) {
            if (!parse_is_end(p)) ERROR_SYNTAX;
            list_csubs(true);
        } else {
            ERROR_SYNTAX;
        }
    } else if ((p = checkstring(cmdline, "FLASH"))) {
        if (parse_is_end(p)) {
            list_flash(false);
        } else if ((p = checkstring(p, "ALL"))) {
            if (!parse_is_end(p)) ERROR_SYNTAX;
            list_flash(true);
        } else {
            ERROR_SYNTAX;
        }
    } else if ((p = checkstring(cmdline, "FUNCTIONS"))) {
        if (!parse_is_end(p)) ERROR_SYNTAX;
        list_functions();
    } else if ((p = checkstring(cmdline, "OPTIONS"))) {
        if (!parse_is_end(p)) ERROR_SYNTAX;
        list_options();
    } else {
        if ((p = checkstring(cmdline, "ALL"))) {
            if (parse_is_end(p)) {
                list_file(NULL, true);
            } else {
                list_file(getCstring(p), true);
            }
        } else {
            list_file(getCstring(cmdline), false);
        }
    }
}
