#include "../common/version.h"

extern char CurrentFile[STRINGSIZE];

void cmd_wedit(void) {
    int rc, del = false;
    char b[STRINGSIZE];
    char fname[STRINGSIZE];
    char *p;
    FILE *f;

    if (CurrentLinePtr) error("Invalid in a program");
    if (*CurrentFile > 1) {
        strcpy(fname, CurrentFile);
    } else {
        strcpy(fname, getenv("TEMP"));
        strcat(fname, "\\MMBasic.tmp");
        f = fopen(fname, "wb");
        if (errno) error("Cannot write to $", fname);

        p = ProgMemory;
        while (!(*p == 0 || *p == 0xff)) {  // this is a safety precaution
            if (*p == T_LINENBR || *p == T_NEWLINE) {
                p = llist(b, p);  // otherwise expand the line
                if (!(p[0] == 0 && p[1] == 0)) strcat(b, "\r\n");
                fwrite(b, strlen(b), 1, f);
                if (errno) error("Cannot write to $", fname);
                if (p[0] == 0 && p[1] == 0) break;  // end of the program ?
            }
        }
        fclose(f);
        del = true;
    }

    // Launch an external editor.
    char *mmeditor = getenv("MMEDITOR");
    mmeditor = mmeditor == NULL ? "code -w" : mmeditor;
    snprintf(b, STRINGSIZE, "%s \"%s\"", mmeditor, fname);
    rc = system(b);
    if (rc != 0) {
        error("Editor could not be run");
    }

    // Reload the file.
    void *quoted_fname = GetTempStrMemory();
    snprintf(quoted_fname, STRINGSIZE, "\"%s\"", fname);
    if (!FileLoadProgram(quoted_fname)) error("Could not read from $", fname);

    if (del) {
        console_set_title("MMBasic - Untitled");
        remove(fname);
    }
}
