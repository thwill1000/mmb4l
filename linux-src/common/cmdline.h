#if !defined(MMB4L_CMDLINE)
#define MMB4L_CMDLINE

typedef struct {
    char help;
    char interactive;
    char version;
    char run_cmd[256];
    char directory[256];
} CmdLineArgs;

int cmdline_parse(int argc, const char *argv[], CmdLineArgs *result);
void cmdline_print_usage(void (*println)(const char *));

#endif
