# Copyright (c) 2021-2026 Thomas Hugo Williams
# License MIT <https://opensource.org/licenses/MIT>

cmake_minimum_required(VERSION 3.14)

# set the project name
project(MMB4L)

# =============================================================================
# SDL2 Configuration Function
# =============================================================================

function(configure_sdl2 target_name link_sdl2main)
    if(WIN32)
        set(SDL2_ROOT "D:/github/thwill1000/SDL2-2.32.10")

        if(NOT EXISTS "${SDL2_ROOT}/include/SDL.h")
            message(FATAL_ERROR "SDL2 not found at ${SDL2_ROOT}")
        endif()

        target_include_directories(${target_name} PUBLIC "${SDL2_ROOT}/include")
        target_link_directories(${target_name} PUBLIC "${SDL2_ROOT}/lib/x64")

        if(link_sdl2main)
            target_link_libraries(${target_name} SDL2main SDL2)
        else()
            target_link_libraries(${target_name} SDL2)
        endif()

        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SDL2_ROOT}/lib/x64/SDL2.dll"
                $<TARGET_FILE_DIR:${target_name}>
        )
    else()
        target_include_directories(${target_name} PUBLIC /usr/include/SDL2)
        target_link_libraries(${target_name} SDL2)
    endif()
endfunction()

# Unit tests require at least C++20
#  - GoogleTest framework requires at least C++17
#  - Some C++ tests use C style designated initializers (C++20)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
#FetchContent_Declare(
#  googletest
#  URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
#)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(WIN32)
    set(PLATFORM "windows")
else()
    set(PLATFORM "linux")
endif()

if (MMB4L_COVERAGE)
    message("** Configuring with code-coverage")
    set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
    set(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
    set(GCOV_LINK_LIBRARY          gcov)
else()
    set(GCOV_LINK_LIBRARY          "")  # Empty on Windows/when coverage disabled
endif()

# Platform-specific compiler flags
if (MSVC)
    # Visual Studio compiler flags
    add_compile_options(
        /W4          # Warning level 4
        /WX          # Treat warnings as errors
        /wd4100      # Unreferenced formal parameter
        /wd4127      # Conditional expression is constant
        /wd4201      # Nonstandard extension used
        /wd4244      # Conversion warnings
        /wd4267      # Size_t conversion warnings
        /wd4456      # Declaration of '...' hides previous local declaration
        /wd4457      # Declaration of '...' hides function parameter
        /wd4459      # Declaration of '...' hides global declaration
        /wd4505      # Unreferenced local function has been removed
        /utf-8       # Set source and execution character sets to UTF-8
        /J           # Make char type unsigned (equivalent to -funsigned-char)
    )

    # Add Windows-specific definitions
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS  # Disable warnings about "unsafe" C functions
        NOMINMAX                 # Prevent Windows.h from defining min/max macros
        WIN32_LEAN_AND_MEAN      # Exclude rarely-used stuff from Windows headers
    )

    # Disable Identical COMDAT Folding: the linker would otherwise merge all
    # identical empty stub functions used in unit-tests into a single address,
    # breaking function pointer comparisons. The main binary is unaffected in
    # practice since its real function bodies are all distinct.
    add_link_options(/OPT:NOICF)
else()
    # GCC/Clang compiler flags (Linux/macOS)
    add_compile_options(
        "-Wwrite-strings;-Wall;-Wextra;-Wno-unused-parameter;-Wno-missing-field-initializers;-Wno-unused-function;-Wno-unused-result;-Werror"
        ${GCC_COVERAGE_COMPILE_FLAGS}
        -funsigned-char
    )
endif()

add_compile_definitions(
    _FILE_OFFSET_BITS=64
    _REENTRANT
)

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

set(MMB4L_CORE_SOURCE_FILES
    src/core/commandtbl.c
    src/core/Commands.c
    src/core/Functions.c
    src/core/funtbl.c
    src/core/maths.c
    src/core/MMBasic.c
    src/core/mmb_globals.c
    src/core/Operators.c
    src/core/tokentbl.c
    src/core/vartbl.c
)

