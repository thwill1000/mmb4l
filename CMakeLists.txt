# Copyright (c) 2021-2022 Thomas Hugo Williams
# License MIT <https://opensource.org/licenses/MIT>

cmake_minimum_required(VERSION 3.14)

# set the project name
project(MMB4L)

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

if (MMB4L_COVERAGE)
    message("** Configuring with code-coverage")
    set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
    set(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
    set(GCOV_LINK_LIBRARY          gcov)
endif()

set(XTRA_WARNING_FLAGS "-Wwrite-strings -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-function -Wno-unused-result -Werror")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} ${XTRA_WARNING_FLAGS}")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} ${XTRA_WARNING_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

set(MMB4L_CORE_SOURCE_FILES
    src/core/Commands.c
    src/core/Functions.c
    src/core/funtbl.c
    src/core/MMBasic.c
    src/core/Operators.c
    src/core/vartbl.c
)

set(MMB4L_COMMANDS_SOURCE_FILES
    src/commands/cmd_autosave.c
    src/commands/cmd_call.c
    src/commands/cmd_cfunction.c
    src/commands/cmd_chdir.c
    src/commands/cmd_cls.c
    src/commands/cmd_close.c
    src/commands/cmd_console.c
    src/commands/cmd_copy.c
    src/commands/cmd_cursor.c
    src/commands/cmd_dummy.c
    src/commands/cmd_edit.c
    src/commands/cmd_end.c
    src/commands/cmd_erase.c
    src/commands/cmd_error.c
    src/commands/cmd_execute.c
    src/commands/cmd_files.c
    src/commands/cmd_inc.c
    src/commands/cmd_ireturn.c
    src/commands/cmd_kill.c
    src/commands/cmd_list.c
    src/commands/cmd_load.c
    src/commands/cmd_longstring.c
    src/commands/cmd_math.c
    src/commands/cmd_memory.c
    src/commands/cmd_mid.c
    src/commands/cmd_mkdir.c
    src/commands/cmd_mode.c
    src/commands/cmd_new.c
    src/commands/cmd_on.c
    src/commands/cmd_open.c
    src/commands/cmd_option.c
    src/commands/cmd_pause.c
    src/commands/cmd_poke.c
    src/commands/cmd_print.c
    src/commands/cmd_quit.c
    src/commands/cmd_read.c
    src/commands/cmd_rename.c
    src/commands/cmd_restore.c
    src/commands/cmd_rmdir.c
    src/commands/cmd_run.c
    src/commands/cmd_seek.c
    src/commands/cmd_settick.c
    src/commands/cmd_settitle.c
    src/commands/cmd_sort.c
    src/commands/cmd_system.c
    src/commands/cmd_timer.c
    src/commands/cmd_trace.c
    src/commands/cmd_xmodem.c
)

set(MMB4L_FUNCTIONS_SOURCE_FILES
    src/functions/fun_at.c
    src/functions/fun_bin2str.c
    src/functions/fun_bound.c
    src/functions/fun_call.c
    src/functions/fun_choice.c
    src/functions/fun_chr.c
    src/functions/fun_cwd.c
    src/functions/fun_date.c
    src/functions/fun_datetime.c
    src/functions/fun_day.c
    src/functions/fun_dir.c
    src/functions/fun_eof.c
    src/functions/fun_epoch.c
    src/functions/fun_format.c
    src/functions/fun_field.c
    src/functions/fun_hres.c
    src/functions/fun_inputstr.c
    src/functions/fun_json.c
    src/functions/fun_lcompare.c
    src/functions/fun_lgetbyte.c
    src/functions/fun_lgetstr.c
    src/functions/fun_linstr.c
    src/functions/fun_llen.c
    src/functions/fun_loc.c
    src/functions/fun_lof.c
    src/functions/fun_math.c
    src/functions/fun_mmcmdline.c
    src/functions/fun_mmdevice.c
    src/functions/fun_mminfo.c
    src/functions/fun_peek.c
    src/functions/fun_rgb.c
    src/functions/fun_str2bin.c
    src/functions/fun_time.c
    src/functions/fun_timer.c
    src/functions/fun_vres.c
)

set(MMB4L_OPERATORS_SOURCE_FILES
    src/operators/op_inv.c
)

set(MMB4L_COMMON_SOURCE_FILES
    src/common/cmdline.c
    src/common/codepage.c
    src/common/console.c
    src/common/cJSON.c
    src/common/cstring.c
    src/common/error.c
    src/common/file.c
    src/common/interrupt.c
    src/common/maths.c
    src/common/memory.c
    src/common/mmresult.c
    src/common/mmtime.c
    src/common/options.c
    src/common/parse.c
    src/common/path.c
    src/common/program.c
    src/common/prompt.c
    src/common/rx_buf.c
    src/common/serial.c
    src/common/xmodem.c
)

set(MMB4L_SOURCE_FILES
    ${MMB4L_COMMANDS_SOURCE_FILES}
    ${MMB4L_FUNCTIONS_SOURCE_FILES}
    ${MMB4L_OPERATORS_SOURCE_FILES}
    ${MMB4L_COMMON_SOURCE_FILES}
    src/main.c)

