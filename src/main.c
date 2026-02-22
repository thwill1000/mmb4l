/*-*****************************************************************************

MMBasic for Linux (MMB4L)

main.c

Copyright 2021-2026 Geoff Graham, Peter Mather and Thomas Hugo Williams.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

4. The name MMBasic be used when referring to the interpreter in any
   documentation and promotional material and the original copyright message
   be displayed  on the console at startup (additional copyright messages may
   be added).

5. All advertising materials mentioning features or use of this software must
   display the following acknowledgement: This product includes software
   developed by Geoff Graham, Peter Mather and Thomas Hugo Williams.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#if defined(__ANDROID__)
#include <SDL_opengles2.h>
#endif

#if defined(__ANDROID__)
#include "android.h"
#endif

#include "common/audio.h"
#include "common/cmdline.h"
#include "common/console.h"
#include "common/cstring.h"
#include "common/display.h"
#include "common/exit_codes.h"
#include "common/features.h"
#include "common/file.h"
#include "common/interrupt.h"
#include "common/keyboard.h"
#include "common/keybuf.h"
#include "common/logger.h"
#include "common/mmb4l.h"
#include "common/mmtime.h"
#include "common/parse.h"
#include "common/path.h"
#include "common/program.h"
#include "common/prompt.h"
#include "common/streamio.h"
#include "common/utility.h"
#include "core/tokentbl.h"

#define MM_VERSION_STR  xstringify(MM_MAJOR) "." xstringify(MM_MINOR) "." xstringify(MM_MICRO)
static const char version[] = "@(#) MMB4L v" MM_VERSION_STR " " __DATE__ " " __TIME__;

// global variables used in MMBasic but must be maintained outside of the
// interpreter
ErrorState mmb_normal_error_state;
ErrorState *mmb_error_state_ptr = &mmb_normal_error_state;
Options mmb_options;
Features mmb_features;
char *OnKeyGOSUB;
char *CFunctionFlash, *CFunctionLibrary;

CmdLineArgs mmb_args = { 0 };

void IntHandler(int signo);
void dump_token_table(const struct s_tokentbl* tbl);

/**
 * If 'true' then RUN program specified by 'mmb_args.run_cmd'.
 * Only used within main() but cannot be a local variable (on the stack)
 * because after a longjmp() it would be restored to its value when
 * setjmp() was called - this is undesireable.
 */
static bool run_flag;

static MmResult get_name_and_version(char *buf, size_t buf_sz) {
    (void) snprintf(
        buf,
        buf_sz,
        "MMBasic for %s v%d.%d%s%d",
        MM_ARCH,
        MM_MAJOR,
        MM_MINOR,
        MM_MICRO < 100
            ? "-alpha."
            : MM_MICRO < 200
                ? "-beta."
                : MM_MICRO < 300
                    ? "-rc."
                    : ".",
        MM_MICRO < 100
            ? MM_MICRO
            : MM_MICRO < 200
                ? MM_MICRO - 100
                : MM_MICRO < 300
                    ? MM_MICRO - 200
                    : MM_MICRO - 300);
    return kOk;
}

static MmResult get_banner(char *buf, size_t buf_sz) {
    ON_FAILURE_RETURN(get_name_and_version(buf, buf_sz));
    return SUCCEEDED(cstring_cat(buf, "\r\n" COPYRIGHT, buf_sz)) ? kOk : kStringTooLong;
}

static void init_mmbasic_config_dir() {
    LOG_FN_ENTRY();

    char config_dir[PATH_MAX] = { '\0' };
    MmResult result = file_get_config_dir(config_dir, PATH_MAX);
    ON_FAILURE_GOTO(result, error);
    result = path_mkdir(config_dir);
    ON_FAILURE_GOTO(result, error);

    RETURN_VOID();

error:
    fprintf(
        stderr,
        "\nFailed to create directory '%s': %s\n",
        config_dir,
        mmresult_to_string(result));
    exit(EX_FAIL);
}

static void init_options_cb(const char *msg) {
    static int count = 0;

    if (strcmp(msg, "END") == 0) {
        if (count > 0) display_puts("\r\n");
        return;
    }

    if (count == 0) {
        display_puts("Warnings in '");
        display_puts(options_filename);
        display_puts("':\r\n");
    }

    display_puts(msg);
    display_puts("\r\n");

    count++;
}