set(MMB4L_COMMANDS_SOURCE_FILES
    src/commands/cmd_autosave.c
    src/commands/cmd_arc.c
    src/commands/cmd_blit.c
    src/commands/cmd_box.c
    src/commands/cmd_breakpoint.c
    src/commands/cmd_call.c
    src/commands/cmd_case.c
    src/commands/cmd_cfunction.c
    src/commands/cmd_chdir.c
    src/commands/cmd_circle.c
    src/commands/cmd_clear.c
    src/commands/cmd_cls.c
    src/commands/cmd_close.c
    src/commands/cmd_colour.c
    src/commands/cmd_console.c
    src/commands/cmd_const.c
    src/commands/cmd_continue.c
    src/commands/cmd_copy.c
    src/commands/cmd_cursor.c
    src/commands/cmd_device.c
    src/commands/cmd_dim.c
    src/commands/cmd_do.c
    src/commands/cmd_dummy.c
    src/commands/cmd_drive.c
    src/commands/cmd_edit.c
    src/commands/cmd_else.c
    src/commands/cmd_end.c
    src/commands/cmd_endfun.c
    src/commands/cmd_erase.c
    src/commands/cmd_error.c
    src/commands/cmd_execute.c
    src/commands/cmd_exit.c
    src/commands/cmd_exitfor.c
    src/commands/cmd_files.c
    src/commands/cmd_flash.c
    src/commands/cmd_font.c
    src/commands/cmd_for.c
    src/commands/cmd_framebuffer.c
    src/commands/cmd_gamepad.c
    src/commands/cmd_gosub.c
    src/commands/cmd_goto.c
    src/commands/cmd_graphics.c
    src/commands/cmd_gui.c
    src/commands/cmd_if.c
    src/commands/cmd_image.c
    src/commands/cmd_in.c
    src/commands/cmd_inc.c
    src/commands/cmd_input.c
    src/commands/cmd_ireturn.c
    src/commands/cmd_kill.c
    src/commands/cmd_let.c
    src/commands/cmd_line.c
    src/commands/cmd_lineinput.c
    src/commands/cmd_list.c
    src/commands/cmd_load.c
    src/commands/cmd_log.c
    src/commands/cmd_loop.c
    src/commands/cmd_longstring.c
    src/commands/cmd_memory.c
    src/commands/cmd_mid.c
    src/commands/cmd_mkdir.c
    src/commands/cmd_mkfile.c
    src/commands/cmd_mmdebug.c
    src/commands/cmd_mode.c
    src/commands/cmd_new.c
    src/commands/cmd_next.c
    src/commands/cmd_null.c
    src/commands/cmd_on.c
    src/commands/cmd_open.c
    src/commands/cmd_option.c
    src/commands/cmd_out.c
    src/commands/cmd_page.c
    src/commands/cmd_pause.c
    src/commands/cmd_pin.c
    src/commands/cmd_pixel.c
    src/commands/cmd_play.c
    src/commands/cmd_poke.c
    src/commands/cmd_print.c
    src/commands/cmd_pulse.c
    src/commands/cmd_pin.c
    src/commands/cmd_polygon.c
    src/commands/cmd_quit.c
    src/commands/cmd_randomize.c
    src/commands/cmd_rbox.c
    src/commands/cmd_read.c
    src/commands/cmd_rename.c
    src/commands/cmd_restore.c
    src/commands/cmd_return.c
    src/commands/cmd_rmdir.c
    src/commands/cmd_run.c
    src/commands/cmd_save.c
    src/commands/cmd_seek.c
    src/commands/cmd_select.c
    src/commands/cmd_setenv.c
    src/commands/cmd_setpin.c
    src/commands/cmd_settick.c
    src/commands/cmd_settitle.c
    src/commands/cmd_sort.c
    src/commands/cmd_sprite.c
    src/commands/cmd_subfun.c
    src/commands/cmd_system.c
    src/commands/cmd_text.c
    src/commands/cmd_timer.c
    src/commands/cmd_trace.c
    src/commands/cmd_triangle.c
    src/commands/cmd_troff.c
    src/commands/cmd_tron.c
    src/commands/cmd_xmodem.c
)

