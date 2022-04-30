#include "../common/mmb4l.h"
#include "../common/error.h"

#define ERROR_NO_DATA                     error_throw_ex(kError, "No DATA to read")
#define ERROR_RESTORE_CALLED_BEFORE_SAVE  error_throw_ex(kError, "READ RESTORE called before READ SAVE")

static char *cmd_read_cached_next_data_line = NULL;
static int cmd_read_cached_next_data = 0;

void cmd_read_clear_cache() {
    cmd_read_cached_next_data_line = NULL;
    cmd_read_cached_next_data = 0;
}

static void cmd_read_save(void) {
    cmd_read_cached_next_data_line = NextDataLine;
    cmd_read_cached_next_data = NextData;
}

static void cmd_read_restore(void) {
    if (!cmd_read_cached_next_data_line && !cmd_read_cached_next_data) {
        ERROR_RESTORE_CALLED_BEFORE_SAVE;
    }
    NextDataLine = cmd_read_cached_next_data_line;
    NextData = cmd_read_cached_next_data;
}

void cmd_read_data(void) {
    int i, len;
    char *p, datatoken, *lineptr = NULL, *x;
    char *vtbl[MAX_ARG_COUNT];
    int vtype[MAX_ARG_COUNT];
    int vsize[MAX_ARG_COUNT];
    int vcnt, vidx;
    getargs(&cmdline, (MAX_ARG_COUNT * 2) - 1, ",");                // getargs macro must be the first executable stmt in a block

    if (argc == 0) ERROR_SYNTAX;

    // step through the arguments and save the pointer and type
    for(vcnt = i = 0; i < argc; i++) {
        if(i & 0x01) {
            if (*argv[i] != ',') ERROR_SYNTAX;
        }
        else {
            vtbl[vcnt] = findvar(argv[i], V_FIND);
            if(vartbl[VarIndex].type & T_CONST) ERROR_CANNOT_CHANGE_A_CONSTANT;
            vtype[vcnt] = TypeMask(vartbl[VarIndex].type);
            vsize[vcnt] = vartbl[VarIndex].size;
            vcnt++;
        }
    }

    // setup for a search through the whole memory
    vidx = 0;
    datatoken = GetCommandValue("Data");
    p = lineptr = NextDataLine;
    if (*p == 0xff) ERROR_NO_DATA;  // error if there is no program

  // search looking for a DATA statement.  We keep returning to this point until all the data is found
search_again:
    while(1) {
        if(*p == 0) p++;                                            // if it is at the end of an element skip the zero marker
        if(*p == 0 || *p == 0xff) ERROR_NO_DATA;                    // end of the program and we still need more data
        if(*p == T_NEWLINE) lineptr = p++;
        if(*p == T_LINENBR) p += 3;
        skipspace(p);
        if(*p == T_LABEL) {                                         // if there is a label here
            p += p[1] + 2;                                          // skip over the label
            skipspace(p);                                           // and any following spaces
        }
        if(*p == datatoken) break;                                  // found a DATA statement
        while(*p) p++;                                              // look for the zero marking the start of the next element
    }
    NextDataLine = lineptr;
    p++;                                                            // step over the token
    skipspace(p);
    if(!*p || *p == '\'') { CurrentLinePtr = lineptr; ERROR_NO_DATA; }

        // we have a DATA statement, first split the line into arguments
        {                                                           // new block, the getargs macro must be the first executable stmt in a block
        getargs(&p, (MAX_ARG_COUNT * 2) - 1, ",");
        if((argc & 1) == 0) { CurrentLinePtr = lineptr; ERROR_SYNTAX; }
        // now step through the variables on the READ line and get their new values from the argument list
        // we set the line number to the number of the DATA stmt so that any errors are reported correctly
        while(vidx < vcnt) {
            // check that there is some data to read if not look for another DATA stmt
            if(NextData > argc) {
                skipline(p);
                NextData = 0;
                goto search_again;
            }
            x = CurrentLinePtr;
            CurrentLinePtr = lineptr;
            if(vtype[vidx] & T_STR) {
                char *p1, *p2;
                if(*argv[NextData] == '"') {                               // if quoted string
                    for(len = 0, p1 = vtbl[vidx], p2 = argv[NextData] + 1; *p2 && *p2 != '"'; len++, p1++, p2++) {
                       *p1 = *p2;                                   // copy up to the quote
                    }
                } else {                                            // else if not quoted
                    for(len = 0, p1 = vtbl[vidx], p2 = argv[NextData]; *p2 && *p2 != '\'' ; len++, p1++, p2++) {
                        if(*p2 < 0x20 || *p2 >= 0x7f) ERROR_INVALID_CHARACTER;
                        *p1 = *p2;                                  // copy up to the comma
                    }
                }
                if (len > vsize[vidx]) ERROR_STRING_TOO_LONG;
                *p1 = 0;                                            // terminate the string
                CtoM(vtbl[vidx]);                                   // convert to a MMBasic string
            }
            else if(vtype[vidx] & T_INT)
                *((long long int *)vtbl[vidx]) = getinteger(argv[NextData]); // much easier if integer variable
            else
                *((MMFLOAT *)vtbl[vidx]) = getnumber(argv[NextData]);      // same for numeric variable

            vidx++;
            NextData += 2;
        }
    }
}

void cmd_read(void) {
    if (checkstring(cmdline, "SAVE")) {
        cmd_read_save();
    } else if (checkstring(cmdline, "RESTORE")) {
        cmd_read_restore();
    } else {
        cmd_read_data();
    }
}