static void init_options() {
    LOG_FN_ENTRY();

    char filename[PATH_MAX] = { '\0' };
    MmResult result = file_get_config_dir(filename, sizeof(filename));
    ON_FAILURE_GOTO(result, error);
    result = file_append_path(filename, "mmbasic.options", sizeof(filename));
    ON_FAILURE_GOTO(result, error);
    result = path_get_canonical(filename, options_filename, sizeof(options_filename));
    ON_FAILURE_GOTO(result, error);

    options_init(&mmb_options);

    result = options_load(&mmb_options, options_filename, init_options_cb);
    switch (result) {
        case kOk:
            // Options loaded, but may still have output warnings.
            break;
        case kFileNotFound:
            // Ignore and use default options.
            break;
        default:
            goto error;
    }
    init_options_cb("END");

    RETURN_VOID();

error:
    fprintf(stderr, "\nFailed to load options: %s\n", mmresult_to_string(result));
    exit(EX_FAIL);
}

void set_start_directory() {
    // if (is_android()) {
    //     snprintf(mmb_args.directory, STRINGSIZE, "%s", android_path());
    // }

    if (mmb_args.directory[0] == '\0') {
        char *MMDIR = getenv("MMDIR");
        if (MMDIR) {
            snprintf(mmb_args.directory, STRINGSIZE, "%s", MMDIR);
            mmb_args.directory[MAXSTRLEN] = '\0';
        }
    }
    char *p = mmb_args.directory;
    cstring_unquote(p);
    if (p[0] == '\0') return;

    MmResult result = file_chdir(p);
    if (FAILED(result)) {
        display_puts("Error: could not set starting directory '");
        display_puts(p);
        display_puts("'.\r\n");
        display_puts(mmresult_to_string(result));
        display_puts(".\r\n");
        display_puts("\r\n");
    }
}

static void reset_console_title() {
    char title[STRINGSIZE + 10];
    sprintf(title, "MMBasic - %s", CurrentFile[0] == '\0' ? "Untitled" : CurrentFile);
    console_set_title(title, false);
}

/** Handle return via longjmp(). */
void longjmp_handler(int jmp_state) {

    if (mmb_args.show_prompt) {
        ON_FAILURE_EXIT(console_show_cursor(true));
        ON_FAILURE_EXIT(console_reset());
        int cursor_x = -1, cursor_y = -1;
        ON_FAILURE_EXIT(display_get_cursor_pos(false, &cursor_x, &cursor_y));
        if (cursor_x > 0) ON_FAILURE_EXIT(display_puts("\r\n"));
    }

    audio_term();

    switch (jmp_state) {
        case JMP_BREAK:
            mmb_state.exit_code = EX_BREAK;
            mmb_state.exiting = !mmb_args.show_prompt;
            break;

        case JMP_END:
            mmb_state.exiting = !mmb_args.show_prompt;
            break;

        case JMP_ERROR:
            display_puts(mmb_error_state_ptr->message);
            display_puts("\r\n");
            mmb_state.exit_code = error_to_exit_code(mmb_error_state_ptr->code);
            mmb_state.exiting = !mmb_args.show_prompt;
            break;

        case JMP_NEW:
            mmb_state.exit_code = EX_OK; // Probably not necessary.
            break;

        case JMP_QUIT:
            mmb_state.exiting = true;
            break;

        default:
            fprintf(stderr, "Unexpected return value from setjmp()");
            mmb_state.exit_code = EX_FAIL;
            mmb_state.exiting = true;
            break;
    }

    // if (mmb_state.exiting) return;

    ContinuePoint = nextstmt;  // In case the user wants to use the continue command
    *tknbuf = 0;               // we do not want to run whatever is in the token buffer
    memset(inpbuf, 0, INPBUF_SIZE);

    reset_console_title();
}

static MmResult init_prompt() {
    reset_console_title();
    console_reset();
    console_clear();
    ON_FAILURE_RETURN(console_show_cursor(true));
    char banner[1024];
    ON_FAILURE_RETURN(get_banner(banner, sizeof(banner)));
    ON_FAILURE_RETURN(display_puts(banner));
    ON_FAILURE_RETURN(display_puts("\r\n\r\n"));
    ON_FAILURE_LOG(prompt_restore_history(""));
    return kOk;
}