set(MMB4L_FUNCTIONS_SOURCE_FILES
    src/functions/fun_at.c
    src/functions/fun_atchar.c
    src/functions/fun_atan2.c
    src/functions/fun_bin.c
    src/functions/fun_bin2str.c
    src/functions/fun_bound.c
    src/functions/fun_call.c
    src/functions/fun_choice.c
    src/functions/fun_chr.c
    src/functions/fun_classic.c
    src/functions/fun_cwd.c
    src/functions/fun_date.c
    src/functions/fun_datetime.c
    src/functions/fun_day.c
    src/functions/fun_device.c
    src/functions/fun_dir.c
    src/functions/fun_eof.c
    src/functions/fun_epoch.c
    src/functions/fun_eval.c
    src/functions/fun_format.c
    src/functions/fun_field.c
    src/functions/fun_gamepad.c
    src/functions/fun_hex.c
    src/functions/fun_hres.c
    src/functions/fun_inputstr.c
    src/functions/fun_instr.c
    src/functions/fun_json.c
    src/functions/fun_keydown.c
    src/functions/fun_lcompare.c
    src/functions/fun_left.c
    src/functions/fun_lgetbyte.c
    src/functions/fun_lgetstr.c
    src/functions/fun_linstr.c
    src/functions/fun_llen.c
    src/functions/fun_loc.c
    src/functions/fun_lof.c
    src/functions/fun_max.c
    src/functions/fun_mid.c
    src/functions/fun_min.c
    src/functions/fun_mmcmdline.c
    src/functions/fun_mmdevice.c
    src/functions/fun_mminfo.c
    src/functions/fun_oct.c
    src/functions/fun_peek.c
    src/functions/fun_pin.c
    src/functions/fun_pixel.c
    src/functions/fun_port.c
    src/functions/fun_right.c
    src/functions/fun_rgb.c
    src/functions/fun_sprite.c
    src/functions/fun_str.c
    src/functions/fun_string.c
    src/functions/fun_str2bin.c
    src/functions/fun_time.c
    src/functions/fun_timer.c
    src/functions/fun_vres.c
)

set(MMB4L_OPERATORS_SOURCE_FILES
    src/operators/op_inv.c
)

set(MMB4L_COMMON_SOURCE_FILES
    src/common/audio.c
    src/common/audio_tables.c
    src/common/cmdline.c
    src/common/codepage.c
    src/common/console.c
    src/common/console_${PLATFORM}.c
    src/common/cstring.c
    src/common/display.c
    src/common/error.c
    src/common/events.c
    src/common/features.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/flash.c
    src/common/fonttbl.c
    src/common/gamepad.c
    src/common/gpio.c
    src/common/graphics.c
    src/common/image.c
    src/common/interrupt.c
    src/common/keyboard.c
    src/common/keybuf.c
    src/common/keybuf_${PLATFORM}.c
    src/common/logger.c
    src/common/memory.c
    src/common/mmgetline.c
    src/common/mmresult.c
    src/common/mmtime.c
    src/common/mmtime_${PLATFORM}.c
    src/common/options.c
    src/common/parse.c
    src/common/path.c
    src/common/editor.c
    src/common/program.c
    src/common/prompt.c
    src/common/rx_buf.c
    src/common/serial_${PLATFORM}.c
    src/common/sprite.c
    src/common/stack.c
    src/common/streamio.c
    src/common/system_${PLATFORM}.c
    src/common/termgfx.c
    src/common/utility.c
    src/common/xmodem.c
)

set(MMB4L_THIRD_PARTY_SOURCE_FILES
    src/third_party/cJSON.c
    src/third_party/dr_audio.c
    src/third_party/hxcmod.c
    src/third_party/picojpeg.c
    src/third_party/spbmp.c
    src/third_party/toojpeg_streaming.cpp
    src/third_party/upng.c
)

set(MMB4L_COMMON_TEST_FILES
    src/common/codepage.c
    src/common/cstring.c
    src/common/logger.c
    src/common/mmresult.c
    src/common/mmtime.c
    src/common/mmtime_${PLATFORM}.c
    src/common/utility.c
    src/common/gtest/stubs/error_stubs.c
)

set(MMB4L_SOURCE_FILES
    ${MMB4L_COMMANDS_SOURCE_FILES}
    ${MMB4L_FUNCTIONS_SOURCE_FILES}
    ${MMB4L_OPERATORS_SOURCE_FILES}
    ${MMB4L_COMMON_SOURCE_FILES}
    ${MMB4L_THIRD_PARTY_SOURCE_FILES}
    src/main.c)

# add the executable
add_executable(mmbasic ${MMB4L_CORE_SOURCE_FILES} ${MMB4L_SOURCE_FILES})

configure_sdl2(mmbasic TRUE)