# add the executable
add_executable(mmbasic ${MMB4L_CORE_SOURCE_FILES} ${MMB4L_SOURCE_FILES})

add_executable(test_keyboard
    src/keyboard_test.c
    src/common/rx_buf.c
    src/common/codepage.c
    src/common/console.c
    src/common/cstring.c
    src/common/mmresult.c
    src/common/mmtime.c
    src/common/options.c
    src/common/path.c)

target_link_libraries(
    mmbasic
    m
    ${GCOV_LINK_LIBRARY}
)

# GoogleTest

enable_testing()

include(GoogleTest)

################################################################################
# test_cmdline
################################################################################

add_executable(
  test_cmdline
  src/common/cmdline.c
  src/common/cstring.c
  src/common/memory.c
  src/common/parse.c
  src/common/gtest/cmdline_test.cxx
)

target_link_libraries(
  test_cmdline
  gtest_main
  gmock
  gmock_main
  ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_cmdline)

################################################################################
# test_cmd_run
################################################################################

add_executable(
  test_cmd_run
  src/commands/cmd_run.c
  src/commands/gtest/cmd_run_test.cxx
  src/common/cstring.c
  src/common/memory.c
  src/common/mmresult.c
  src/common/parse.c
  src/core/funtbl.c
  src/core/MMBasic.c
  src/core/vartbl.c
  src/functions/fun_mmcmdline.c
)

target_link_libraries(
  test_cmd_run
  gtest_main
  gmock
  gmock_main
  ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_cmd_run)

################################################################################
# test_cstring
################################################################################

add_executable(
  test_cstring
  src/common/cstring.c
  src/common/gtest/cstring_test.cxx
)

target_link_libraries(
  test_cstring
  gtest_main
  gmock
  gmock_main
)

gtest_discover_tests(test_cstring)

################################################################################
# test_funtbl
################################################################################

add_executable(
  test_funtbl
  src/core/funtbl.c
  src/core/gtest/funtbl_test.cxx
)

target_link_libraries(
  test_funtbl
  gtest_main
  gmock
  gmock_main
)

gtest_discover_tests(test_funtbl)

################################################################################
# test_hash
################################################################################

add_executable(
  test_hash
  src/common/gtest/hash_test.cxx
)

target_link_libraries(
  test_hash
  gtest_main
  gmock
  gmock_main
)

gtest_discover_tests(test_hash)

################################################################################
# test_options
################################################################################

add_executable(
  test_options
  src/common/codepage.c
  src/common/cstring.c
  src/common/options.c
  src/common/mmresult.c
  src/common/path.c
  src/common/gtest/options_test.cxx
)

target_compile_definitions(test_options PUBLIC OPTION_TESTS)

target_link_libraries(
  test_options
  gtest_main
  gmock
  gmock_main
  ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_options)

################################################################################
# test_parse
################################################################################

add_executable(
  test_parse
  src/common/cstring.c
  src/common/memory.c
  src/common/parse.c
  src/common/gtest/parse_test.cxx
)

target_link_libraries(
  test_parse
  gtest_main
  gmock
  gmock_main
  ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_parse)

################################################################################
# test_path
################################################################################

add_executable(
  test_path
  src/common/cstring.c
  src/common/path.c
  src/common/gtest/path_test.cxx
)

target_link_libraries(
  test_path
  gtest_main
  gmock
  gmock_main
  ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_path)

################################################################################
# test_program
################################################################################

add_executable(
  test_program
  src/common/cstring.c
  src/common/path.c
  src/common/program.c
  src/common/gtest/program_test.cxx
)

target_link_libraries(
  test_program
  gtest_main
  gmock
  gmock_main
  ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_program)

################################################################################
# test_prompt
################################################################################

add_executable(
  test_prompt
  src/common/cstring.c
  src/common/prompt.c
  src/common/gtest/prompt_test.cxx
)

target_link_libraries(
  test_prompt
  gtest_main
  gmock
  gmock_main
  ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_prompt)

################################################################################
# test_rx_buf
################################################################################

add_executable(
  test_rx_buf
  src/common/rx_buf.c
  src/common/gtest/rx_buf_test.cxx
)

target_link_libraries(
  test_rx_buf
  gtest_main
  gmock
  gmock_main
  ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_rx_buf)

################################################################################
# test_mmbasic_core
################################################################################

add_executable(
  test_mmbasic_core
  src/common/cstring.c
  src/common/file.c
  src/common/interrupt.c
  src/common/memory.c
  src/common/mmresult.c
  src/common/mmtime.c
  src/common/parse.c
  src/common/path.c
  src/common/program.c
  src/common/rx_buf.c
  src/common/serial.c
  src/core/funtbl.c
  src/core/MMBasic.c
  src/core/vartbl.c
  src/core/gtest/mmbasic_core_test.cxx
)

target_link_libraries(
  test_mmbasic_core
  gtest_main
  gmock
  gmock_main
  ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_mmbasic_core)

################################################################################
# test_vartbl
################################################################################

add_executable(
  test_vartbl
  src/core/vartbl.c
  src/core/gtest/vartbl_test.cxx
)

target_link_libraries(
  test_vartbl
  gtest_main
  gmock
  gmock_main
  ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_vartbl)
