#include "console.h"
#include "file.h"
#include "version.h"

#define NBRPROGKEYS  12  // Number of programmable function keys.
#define MAXKEYLEN    24  // Maximum length of programmable function keys.
#define HISTORY_SIZE  4 * STRINGSIZE

typedef int bool;

typedef struct {
    char buf[MAXKEYLEN + 3];
    int char_index;
    bool buf_edited;
    int history_idx;
    bool insert;
    int start_line;
    int max_chars;
    bool save_line;
} PromptState;

char FunKey[NBRPROGKEYS][MAXKEYLEN + 1] =
    {                   // data storage for the function keys
        {"FILES\r\n"},  // Run immediately
        {"RUN\r\n"},
        {"LIST\r\n"},
        {"EDIT\r\n"},
        //
        {"AUTOSAVE \"\"\x82"},  // Position cursor inside pair of double quotes
        {"XMODEM RECEIVE \"\"\x82"},
        {"XMODEM SEND \"\"\x82"},
        {"EDIT \"\"\x82"},
        {"LIST \"\"\x82"},
        {"RUN \"\"\x82"},
        {""},
        {""}};

static char history[HISTORY_SIZE]; // used to store the last command in case it is
                                          // needed by the EDIT command

static const char NO_ITEM[] = "";

/** Displays the contents of the 'history' buffer. */
static void dump_history() {
    char s[STRINGSIZE];
    char *p = history;
    char *start = p;
    MMPrintString("[BEGIN]\r\n");
    for (; p < history + HISTORY_SIZE; ++p) {
        if (*p == '\0') {
            int len = p - start;
            if (len == 0) break;
            memset(s, 0, STRINGSIZE);
            memcpy(s, start, len);
            MMPrintString("~");
            MMPrintString(s);
            MMPrintString("~\r\n");
            start = p + 1;
        }
    }
    MMPrintString("[END]\r\n");
    MMPrintString("\r\n");
}

/** Gets an item from the 'history' buffer. */
static char *get_history_item(int idx) {
    if (idx < 0) {
        return (char *) NO_ITEM;
    }

    int current = 0;
    char *p = history;
    char *item;

    for (;;) {
        item = p;
        if (current == idx) break;
        p += strlen(p);
        p++;
        if (p >= history + HISTORY_SIZE || *p == '\0') break;
        current++;
    }

    return item;
}

/** Gets the number of items in the 'history' buffer. */
static int get_history_count() {
    int count = 0;
    char *p = history;

    for (;;) {
        if (p >= history + HISTORY_SIZE || *p == '\0') break;
        p += strlen(p);
        p++;
        count++;
    }

    return count;
}

/**
 * Inserts a string into the start of the 'history' buffer.
 * The buffer is a sequence of strings separated by a zero byte.
 * using the up arrow usere can call up the last few commands executed.
 */
void put_history_item(char *s) {
    if (strcmp(history, s) == 0) return;  // Don't store duplicates.
    int slen = strlen(s);
    if (slen < 1 || slen > HISTORY_SIZE - 1) return;
    slen++;

    // Shift the contents of the buffer to the right.
    for (int i = HISTORY_SIZE - 1; i >= slen; i--) {
        history[i] = history[i - slen];
    }

    // Insert new string at the beginning.
    strcpy(history, s);

    // Zero the end of the buffer.
    for (int i = HISTORY_SIZE - 1; history[i]; i--) {
        history[i] = '\0';
    }
}

static void handle_backspace(PromptState *pstate) {
    if (pstate->char_index <= 0) return;

    pstate->buf_edited = true;
    int i = pstate->char_index - 1;
    for (char *p = inpbuf + i; *p; p++) {
        *p = *(p + 1);  // remove the char from inpbuf
    }
    while (pstate->char_index) {
        MMputchar('\b');
        pstate->char_index--;
    }  // go to the beginning of the line
    MMPrintString(inpbuf);
    MMputchar(' ');
    MMputchar('\b');  // display the line and erase the last char
    for (pstate->char_index = strlen(inpbuf); pstate->char_index > i; pstate->char_index--) {
        MMputchar('\b');  // return the cursor to the right position
    }
}