if(NOT WIN32)
    target_link_libraries(
        mmbasic
        m
        ${GCOV_LINK_LIBRARY}
        SDL2
    )
endif()

################################################################################
# test_keybuf_get
################################################################################

add_executable(test_console_getc
    ${MMB4L_COMMON_TEST_FILES}
    src/keybuf_get_test.c
    src/common/rx_buf.c
    src/common/console.c
    src/common/console_${PLATFORM}.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/keybuf.c
    src/common/keybuf_${PLATFORM}.c
    src/common/mmtime.c
    src/common/mmtime_${PLATFORM}.c
    src/common/options.c
    src/common/path.c
    src/common/gtest/stubs/display_stubs.c
    src/common/gtest/stubs/serial_stubs.c
)

configure_sdl2(test_console_getc FALSE)

# GoogleTest

enable_testing()

include(GoogleTest)

################################################################################
# test_cmdline
################################################################################

add_executable(test_cmdline
    ${MMB4L_COMMON_TEST_FILES}
    src/common/cmdline.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/features.c
    src/common/memory.c
    src/common/mmresult.c
    src/common/options.c
    src/common/parse.c
    src/common/path.c
    src/common/gtest/cmdline_test.cxx
    src/common/gtest/stubs/serial_stubs.c
    src/core/vartbl.c
)

configure_sdl2(test_cmdline FALSE)

target_link_libraries(test_cmdline
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_cmdline)

################################################################################
# test_cmd_do
################################################################################

add_executable(test_cmd_do
    ${MMB4L_COMMON_TEST_FILES}
    src/commands/cmd_do.c
    src/commands/gtest/cmd_do_test.cxx
    src/common/features.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/fonttbl.c
    src/common/options.c
    src/common/memory.c
    src/common/parse.c
    src/common/path.c
    src/common/gtest/test_helper.c
    src/common/gtest/stubs/audio_stubs.c
    src/common/gtest/stubs/display_stubs.c
    src/common/gtest/stubs/flash_stubs.c
    src/common/gtest/stubs/gamepad_stubs.c
    src/common/gtest/stubs/graphics_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/serial_stubs.c
    src/core/commandtbl.c
    src/core/funtbl.c
    src/core/MMBasic.c
    src/core/mmb_globals.c
    src/core/tokentbl.c
    src/core/vartbl.c
)

configure_sdl2(test_cmd_do FALSE)

target_link_libraries(test_cmd_do
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_cmd_do)

################################################################################
# test_cmd_run
################################################################################

add_executable(test_cmd_run
    ${MMB4L_COMMON_TEST_FILES}
    src/commands/cmd_run.c
    src/commands/gtest/cmd_run_test.cxx
    src/common/features.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/fonttbl.c
    src/common/memory.c
    src/common/mmresult.c
    src/common/options.c
    src/common/parse.c
    src/common/path.c
    src/common/gtest/stubs/audio_stubs.c
    src/common/gtest/stubs/display_stubs.c
    src/common/gtest/stubs/flash_stubs.c
    src/common/gtest/stubs/gamepad_stubs.c
    src/common/gtest/stubs/graphics_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/serial_stubs.c
    src/core/commandtbl.c
    src/core/funtbl.c
    src/core/MMBasic.c
    src/core/mmb_globals.c
    src/core/tokentbl.c
    src/core/vartbl.c
    src/functions/fun_mmcmdline.c
)

configure_sdl2(test_cmd_run FALSE)

target_compile_definitions(test_cmd_run PUBLIC ENABLE_GTEST_EXTRAS)

target_link_libraries(test_cmd_run
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_cmd_run)

################################################################################
# test_bitset
################################################################################

add_executable(test_bitset
    ${MMB4L_COMMON_TEST_FILES}
    src/common/gtest/bitset_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_bitset FALSE)

target_link_libraries(
    test_bitset
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_bitset)

################################################################################
# test_cstring
################################################################################

add_executable(test_cstring
    ${MMB4L_COMMON_TEST_FILES}
    src/common/gtest/cstring_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_cstring FALSE)

target_link_libraries(test_cstring
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_cstring)

################################################################################
# test_file
################################################################################

add_executable(test_file
    ${MMB4L_COMMON_TEST_FILES}
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/path.c
    src/common/streamio.c
    src/common/gtest/file_test.cxx
    src/common/gtest/stubs/serial_stubs.c
)

configure_sdl2(test_file FALSE)

