#include "../common/utility.h"
#include "../common/version.h"

void ListNewLine(int *ListCnt, int all);

/* qsort C-string comparison function */
static int cstring_cmp(const void *a, const void *b)  {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcasecmp(*ia, *ib);
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
        if (primary[i].fptr == cmd_dummy) {
            strcat(tbl[i], " (d)");
        }
    }

    // Copy secondary items.
    char buf[256];
    for (int i = num_primary; i < total; i++) {
        sprintf(buf, "%s (*)", secondary[i - num_primary]);
        strcpy(tbl[i], buf);
    }

    // Sort the table.
    qsort(tbl, total, sizeof(char *), cstring_cmp);

    int step = 4;
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

static void list_commands() {
    const char *secondary_commands[] = {
            "foo",
            "bar",
            NULL };
    list_tokens("commands", commandtbl, CommandTableSize - 1, secondary_commands);
}

static void list_functions() {
    const char *secondary_functions[] = {
            "foo",
            "bar",
            NULL };
    list_tokens("functions", tokentbl, TokenTableSize - 1, secondary_functions);
}

static void list_file(const char *filename, int all) {
    //printf("list_file(%s, %d)\n", filename ? filename : "null", all);

    if (!filename && CurrentFile[0] == '\0') {
        MMPrintString("Nothing to list\r\n");
        return;
    }

    char file_path[STRINGSIZE];
    canonicalize_path(filename ? filename : CurrentFile, file_path, STRINGSIZE);

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
}

static void list_flash(int all) {
    ListProgram(ProgMemory, all);
}

void cmd_list(void) {
    getargs(&cmdline, 3, " ,");

    if (argc == 0) {
        list_file(NULL, false);
    } else if (checkstring(argv[0], "COMMANDS")) {
        if (argc == 1) {
            list_commands();
        } else {
            error("Syntax");
        }
    } else if (checkstring(argv[0], "FLASH")) {
        if (argc == 1) {
            list_flash(false);
        } else if (argc == 3 && checkstring(argv[2], "ALL")) {
            list_flash(true);
        } else {
            error("Syntax");
        }
    } else if (checkstring(argv[0], "FUNCTIONS")) {
        if (argc == 1) {
            list_functions();
        } else {
            error("Syntax");
        }
    } else {
        if (argc == 1 && checkstring(argv[0], "ALL")) {
            list_file(NULL, true);
        } else if (argc == 1) {
            list_file(getCstring(argv[0]), false);
        } else if (argc == 3 && checkstring(argv[0], "ALL")) {
            list_file(getCstring(argv[2]), true);
        } else {
            error("Syntax");
        }
    }
}
