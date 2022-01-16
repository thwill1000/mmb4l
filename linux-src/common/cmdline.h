#if !defined(MMB4L_CMDLINE)
#define MMB4L_CMDLINE

typedef struct {
    char help;
    char interactive;
    char version;
    char run_cmd[256];
    char directory[256];
} CmdLineArgs;

/** @return  0 on success, -1 on error. */
int cmdline_parse(int argc, const char *argv[], CmdLineArgs *result);

void cmdline_print_usage();

#endif