int android_main(int argc, char* argv[]) {
    LOG_INFO("Wooga Wooga Wooga");
#if defined(__ANDROID__)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_INFO("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL2 Android App",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (!window) {
        LOG_INFO("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        LOG_INFO("SDL_GL_CreateContext failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Clear screen with blue color
        glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
#endif
    return 0;
}

int main(int argc, char *argv[]) {
    (void) version; // To force the linker to retain it.

#if !defined(__ANDROID__) && !defined(NDEBUG)
    ON_FAILURE_EXIT(logger_init("mmb4l.log"));
#endif

    LOG_FN_ENTRY("argc=%d, argv=%p", argc, argv);
    {
        char banner[1024];
        ON_FAILURE_EXIT(get_name_and_version(banner, sizeof(banner)));
        LOG_INFO("starting %s", banner);
    }

    ON_FAILURE_EXIT(memory_init());

#if defined(__ANDROID__)
    android_init();
#endif

    MmResult result = cmdline_parse(argc, (const char **) argv, &mmb_args);
    if (FAILED(result)) {
        if (result == kStringTooLong) {
            fprintf(stderr, "Command line too long\n");
        } else {
            fprintf(stderr, "%s\n", mmresult_to_string(result));
        }
        cmdline_print_usage();
        exit(EX_FAIL);
    }

    if (mmb_args.help) {
        cmdline_print_usage();
        exit(EX_OK);
    }

    ON_FAILURE_EXIT(keybuf_init());

#if !defined(__ANDROID__)
    // Initialise the tty console.
    ON_FAILURE_EXIT(console_init(!mmb_args.show_prompt));
    console_enable_raw_mode();
    atexit(console_disable_raw_mode);
    ON_FAILURE_EXIT(console_sync());
#endif

    if (mmb_args.version) {
        char banner[1024];
        ON_FAILURE_EXIT(get_banner(banner, sizeof(banner)));
        fprintf(stdout, "%s\r\n", banner);
        exit(EX_OK);
    }

    init_mmbasic_config_dir();
    init_options();

#if defined(__ANDROID__)
    mmb_state.default_simulate = kSimulatePicocalc;
#else
    mmb_state.default_simulate =
        (mmb_args.simulate == kSimulateUnspecified) ? kSimulateMmb4l : mmb_args.simulate;
#endif

    ON_FAILURE_EXIT(InitBasic());
    ON_FAILURE_EXIT(keyboard_init());

    //printf("Commands\n--------\n");
    //dump_token_table(commandtbl);
    //printf("\n");
    //printf("Tokens\n--------\n");
    //dump_token_table(tokentbl);

#if 0
    signal(SIGBREAK, IntHandler);  // CTRL-C handler
    signal(SIGINT, IntHandler);
#endif

    run_flag = mmb_args.run_cmd[0] != '\0';

    if (mmb_args.show_prompt) {
        ON_FAILURE_EXIT(init_prompt());
    }
    set_start_directory();

    // Note that weird restrictions on what you can do with the return value
    // from setjmp() mean we cannot simply write longjmp_handler(setjmp(mark));
    switch (setjmp(mark)) {
        case 0: break;
        case JMP_BREAK: longjmp_handler(JMP_BREAK); break;
        case JMP_END:   longjmp_handler(JMP_END); break;
        case JMP_ERROR: longjmp_handler(JMP_ERROR); break;
        case JMP_NEW:   longjmp_handler(JMP_NEW); break;
        case JMP_QUIT:  longjmp_handler(JMP_QUIT); break;
        default:        longjmp_handler(JMP_UNEXPECTED); break;
    }

// #if defined(__ANDROID__)
//     return android_main(argc, argv);
// #endif

    while (!mmb_state.exiting) {
        MMAbort = false;
        LocalIndex = 0;     // this should not be needed but it ensures that all
                            // space will be cleared
        ClearTempMemory();  // clear temp string space (might have been used by
                            // the prompt)
        CurrentLinePtr = NULL;  // do not use the line number in error reporting
        int cursor_x = -1, cursor_y = -1;
        ON_FAILURE_EXIT(display_get_cursor_pos(false, &cursor_x, &cursor_y));
        if (cursor_x > 0) {
            display_puts("\r\n");  // prompt should be on a new line
        }
        //PrepareProgram(false); // This seems superflous so comment it out and see what breaks!
        // if (!ErrorInPrompt && FindSubFun("MM.PROMPT", kSub) >= 0) {
        //     ErrorInPrompt = true;
        //     ExecuteProgram("MM.PROMPT\0");
        // } else {
        if (mmb_args.show_prompt) {
            display_puts("> ");  // print the prompt
        }
        // }
        // ErrorInPrompt = false;

        // This will clear all the interrupts including ON KEY
        // TODO: is this too drastic ? it means all interrupts will have been
        //       lost if the user tries to CONTINUE a program that has halted
        //       from CTRL-C or ERROR.
        interrupt_clear();

        memset(inpbuf, 0, INPBUF_SIZE);
        if (run_flag) {
            if (mmb_args.show_prompt) {
                display_puts(mmb_args.run_cmd);
                display_puts("\r\n");
            }
            strcpy(inpbuf, mmb_args.run_cmd);
            run_flag = false;
        } else {
            ON_FAILURE_ERROR_EX(prompt_get_input(), EXIT_FAILURE);
            ON_FAILURE_ERROR_EX(parse_transform_input_buffer(inpbuf), EXIT_FAILURE);
        }

        if (!*inpbuf) continue;  // ignore an empty line
        tokenise(true);          // turn into executable code
        if (*tknbuf == T_LINENBR)  // don't let someone use line numbers at the prompt
            tknbuf[0] = tknbuf[1] = tknbuf[2] = ' '; // convert the line number into spaces
        CurrentLinePtr = NULL;  // do not use the line number in error reporting

        ExecuteProgram(tknbuf);  // execute the line straight away
    }

    ON_FAILURE_LOG(prompt_save_history(""));

#if defined(__ANDROID__)
    android_term();
    // 24-Jan-2026: The call to SDL_Quit() was segfaulting when built and run
    //              with Userland/Ubuntu but I have found previously it was
    //              necessary for the Android NDK build.
    LOG_INFO("SDL quit ...");
    SDL_Quit();  // Properly cleanup SDL
#endif

    LOG_INFO("exiting with code: %d", mmb_state.exit_code);
    return mmb_state.exit_code;
}

void IntHandler(int signo) {
#if 0
    signal(SIGBREAK, IntHandler);
    signal(SIGINT, IntHandler);
#endif
    MMAbort = true;
}

/**
 * Dump a memory area to the console in hexadecimal and ASCII format.
 * Displays 16 bytes per line with address, hex values, and printable characters.
 * 
 * @param p Pointer to the start of the memory area to dump
 * @param nbr Number of bytes to dump
 */
void dump(char *p, int nbr) {
    char buf1[80], buf2[80], *b1, *b2, *pt;
    b1 = buf1;
    b2 = buf2;
    display_puts(
        "   addr    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F    "
        "0123456789ABCDEF\r\n");
    b1 += sprintf(b1, "%8" PRIxPTR ": ", (uintptr_t) p);
    for (pt = p; (uintptr_t)pt % 16 != 0; pt--) {
        b1 += sprintf(b1, "   ");
        b2 += sprintf(b2, " ");
    }
    while (nbr > 0) {
        b1 += sprintf(b1, "%02x ", *p);
        b2 += sprintf(b2, "%c", (*p >= ' ' && *p < 0x7f) ? *p : '.');
        p++;
        nbr--;
        if ((uintptr_t)p % 16 == 0) {
            display_puts(buf1);
            display_puts("   ");
            display_puts(buf2);
            b1 = buf1;
            b2 = buf2;
            b1 += sprintf(b1, "\r\n%8" PRIxPTR ": ", (uintptr_t) p);
        }
    }
    if (b2 != buf2) {
        display_puts(buf1);
        display_puts("   ");
        for (pt = p; (uintptr_t)pt % 16 != 0; pt++) {
            display_puts("   ");
        }
        display_puts(buf2);
    }
    display_puts("\r\n");
}

void dump_token_table(const struct s_tokentbl* tbl) {
    for (int i = 0;; i++) {
        printf("%3d:  %-15s, %5d, %5d, 0x%8" PRIxPTR "\n", i, tbl[i].name, tbl[i].type, tbl[i].precedence, (uintptr_t) tbl[i].fptr);
        if (*(tbl[i].name) == 0) break;
    }
}
