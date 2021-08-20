#include <stddef.h>

char *StartEditPoint = NULL;
int StartEditChar = 0;

void cmd_wedit();

void cmd_edit(void) {
    cmd_wedit();
}