static void handle_delete(PromptState *pstate) {
    if (pstate->char_index >= strlen(inpbuf)) return;

    pstate->buf_edited = true;
    int i = pstate->char_index;
    for (char *p = inpbuf + i; *p; p++) {
        *p = *(p + 1);  // remove the char from inpbuf
    }
    while (pstate->char_index) {
        MMputchar('\b');
        pstate->char_index--;
    }  // go to the beginning of the line
    MMPrintString(inpbuf);
    MMputchar(' ');
    MMputchar('\b');  // display the line and erase the last char
    for (pstate->char_index = strlen(inpbuf); pstate->char_index > i; pstate->char_index--) {
        MMputchar('\b');  // return the cursor to the right position
    }
}

static void insert_history_item(PromptState *pstate) {
    // Update input buffer from the history.
    strcpy(inpbuf, get_history_item(pstate->history_idx));

    // Backspace cursor until just after the prompt.
    while (pstate->char_index) {
        MMputchar('\b');
        pstate->char_index--;
    }

    // Display the line.
    MMPrintString(inpbuf);

    // Handle the line being too long.
    if (strlen(inpbuf) + pstate->start_line >= pstate->max_chars) {
        error("Line is too long to edit");
    }

    // Erase everything else on the line.
    int num_spaces = pstate->max_chars - pstate->start_line - strlen(inpbuf);
    for (int i = 0; i < num_spaces; ++i) MMputchar(' ');
    for (int i = 0; i < num_spaces - 1; ++i) MMputchar('\b');

    // Update 'char_index' to reflect new line contents.
    pstate->char_index = strlen(inpbuf);
}

static void handle_down(PromptState *pstate) {
    // [DOWN] is non-functional once the user starts to edit the buffer.
    if (pstate->buf_edited) return;

    pstate->history_idx--;
    if (pstate->history_idx < 0) pstate->history_idx = -1;
    insert_history_item(pstate);
}

static void handle_end(PromptState *pstate) {
    while (pstate->char_index < strlen(inpbuf)) {
        MMputchar(inpbuf[pstate->char_index++]);
    }
}

static void handle_function_key(PromptState *pstate) {
    if (*FunKey[pstate->buf[0] - 0x91]) {
        // Copy a function key string into the buffer.
        strcpy(&pstate->buf[1], (char *)FunKey[pstate->buf[0] - 0x91]);
    }
}

static void handle_home(PromptState *pstate) {
    if (pstate->char_index <= 0) return;

    if (pstate->char_index == strlen(inpbuf)) {
        pstate->insert = true;
    }

    while (pstate->char_index) {
        MMputchar('\b');
        pstate->char_index--;
    }
}

static void handle_insert(PromptState *pstate) {
    pstate->insert = !pstate->insert;
}

static void handle_left(PromptState *pstate) {
    if (pstate->char_index <= 0) return;

    if (pstate->char_index == strlen(inpbuf)) {
        pstate->insert = true;
    }
    MMputchar('\b');
    pstate->char_index--;
}

static void handle_newline(PromptState *pstate) {
    pstate->save_line = 1;
}

static void handle_other(PromptState *pstate) {
    if (pstate->buf[0] < ' ' || pstate->buf[0] >= 0x7f) return;

    pstate->buf_edited = true;  // this means that something was typed
    // int i = pstate->char_index;
    int j = strlen(inpbuf);

    if (pstate->insert) {
        if (strlen(inpbuf) >= pstate->max_chars - 1) return;  // sorry, line full
        for (char *p = inpbuf + strlen(inpbuf); j >= pstate->char_index; p--, j--) {
            *(p + 1) = *p;
        }
        inpbuf[pstate->char_index] = pstate->buf[0];  // insert the char
        MMPrintString(&inpbuf[pstate->char_index]);   // display new part of
                                                      // the line
        pstate->char_index++;
        for (j = strlen(inpbuf); j > pstate->char_index; j--) {
            MMputchar('\b');  // return the cursor to the right position
        }
    } else {
        inpbuf[strlen(inpbuf) + 1] = 0;  // incase we are adding to the end
                                         // of the string
        inpbuf[pstate->char_index++] = pstate->buf[0];  // overwrite the char
        MMputchar(pstate->buf[0]);                      // display it
        if (pstate->char_index + pstate->start_line >=
            pstate->max_chars) {  // has the input gone beyond the
                                  // end of the line?
            MMgetline(0, inpbuf);  // use the old fashioned way
                                   // of getting the line
            // if(autoOn && atoi(inpbuf) > 0) autoNext =
            // atoi(inpbuf) + autoIncr;
            pstate->save_line = 1;
        }
    }
}