target_link_libraries(test_file
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_file)

################################################################################
# test_display
################################################################################

add_executable(test_display
    ${MMB4L_COMMON_TEST_FILES}
    src/common/display.c
    src/common/fonttbl.c
    src/common/termgfx.c
    src/common/gtest/display_test.cxx
    src/common/gtest/stubs/console_stubs.c
    src/common/gtest/stubs/file_stubs.c
    src/common/gtest/stubs/graphics_stubs.c
    src/common/gtest/stubs/sdl2_stubs.c
)

configure_sdl2(test_display FALSE)

target_link_libraries(
    test_display
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_display)

################################################################################
# test_editor
################################################################################

add_executable(test_editor
    ${MMB4L_COMMON_TEST_FILES}
    src/common/gtest/editor_test.cxx
    src/common/editor.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/memory.c
    src/common/path.c
    src/common/gtest/stubs/console_stubs.c
    src/common/gtest/stubs/display_stubs.c
    src/common/gtest/stubs/keybuf_stubs.c
    src/common/gtest/stubs/streamio_stubs.c
    src/core/commandtbl.c
    src/core/tokentbl.c
)

configure_sdl2(test_editor FALSE)

target_compile_definitions(test_editor PUBLIC ENABLE_GTEST_EXTRAS)

target_link_libraries(test_editor
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_editor)

################################################################################
# test_funtbl
################################################################################

add_executable(test_funtbl
    ${MMB4L_COMMON_TEST_FILES}
    src/core/funtbl.c
    src/core/gtest/funtbl_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_funtbl FALSE)

target_link_libraries(
    test_funtbl
    gtest_main
    gmock
    gmock_main
)

################################################################################
# test_fun_mminfo
################################################################################

add_executable(test_fun_mminfo
    ${MMB4L_COMMON_TEST_FILES}
    src/functions/gtest/fun_mminfo_test.cxx
    src/common/events.c
    src/common/features.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/fonttbl.c
    src/common/graphics.c
    src/common/memory.c
    src/common/options.c
    src/common/parse.c
    src/common/path.c
    src/common/sprite.c
    src/common/streamio.c
    src/common/gtest/stubs/audio_stubs.c
    src/common/gtest/stubs/display_stubs.c
    src/common/gtest/stubs/flash_stubs.c
    src/common/gtest/stubs/gamepad_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/serial_stubs.c
    src/core/commandtbl.c
    src/core/funtbl.c
    src/core/MMBasic.c
    src/core/mmb_globals.c
    src/core/tokentbl.c
    src/core/vartbl.c
    src/functions/fun_hres.c
    src/functions/fun_mminfo.c
    src/functions/fun_vres.c
    src/third_party/spbmp.c
    src/third_party/upng.c
)

target_compile_definitions(test_fun_mminfo PUBLIC ENABLE_GTEST_EXTRAS)

configure_sdl2(test_fun_mminfo FALSE)

target_link_libraries(test_fun_mminfo
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_fun_mminfo)

################################################################################
# test_fun_sprite
################################################################################

add_executable(test_fun_sprite
    ${MMB4L_COMMON_TEST_FILES}
    src/functions/gtest/fun_sprite_test.cxx
    src/common/events.c
    src/common/features.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/fonttbl.c
    src/common/graphics.c
    src/common/memory.c
    src/common/options.c
    src/common/parse.c
    src/common/path.c
    src/common/sprite.c
    src/common/streamio.c
    src/common/gtest/test_helper.c
    src/common/gtest/stubs/audio_stubs.c
    src/common/gtest/stubs/display_stubs.c
    src/common/gtest/stubs/flash_stubs.c
    src/common/gtest/stubs/gamepad_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/serial_stubs.c
    src/core/commandtbl.c
    src/core/funtbl.c
    src/core/MMBasic.c
    src/core/mmb_globals.c
    src/core/tokentbl.c
    src/core/vartbl.c
    src/functions/fun_sprite.c
    src/third_party/spbmp.c
    src/third_party/upng.c
)

target_compile_definitions(test_fun_sprite PUBLIC ENABLE_GTEST_EXTRAS)

configure_sdl2(test_fun_sprite FALSE)

target_link_libraries(test_fun_sprite
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_fun_sprite)

################################################################################
# test_gpio
################################################################################

