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
    linux-src/commands/call.c
    linux-src/commands/cls.c
    linux-src/commands/cmd_math.c
    linux-src/commands/colour.c
    linux-src/commands/cursor.c
    linux-src/commands/dummy.c
    linux-src/commands/execute.c
    linux-src/commands/exitmmb.c
    linux-src/commands/inc.c
    linux-src/commands/ireturn.c
    linux-src/commands/list.c
    linux-src/commands/load.c
    linux-src/commands/longstring.c
    linux-src/commands/memory_cmd.c
    linux-src/commands/on_cmd.c
    linux-src/commands/option.c
    linux-src/commands/pause.c
    linux-src/commands/poke.c
    linux-src/commands/program.c
    linux-src/commands/reload.c
    linux-src/commands/run.c
    linux-src/commands/settitle.c
    linux-src/commands/sort.c
    linux-src/commands/system.c
    linux-src/commands/timer_cmd.c
    linux-src/commands/trace.c
    linux-src/commands/wedit.c)

set(MMBASIC_LINUX_FUNCTIONS_SOURCE_FILES
    linux-src/functions/bound.c
    linux-src/functions/choice.c
    linux-src/functions/cwd.c
    linux-src/functions/date.c
    linux-src/functions/device.c
    linux-src/functions/dir.c
    linux-src/functions/format.c
    linux-src/functions/fun_call.c
    linux-src/functions/fun_field.c
    linux-src/functions/fun_lcompare.c
    linux-src/functions/fun_linstr.c
    linux-src/functions/hres.c
    linux-src/functions/info.c
    linux-src/functions/lgetbyte.c
    linux-src/functions/lgetstr.c
    linux-src/functions/llen.c
    linux-src/functions/mmcmdline.c
    linux-src/functions/peek.c
    linux-src/functions/rgb.c
    linux-src/functions/time_fun.c
    linux-src/functions/timer_fun.c
    linux-src/functions/vres.c
    linux-src/operators/op_inv.c)

set(MMBASIC_LINUX_SOURCE_FILES
    ${MMBASIC_LINUX_COMMANDS_SOURCE_FILES}
    ${MMBASIC_LINUX_FUNCTIONS_SOURCE_FILES}
    linux-src/common/error.c
    linux-src/common/console.c
    linux-src/common/interrupt.c
    linux-src/common/utility.c
    linux-src/editor.c
    linux-src/file_io.c
    linux-src/main.c
    linux-src/memory.c)

# add the executable
add_executable(MMBasic ${MMBASIC_CORE_SOURCE_FILES} ${MMBASIC_LINUX_SOURCE_FILES})

add_executable(keyboard_test linux-src/keyboard_test.c linux-src/common/console.c)

target_link_libraries(MMBasic m)