static void handle_right(PromptState *pstate) {
    if (pstate->char_index >= strlen(inpbuf)) return;

    MMputchar(inpbuf[pstate->char_index]);
    pstate->char_index++;
}

static void handle_up(PromptState *pstate) {
    // [UP] is non-functional once the user starts to edit the buffer.
    if (pstate->buf_edited) return;

    pstate->history_idx++;
    if (pstate->history_idx >= get_history_count()) pstate->history_idx--;
    insert_history_item(pstate);
}

void prompt_get_input(void) {
    int width, height;
    console_get_size(&width, &height); // TODO: check return value

    PromptState state = { 0 };
    state.char_index = strlen(inpbuf); // get the current cursor position in the line
    state.start_line = MMCharPos - 1;  // save the current cursor position
    state.max_chars = width;
    state.history_idx = -1;

    MMPrintString(inpbuf);  // display the contents of the input buffer (if any)

    if (strlen(inpbuf) >= state.max_chars) {
        error("Line is too long to edit");
    }

    int ch;
    while (1) {
        ch = MMgetchar(); // MMgetchar(1);
        if (ch == TAB) {
            strcpy(state.buf, "        ");
            switch (Option.Tab) {
                case 2:
                    state.buf[2 - (state.char_index % 2)] = 0;
                    break;
                case 3:
                    state.buf[3 - (state.char_index % 3)] = 0;
                    break;
                case 4:
                    state.buf[4 - (state.char_index % 4)] = 0;
                    break;
                case 8:
                    state.buf[8 - (state.char_index % 8)] = 0;
                    break;
            }
        } else {
            state.buf[0] = ch;
            state.buf[1] = '\0';
        }

        do {
            switch (state.buf[0]) {
                case '\r':
                case '\n':
                    handle_newline(&state);
                    break;

                case '\b':
                    handle_backspace(&state);
                    break;

                // case CTRLKEY('S'):
                case LEFT:
                    handle_left(&state);
                    break;

                // case CTRLKEY('D'):
                case RIGHT:
                    handle_right(&state);
                    break;

                // case CTRLKEY(']'):
                case DEL:
                    handle_delete(&state);
                    break;

                // case CTRLKEY('N'):
                case INSERT:
                    handle_insert(&state);
                    break;

                // case CTRLKEY('U'):
                case HOME:
                    handle_home(&state);
                    break;

                // case CTRLKEY('K'):
                case END:
                    handle_end(&state);
                    break;

                case 0x91:
                case 0x92:
                case 0x93:
                case 0x94:
                case 0x95:
                case 0x96:
                case 0x97:
                case 0x98:
                case 0x99:
                case 0x9a:
                case 0x9b:
                case 0x9c:
                    handle_function_key(&state);
                    break;

                // case CTRLKEY('E'):
                case UP:
                    handle_up(&state);
                    break;

                // case CTRLKEY('X'):
                case DOWN:
                    handle_down(&state);
                    break;

                default:
                    handle_other(&state);
                    break;
            }

            if (state.save_line) goto saveline;

            for (int i = 0; i < MAXKEYLEN + 1; i++) {
                state.buf[i] = state.buf[i + 1];  // shuffle down the buffer to get the next char
            }
        } while (*state.buf);

        if (state.char_index == strlen(inpbuf)) {
            state.insert = false;
        }
    }

saveline:
    MMPrintString("\r\n");

    put_history_item(inpbuf);
}