add_executable(test_gpio
    ${MMB4L_COMMON_TEST_FILES}
    src/common/features.c
    src/common/gpio.c
    src/common/mmtime.c
    src/common/mmtime_${PLATFORM}.c
    src/common/gtest/gpio_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_gpio FALSE)

target_link_libraries(test_gpio
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_gpio NO_PRETTY_VALUES)

################################################################################
# test_gamepad
################################################################################

add_executable(test_gamepad
    ${MMB4L_COMMON_TEST_FILES}
    src/common/events.c
    src/common/gamepad.c
    src/common/gtest/gamepad_test.cxx
    src/common/gtest/stubs/file_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/sdl2_stubs.c
)

configure_sdl2(test_gamepad FALSE)

target_link_libraries(test_gamepad
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_gamepad)

################################################################################
# test_graphics
################################################################################

add_executable(test_graphics
    ${MMB4L_COMMON_TEST_FILES}
    src/common/events.c
    src/common/features.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/fonttbl.c
    src/common/graphics.c
    src/common/memory.c
    src/common/mmresult.c
    src/common/options.c
    src/common/path.c
    src/common/streamio.c
    src/common/gtest/graphics_test.cxx
    src/common/gtest/graphics_blit_test.cxx
    src/common/gtest/stubs/console_stubs.c
    src/common/gtest/stubs/gamepad_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/serial_stubs.c
    src/common/gtest/stubs/sdl2_stubs.c
    src/third_party/upng.c
)

configure_sdl2(test_graphics FALSE)

target_link_libraries(test_graphics
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_graphics)

################################################################################
# test_safe_buffer
################################################################################

add_executable(test_safe_buffer
    ${MMB4L_COMMON_TEST_FILES}
    src/common/gtest/safe_buffer_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_safe_buffer FALSE)

target_link_libraries(test_safe_buffer
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_safe_buffer)

################################################################################
# test_hash
################################################################################

add_executable(test_hash
    ${MMB4L_COMMON_TEST_FILES}
    src/common/gtest/hash_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_hash FALSE)

target_link_libraries(test_hash
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_hash)

################################################################################
# test_image
################################################################################

add_executable(test_image
    ${MMB4L_COMMON_TEST_FILES}
    src/common/gtest/image_test.cxx
    src/common/events.c
    src/common/features.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/fonttbl.c
    src/common/graphics.c
    src/common/image.c
    src/common/memory.c
    src/common/options.c
    src/common/parse.c
    src/common/path.c
    src/common/sprite.c
    src/common/streamio.c
    src/common/system_${PLATFORM}.c
    src/common/gtest/stubs/audio_stubs.c
    src/common/gtest/stubs/display_stubs.c
    src/common/gtest/stubs/flash_stubs.c
    src/common/gtest/stubs/gamepad_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/serial_stubs.c
    src/core/commandtbl.c
    src/core/funtbl.c
    src/core/MMBasic.c
    src/core/mmb_globals.c
    src/core/tokentbl.c
    src/core/vartbl.c
    src/third_party/picojpeg.c
    src/third_party/spbmp.c
    src/third_party/toojpeg_streaming.cpp
    src/third_party/upng.c
)

target_compile_definitions(test_image PUBLIC ENABLE_GTEST_EXTRAS)

configure_sdl2(test_image FALSE)

target_link_libraries(test_image
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_image)

################################################################################
# test_keyboard
################################################################################

add_executable(test_keyboard
    ${MMB4L_COMMON_TEST_FILES}
    src/common/keyboard.c
    src/common/gtest/keyboard_test.cxx
    src/common/gtest/stubs/file_stubs.c
    src/common/gtest/stubs/keybuf_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
)

configure_sdl2(test_keyboard FALSE)

target_link_libraries(test_keyboard
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_keyboard)

################################################################################
# test_logger
################################################################################

add_executable(test_logger
    ${MMB4L_COMMON_TEST_FILES}
    src/common/gtest/logger_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_logger FALSE)

target_link_libraries(test_logger
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_logger)

################################################################################
# test_options
################################################################################

add_executable(test_options
    ${MMB4L_COMMON_TEST_FILES}
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/options.c
    src/common/path.c
    src/common/streamio.c
    src/common/gtest/options_test.cxx
    src/common/gtest/stubs/serial_stubs.c
)

configure_sdl2(test_options FALSE)

target_compile_definitions(test_options PUBLIC OPTION_TESTS)

