cmake_minimum_required(VERSION 3.10)

# set the project name
project(MMBasic)

add_definitions(-funsigned-char)

set(MMBASIC_CORE_SOURCE_FILES
    MMBasic/MMBasic.c
    MMBasic/Operators.c
    MMBasic/Functions.c
    MMBasic/Commands.c)

set(MMBASIC_LINUX_COMMANDS_SOURCE_FILES
    linux-src/commands/cmd_call.c
    linux-src/commands/cmd_chdir.c
    linux-src/commands/cmd_cls.c
    linux-src/commands/cmd_close.c
    linux-src/commands/cmd_console.c
    linux-src/commands/cmd_copy.c
    linux-src/commands/cmd_cursor.c
    linux-src/commands/cmd_dummy.c
    linux-src/commands/cmd_edit.c
    linux-src/commands/cmd_execute.c
    linux-src/commands/cmd_exitmmb.c
    linux-src/commands/cmd_files.c
    linux-src/commands/cmd_inc.c
    linux-src/commands/cmd_ireturn.c
    linux-src/commands/cmd_kill.c
    linux-src/commands/cmd_list.c
    linux-src/commands/cmd_load.c
    linux-src/commands/cmd_longstring.c
    linux-src/commands/cmd_math.c
    linux-src/commands/cmd_memory.c
    linux-src/commands/cmd_mkdir.c
    linux-src/commands/cmd_on.c
    linux-src/commands/cmd_open.c
    linux-src/commands/cmd_option.c
    linux-src/commands/cmd_pause.c
    linux-src/commands/cmd_poke.c
    linux-src/commands/cmd_program.c
    linux-src/commands/cmd_rename.c
    linux-src/commands/cmd_rmdir.c
    linux-src/commands/cmd_run.c
    linux-src/commands/cmd_seek.c
    linux-src/commands/cmd_settitle.c
    linux-src/commands/cmd_sort.c
    linux-src/commands/cmd_system.c
    linux-src/commands/cmd_timer.c
    linux-src/commands/cmd_trace.c)

set(MMBASIC_LINUX_FUNCTIONS_SOURCE_FILES
    linux-src/functions/fun_bound.c
    linux-src/functions/fun_call.c
    linux-src/functions/fun_choice.c
    linux-src/functions/fun_cwd.c
    linux-src/functions/fun_date.c
    linux-src/functions/fun_dir.c
    linux-src/functions/fun_eof.c
    linux-src/functions/fun_format.c
    linux-src/functions/fun_field.c
    linux-src/functions/fun_hres.c
    linux-src/functions/fun_inputstr.c
    linux-src/functions/fun_lcompare.c
    linux-src/functions/fun_lgetbyte.c
    linux-src/functions/fun_lgetstr.c
    linux-src/functions/fun_linstr.c
    linux-src/functions/fun_llen.c
    linux-src/functions/fun_loc.c
    linux-src/functions/fun_lof.c
    linux-src/functions/fun_mmcmdline.c
    linux-src/functions/fun_mmdevice.c
    linux-src/functions/fun_mminfo.c
    linux-src/functions/fun_peek.c
    linux-src/functions/fun_rgb.c
    linux-src/functions/fun_time.c
    linux-src/functions/fun_timer.c
    linux-src/functions/fun_vres.c)

set(MMBASIC_LINUX_OPERATORS_SOURCE_FILES
    linux-src/operators/op_inv.c)

set(MMBASIC_LINUX_SOURCE_FILES
    ${MMBASIC_LINUX_COMMANDS_SOURCE_FILES}
    ${MMBASIC_LINUX_FUNCTIONS_SOURCE_FILES}
    ${MMBASIC_LINUX_OPERATORS_SOURCE_FILES}
    linux-src/common/console.c
    linux-src/common/error.c
    linux-src/common/file.c
    linux-src/common/interrupt.c
    linux-src/common/option.c
    linux-src/common/parse.c
    linux-src/common/prompt.c
    linux-src/common/utility.c
    linux-src/main.c
    linux-src/memory.c)

# add the executable
add_executable(MMBasic ${MMBASIC_CORE_SOURCE_FILES} ${MMBASIC_LINUX_SOURCE_FILES})

add_executable(keyboard_test linux-src/keyboard_test.c linux-src/common/console.c)

target_link_libraries(MMBasic m)
