cmake_minimum_required(VERSION 3.14)

# set the project name
project(MMBasic)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_definitions(-funsigned-char)

set(MMBASIC_CORE_SOURCE_FILES
    MMBasic/MMBasic.c
    MMBasic/Operators.c
    MMBasic/Functions.c
    MMBasic/Commands.c)

set(MMBASIC_LINUX_COMMANDS_SOURCE_FILES
    linux-src/commands/cmd_call.c
    linux-src/commands/cmd_cfunction.c
    linux-src/commands/cmd_chdir.c
    linux-src/commands/cmd_cls.c
    linux-src/commands/cmd_close.c
    linux-src/commands/cmd_console.c
    linux-src/commands/cmd_copy.c
    linux-src/commands/cmd_cursor.c
    linux-src/commands/cmd_dummy.c
    linux-src/commands/cmd_edit.c
    linux-src/commands/cmd_end.c
    linux-src/commands/cmd_error.c
    linux-src/commands/cmd_execute.c
    linux-src/commands/cmd_files.c
    linux-src/commands/cmd_inc.c
    linux-src/commands/cmd_ireturn.c
    linux-src/commands/cmd_kill.c
    linux-src/commands/cmd_list.c
    linux-src/commands/cmd_load.c
    linux-src/commands/cmd_longstring.c
    linux-src/commands/cmd_math.c
    linux-src/commands/cmd_memory.c
    linux-src/commands/cmd_mid.c
    linux-src/commands/cmd_mkdir.c
    linux-src/commands/cmd_new.c
    linux-src/commands/cmd_on.c
    linux-src/commands/cmd_open.c
    linux-src/commands/cmd_option.c
    linux-src/commands/cmd_pause.c
    linux-src/commands/cmd_poke.c
    linux-src/commands/cmd_quit.c
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
    linux-src/functions/fun_math.c
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

set(MMBASIC_LINUX_COMMON_SOURCE_FILES
    linux-src/common/cmdline.c
    linux-src/common/console.c
    linux-src/common/error.c
    linux-src/common/file.c
    linux-src/common/interrupt.c
    linux-src/common/maths.c
    linux-src/common/memory.c
    linux-src/common/option.c
    linux-src/common/parse.c
    linux-src/common/program.c
    linux-src/common/prompt.c
    linux-src/common/rx_buf.c
    linux-src/common/utility.c)

set(MMBASIC_LINUX_SOURCE_FILES
    ${MMBASIC_LINUX_COMMANDS_SOURCE_FILES}
    ${MMBASIC_LINUX_FUNCTIONS_SOURCE_FILES}
    ${MMBASIC_LINUX_OPERATORS_SOURCE_FILES}
    ${MMBASIC_LINUX_COMMON_SOURCE_FILES}
    linux-src/main.c)

# add the executable
add_executable(mmbasic ${MMBASIC_CORE_SOURCE_FILES} ${MMBASIC_LINUX_SOURCE_FILES})

add_executable(keyboard_test linux-src/keyboard_test.c linux-src/common/rx_buf.c linux-src/common/console.c linux-src/common/utility.c)

target_link_libraries(mmbasic m)

# GoogleTest

enable_testing()

add_executable(
  mmbasic_test
  linux-src/common/cmdline.c
  linux-src/common/file.c
  linux-src/common/rx_buf.c
  linux-src/common/utility.c
  linux-src/common/gtest/cmdline_test.cxx
  linux-src/common/gtest/file_test.cxx
  linux-src/common/gtest/rx_buf_test.cxx
)

target_link_libraries(
  mmbasic_test
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(mmbasic_test)