target_link_libraries(test_options
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_options)

################################################################################
# test_parse
################################################################################

add_executable(test_parse
    ${MMB4L_COMMON_TEST_FILES}
    src/common/features.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/memory.c
    src/common/mmgetline.c
    src/common/mmresult.c
    src/common/mmtime.c
    src/common/mmtime_${PLATFORM}.c
    src/common/options.c
    src/common/parse.c
    src/common/path.c
    src/common/program.c
    src/common/rx_buf.c
    src/common/streamio.c
    src/common/gtest/parse_test.cxx
    src/common/gtest/test_helper.c
    src/common/gtest/stubs/audio_stubs.c
    src/common/gtest/stubs/console_stubs.c
    src/common/gtest/stubs/display_stubs.c
    src/common/gtest/stubs/flash_stubs.c
    src/common/gtest/stubs/gamepad_stubs.c
    src/common/gtest/stubs/graphics_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/keybuf_stubs.c
    src/common/gtest/stubs/serial_stubs.c
    src/core/commandtbl.c
    src/core/funtbl.c
    src/core/MMBasic.c
    src/core/mmb_globals.c
    src/core/tokentbl.c
    src/core/vartbl.c
)

configure_sdl2(test_parse FALSE)

target_link_libraries(test_parse
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_parse)

################################################################################
# test_path
################################################################################

add_executable(test_path
    ${MMB4L_COMMON_TEST_FILES}
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/memory.c
    src/common/path.c
    src/common/rx_buf.c
    src/common/streamio.c
    src/common/gtest/path_test.cxx
    src/common/gtest/stubs/console_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/serial_stubs.c
)

configure_sdl2(test_path FALSE)

target_link_libraries(test_path
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_path)

################################################################################
# test_program
################################################################################

add_executable(test_program
    ${MMB4L_COMMON_TEST_FILES}
    src/common/features.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/streamio.c
    src/common/memory.c
    src/common/mmgetline.c
    src/common/mmtime.c
    src/common/mmtime_${PLATFORM}.c
    src/common/options.c
    src/common/path.c
    src/common/parse.c
    src/common/program.c
    src/common/rx_buf.c
    src/common/gtest/program_test.cxx
    src/common/gtest/test_helper.c
    src/common/gtest/stubs/audio_stubs.c
    src/common/gtest/stubs/console_stubs.c
    src/common/gtest/stubs/display_stubs.c
    src/common/gtest/stubs/flash_stubs.c
    src/common/gtest/stubs/gamepad_stubs.c
    src/common/gtest/stubs/graphics_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/keybuf_stubs.c
    src/common/gtest/stubs/serial_stubs.c
    src/core/commandtbl.c
    src/core/funtbl.c
    src/core/MMBasic.c
    src/core/mmb_globals.c
    src/core/tokentbl.c
    src/core/vartbl.c
)

configure_sdl2(test_program FALSE)

target_compile_definitions(test_program PUBLIC ENABLE_GTEST_EXTRAS)

target_link_libraries(test_program
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_program)

################################################################################
# test_prompt
################################################################################

add_executable(test_prompt
    ${MMB4L_COMMON_TEST_FILES}
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/path.c
    src/common/prompt.c
    src/common/streamio.c
    src/common/gtest/prompt_test.cxx
    src/common/gtest/stubs/display_stubs.c
    src/common/gtest/stubs/keybuf_stubs.c
    src/common/gtest/stubs/serial_stubs.c
)

configure_sdl2(test_prompt FALSE)

target_link_libraries(test_prompt
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_prompt)

################################################################################
# test_queue
################################################################################

add_executable(test_queue
    ${MMB4L_COMMON_TEST_FILES}
    src/common/gtest/queue_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_queue FALSE)

target_link_libraries(test_queue
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_queue)

################################################################################
# test_rx_buf
################################################################################

add_executable(test_rx_buf
    ${MMB4L_COMMON_TEST_FILES}
    src/common/rx_buf.c
    src/common/gtest/rx_buf_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_rx_buf FALSE)

target_link_libraries(test_rx_buf
  gtest_main
  gmock
  gmock_main
  ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_rx_buf)

################################################################################
# test_sha256
################################################################################

add_executable(test_sha256
    ${MMB4L_COMMON_TEST_FILES}
    src/common/sha256.c
    src/common/gtest/sha256_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_sha256 FALSE)

target_link_libraries(test_sha256
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_sha256)

################################################################################
# test_sprite
################################################################################

add_executable(test_sprite
    ${MMB4L_COMMON_TEST_FILES}
    src/common/events.c
    src/common/features.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/fonttbl.c
    src/common/streamio.c
    src/common/graphics.c
    src/common/memory.c
    src/common/options.c
    src/common/path.c
    src/common/sprite.c
    src/common/gtest/sprite_test.cxx
    src/common/gtest/stubs/console_stubs.c
    src/common/gtest/stubs/gamepad_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/serial_stubs.c
    src/common/gtest/stubs/sdl2_stubs.c
    src/third_party/upng.c
)

configure_sdl2(test_sprite FALSE)

target_link_libraries(test_sprite
    gtest_main
    gmock
    gmock_main
)

gtest_discover_tests(test_sprite)

################################################################################
# test_stack
################################################################################

add_executable(test_stack
    ${MMB4L_COMMON_TEST_FILES}
    src/common/gtest/stack_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_stack FALSE)

target_link_libraries(test_stack
  gtest_main
  gmock
  gmock_main
)

gtest_discover_tests(test_stack)

################################################################################
# test_streamio
################################################################################

add_executable(test_streamio
    ${MMB4L_COMMON_TEST_FILES}
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/path.c
    src/common/streamio.c
    src/common/gtest/streamio_test.cxx
    src/common/gtest/stubs/serial_stubs.c
)

configure_sdl2(test_streamio FALSE)

target_compile_definitions(test_streamio PUBLIC ENABLE_GTEST_EXTRAS)

target_link_libraries(test_streamio
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_streamio)

################################################################################
# test_mmbasic_core
################################################################################

add_executable(test_mmbasic_core
    ${MMB4L_COMMON_TEST_FILES}
    src/common/features.c
    src/common/file.c
    src/common/file_${PLATFORM}.c
    src/common/fonttbl.c
    src/common/streamio.c
    src/common/memory.c
    src/common/mmtime.c
    src/common/mmtime_${PLATFORM}.c
    src/common/options.c
    src/common/parse.c
    src/common/path.c
    src/common/program.c
    src/common/rx_buf.c
    src/common/gtest/test_helper.c
    src/common/gtest/stubs/audio_stubs.c
    src/common/gtest/stubs/console_stubs.c
    src/common/gtest/stubs/display_stubs.c
    src/common/gtest/stubs/flash_stubs.c
    src/common/gtest/stubs/gamepad_stubs.c
    src/common/gtest/stubs/graphics_stubs.c
    src/common/gtest/stubs/interrupt_stubs.c
    src/common/gtest/stubs/keybuf_stubs.c
    src/common/gtest/stubs/serial_stubs.c
    src/core/commandtbl.c
    src/core/funtbl.c
    src/core/MMBasic.c
    src/core/mmb_globals.c
    src/core/tokentbl.c
    src/core/vartbl.c
    src/core/gtest/mmbasic_core_test.cxx
)

configure_sdl2(test_mmbasic_core FALSE)

target_compile_definitions(test_mmbasic_core PUBLIC ENABLE_GTEST_EXTRAS)

target_link_libraries(test_mmbasic_core
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_mmbasic_core)

################################################################################
# test_tokentbl
################################################################################

add_executable(test_tokentbl
    ${MMB4L_COMMON_TEST_FILES}
    src/core/tokentbl.c
    src/core/gtest/tokentbl_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_tokentbl FALSE)

target_link_libraries(test_tokentbl
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_tokentbl)

################################################################################
# test_vartbl
################################################################################

add_executable(test_vartbl
    ${MMB4L_COMMON_TEST_FILES}
    src/core/vartbl.c
    src/core/gtest/vartbl_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_vartbl FALSE)

target_link_libraries(test_vartbl
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_vartbl)

################################################################################
# third_party/test_spbmp
################################################################################

add_executable(test_spbmp
    ${MMB4L_COMMON_TEST_FILES}
    src/third_party/spbmp.c
    src/third_party/gtest/spbmp_test.cxx
    src/common/gtest/stubs/file_stubs.c
)

configure_sdl2(test_spbmp FALSE)

target_link_libraries(test_spbmp
    gtest_main
    gmock
    gmock_main
    ${GCOV_LINK_LIBRARY}
)

gtest_discover_tests(test_spbmp)
