cmake_minimum_required(VERSION 3.22.1)
project(SDL2Example)

# Set C standard
set(CMAKE_C_STANDARD 17) # Or 11, or 17, as long as it's C99 or newer
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-funsigned-char)

# Find system libraries
find_library(LOG_LIBRARY log)
find_library(GLES_LIBRARY GLESv2)
find_library(ANDROID_LIBRARY android)

# Add SDL2 as a subdirectory (this builds SDL2 from source)
set(SDL2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../jni/SDL)
add_subdirectory(${SDL2_DIR} SDL2)

set(MMB4L_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../mmb4l/src)

set(MMB4L_CORE_SOURCE_FILES
    ${MMB4L_SRC_DIR}/core/commandtbl.c
    ${MMB4L_SRC_DIR}/core/Commands.c
    ${MMB4L_SRC_DIR}/core/Functions.c
    ${MMB4L_SRC_DIR}/core/funtbl.c
    ${MMB4L_SRC_DIR}/core/maths.c
    ${MMB4L_SRC_DIR}/core/MMBasic.c
    ${MMB4L_SRC_DIR}/core/Operators.c
    ${MMB4L_SRC_DIR}/core/tokentbl.c
    ${MMB4L_SRC_DIR}/core/vartbl.c
)

set(MMB4L_COMMANDS_SOURCE_FILES
    ${MMB4L_SRC_DIR}/commands/cmd_autosave.c
    ${MMB4L_SRC_DIR}/commands/cmd_arc.c
    ${MMB4L_SRC_DIR}/commands/cmd_blit.c
    ${MMB4L_SRC_DIR}/commands/cmd_box.c
    ${MMB4L_SRC_DIR}/commands/cmd_call.c
    ${MMB4L_SRC_DIR}/commands/cmd_case.c
    ${MMB4L_SRC_DIR}/commands/cmd_cfunction.c
    ${MMB4L_SRC_DIR}/commands/cmd_chdir.c
    ${MMB4L_SRC_DIR}/commands/cmd_circle.c
    ${MMB4L_SRC_DIR}/commands/cmd_clear.c
    ${MMB4L_SRC_DIR}/commands/cmd_cls.c
    ${MMB4L_SRC_DIR}/commands/cmd_close.c
    ${MMB4L_SRC_DIR}/commands/cmd_colour.c
    ${MMB4L_SRC_DIR}/commands/cmd_console.c
    ${MMB4L_SRC_DIR}/commands/cmd_const.c
    ${MMB4L_SRC_DIR}/commands/cmd_continue.c
    ${MMB4L_SRC_DIR}/commands/cmd_copy.c
    ${MMB4L_SRC_DIR}/commands/cmd_cursor.c
    ${MMB4L_SRC_DIR}/commands/cmd_device.c
    ${MMB4L_SRC_DIR}/commands/cmd_dim.c
    ${MMB4L_SRC_DIR}/commands/cmd_do.c
    ${MMB4L_SRC_DIR}/commands/cmd_dummy.c
    ${MMB4L_SRC_DIR}/commands/cmd_edit.c
    ${MMB4L_SRC_DIR}/commands/cmd_else.c
    ${MMB4L_SRC_DIR}/commands/cmd_end.c
    ${MMB4L_SRC_DIR}/commands/cmd_endfun.c
    ${MMB4L_SRC_DIR}/commands/cmd_erase.c
    ${MMB4L_SRC_DIR}/commands/cmd_error.c
    ${MMB4L_SRC_DIR}/commands/cmd_execute.c
    ${MMB4L_SRC_DIR}/commands/cmd_exit.c
    ${MMB4L_SRC_DIR}/commands/cmd_exitfor.c
    ${MMB4L_SRC_DIR}/commands/cmd_files.c
    ${MMB4L_SRC_DIR}/commands/cmd_flash.c
    ${MMB4L_SRC_DIR}/commands/cmd_font.c
    ${MMB4L_SRC_DIR}/commands/cmd_for.c
    ${MMB4L_SRC_DIR}/commands/cmd_framebuffer.c
    ${MMB4L_SRC_DIR}/commands/cmd_gamepad.c
    ${MMB4L_SRC_DIR}/commands/cmd_gosub.c
    ${MMB4L_SRC_DIR}/commands/cmd_goto.c
    ${MMB4L_SRC_DIR}/commands/cmd_graphics.c
    ${MMB4L_SRC_DIR}/commands/cmd_gui.c
    ${MMB4L_SRC_DIR}/commands/cmd_if.c
    ${MMB4L_SRC_DIR}/commands/cmd_image.c
    ${MMB4L_SRC_DIR}/commands/cmd_in.c
    ${MMB4L_SRC_DIR}/commands/cmd_inc.c
    ${MMB4L_SRC_DIR}/commands/cmd_input.c
    ${MMB4L_SRC_DIR}/commands/cmd_ireturn.c
    ${MMB4L_SRC_DIR}/commands/cmd_kill.c
    ${MMB4L_SRC_DIR}/commands/cmd_let.c
    ${MMB4L_SRC_DIR}/commands/cmd_line.c
    ${MMB4L_SRC_DIR}/commands/cmd_lineinput.c
    ${MMB4L_SRC_DIR}/commands/cmd_list.c
    ${MMB4L_SRC_DIR}/commands/cmd_load.c
    ${MMB4L_SRC_DIR}/commands/cmd_loop.c
    ${MMB4L_SRC_DIR}/commands/cmd_longstring.c
    ${MMB4L_SRC_DIR}/commands/cmd_memory.c
    ${MMB4L_SRC_DIR}/commands/cmd_mid.c
    ${MMB4L_SRC_DIR}/commands/cmd_mkdir.c
    ${MMB4L_SRC_DIR}/commands/cmd_mkfile.c
    ${MMB4L_SRC_DIR}/commands/cmd_mmdebug.c
    ${MMB4L_SRC_DIR}/commands/cmd_mode.c
    ${MMB4L_SRC_DIR}/commands/cmd_new.c
    ${MMB4L_SRC_DIR}/commands/cmd_next.c
    ${MMB4L_SRC_DIR}/commands/cmd_null.c
    ${MMB4L_SRC_DIR}/commands/cmd_on.c
    ${MMB4L_SRC_DIR}/commands/cmd_open.c
    ${MMB4L_SRC_DIR}/commands/cmd_option.c
    ${MMB4L_SRC_DIR}/commands/cmd_out.c
    ${MMB4L_SRC_DIR}/commands/cmd_page.c
    ${MMB4L_SRC_DIR}/commands/cmd_pause.c
    ${MMB4L_SRC_DIR}/commands/cmd_pin.c
    ${MMB4L_SRC_DIR}/commands/cmd_pixel.c
    ${MMB4L_SRC_DIR}/commands/cmd_play.c
    ${MMB4L_SRC_DIR}/commands/cmd_poke.c
    ${MMB4L_SRC_DIR}/commands/cmd_print.c
    ${MMB4L_SRC_DIR}/commands/cmd_pulse.c
    ${MMB4L_SRC_DIR}/commands/cmd_pin.c
    ${MMB4L_SRC_DIR}/commands/cmd_polygon.c
    ${MMB4L_SRC_DIR}/commands/cmd_quit.c
    ${MMB4L_SRC_DIR}/commands/cmd_randomize.c
    ${MMB4L_SRC_DIR}/commands/cmd_rbox.c
    ${MMB4L_SRC_DIR}/commands/cmd_read.c
    ${MMB4L_SRC_DIR}/commands/cmd_rename.c
    ${MMB4L_SRC_DIR}/commands/cmd_restore.c
    ${MMB4L_SRC_DIR}/commands/cmd_return.c
    ${MMB4L_SRC_DIR}/commands/cmd_rmdir.c
    ${MMB4L_SRC_DIR}/commands/cmd_run.c
    ${MMB4L_SRC_DIR}/commands/cmd_save.c
    ${MMB4L_SRC_DIR}/commands/cmd_seek.c
    ${MMB4L_SRC_DIR}/commands/cmd_select.c
    ${MMB4L_SRC_DIR}/commands/cmd_setenv.c
    ${MMB4L_SRC_DIR}/commands/cmd_setpin.c
    ${MMB4L_SRC_DIR}/commands/cmd_settick.c
    ${MMB4L_SRC_DIR}/commands/cmd_settitle.c
    ${MMB4L_SRC_DIR}/commands/cmd_sort.c
    ${MMB4L_SRC_DIR}/commands/cmd_sprite.c
    ${MMB4L_SRC_DIR}/commands/cmd_subfun.c
    ${MMB4L_SRC_DIR}/commands/cmd_system.c
    ${MMB4L_SRC_DIR}/commands/cmd_text.c
    ${MMB4L_SRC_DIR}/commands/cmd_timer.c
    ${MMB4L_SRC_DIR}/commands/cmd_trace.c
    ${MMB4L_SRC_DIR}/commands/cmd_triangle.c
    ${MMB4L_SRC_DIR}/commands/cmd_troff.c
    ${MMB4L_SRC_DIR}/commands/cmd_tron.c
    ${MMB4L_SRC_DIR}/commands/cmd_xmodem.c
)

set(MMB4L_FUNCTIONS_SOURCE_FILES
    ${MMB4L_SRC_DIR}/functions/fun_at.c
    ${MMB4L_SRC_DIR}/functions/fun_atchar.c
    ${MMB4L_SRC_DIR}/functions/fun_atan2.c
    ${MMB4L_SRC_DIR}/functions/fun_bin.c
    ${MMB4L_SRC_DIR}/functions/fun_bin2str.c
    ${MMB4L_SRC_DIR}/functions/fun_bound.c
    ${MMB4L_SRC_DIR}/functions/fun_call.c
    ${MMB4L_SRC_DIR}/functions/fun_choice.c
    ${MMB4L_SRC_DIR}/functions/fun_chr.c
    ${MMB4L_SRC_DIR}/functions/fun_classic.c
    ${MMB4L_SRC_DIR}/functions/fun_cwd.c
    ${MMB4L_SRC_DIR}/functions/fun_date.c
    ${MMB4L_SRC_DIR}/functions/fun_datetime.c
    ${MMB4L_SRC_DIR}/functions/fun_day.c
    ${MMB4L_SRC_DIR}/functions/fun_device.c
    ${MMB4L_SRC_DIR}/functions/fun_dir.c
    ${MMB4L_SRC_DIR}/functions/fun_eof.c
    ${MMB4L_SRC_DIR}/functions/fun_epoch.c
    ${MMB4L_SRC_DIR}/functions/fun_eval.c
    ${MMB4L_SRC_DIR}/functions/fun_format.c
    ${MMB4L_SRC_DIR}/functions/fun_field.c
    ${MMB4L_SRC_DIR}/functions/fun_gamepad.c
    ${MMB4L_SRC_DIR}/functions/fun_hex.c
    ${MMB4L_SRC_DIR}/functions/fun_hres.c
    ${MMB4L_SRC_DIR}/functions/fun_inputstr.c
    ${MMB4L_SRC_DIR}/functions/fun_instr.c
    ${MMB4L_SRC_DIR}/functions/fun_json.c
    ${MMB4L_SRC_DIR}/functions/fun_keydown.c
    ${MMB4L_SRC_DIR}/functions/fun_lcompare.c
    ${MMB4L_SRC_DIR}/functions/fun_left.c
    ${MMB4L_SRC_DIR}/functions/fun_lgetbyte.c
    ${MMB4L_SRC_DIR}/functions/fun_lgetstr.c
    ${MMB4L_SRC_DIR}/functions/fun_linstr.c
    ${MMB4L_SRC_DIR}/functions/fun_llen.c
    ${MMB4L_SRC_DIR}/functions/fun_loc.c
    ${MMB4L_SRC_DIR}/functions/fun_lof.c
    ${MMB4L_SRC_DIR}/functions/fun_max.c
    ${MMB4L_SRC_DIR}/functions/fun_mid.c
    ${MMB4L_SRC_DIR}/functions/fun_min.c
    ${MMB4L_SRC_DIR}/functions/fun_mmcmdline.c
    ${MMB4L_SRC_DIR}/functions/fun_mmdevice.c
    ${MMB4L_SRC_DIR}/functions/fun_mminfo.c
    ${MMB4L_SRC_DIR}/functions/fun_oct.c
    ${MMB4L_SRC_DIR}/functions/fun_peek.c
    ${MMB4L_SRC_DIR}/functions/fun_pin.c
    ${MMB4L_SRC_DIR}/functions/fun_pixel.c
    ${MMB4L_SRC_DIR}/functions/fun_port.c
    ${MMB4L_SRC_DIR}/functions/fun_right.c
    ${MMB4L_SRC_DIR}/functions/fun_rgb.c
    ${MMB4L_SRC_DIR}/functions/fun_sprite.c
    ${MMB4L_SRC_DIR}/functions/fun_str.c
    ${MMB4L_SRC_DIR}/functions/fun_string.c
    ${MMB4L_SRC_DIR}/functions/fun_str2bin.c
    ${MMB4L_SRC_DIR}/functions/fun_time.c
    ${MMB4L_SRC_DIR}/functions/fun_timer.c
    ${MMB4L_SRC_DIR}/functions/fun_vres.c
)

set(MMB4L_OPERATORS_SOURCE_FILES
    ${MMB4L_SRC_DIR}/operators/op_inv.c
)

set(MMB4L_COMMON_SOURCE_FILES
    ${MMB4L_SRC_DIR}/common/audio.c
    ${MMB4L_SRC_DIR}/common/audio_tables.c
    ${MMB4L_SRC_DIR}/common/cmdline.c
    ${MMB4L_SRC_DIR}/common/codepage.c
    ${MMB4L_SRC_DIR}/common/console.c
    ${MMB4L_SRC_DIR}/common/cstring.c
    ${MMB4L_SRC_DIR}/common/display.c
    ${MMB4L_SRC_DIR}/common/editor.c
    ${MMB4L_SRC_DIR}/common/error.c
    ${MMB4L_SRC_DIR}/common/events.c
    ${MMB4L_SRC_DIR}/common/features.c
    ${MMB4L_SRC_DIR}/common/file.c
    ${MMB4L_SRC_DIR}/common/flash.c
    ${MMB4L_SRC_DIR}/common/fonttbl.c
    ${MMB4L_SRC_DIR}/common/gamepad.c
    ${MMB4L_SRC_DIR}/common/gpio.c
    ${MMB4L_SRC_DIR}/common/graphics.c
    ${MMB4L_SRC_DIR}/common/interrupt.c
    ${MMB4L_SRC_DIR}/common/keyboard.c
    ${MMB4L_SRC_DIR}/common/logger.c
    ${MMB4L_SRC_DIR}/common/memory.c
    ${MMB4L_SRC_DIR}/common/mmgetline.c
    ${MMB4L_SRC_DIR}/common/mmresult.c
    ${MMB4L_SRC_DIR}/common/mmtime.c
    ${MMB4L_SRC_DIR}/common/options.c
    ${MMB4L_SRC_DIR}/common/parse.c
    ${MMB4L_SRC_DIR}/common/path.c
    ${MMB4L_SRC_DIR}/common/process.c
    ${MMB4L_SRC_DIR}/common/program.c
    ${MMB4L_SRC_DIR}/common/prompt.c
    ${MMB4L_SRC_DIR}/common/rx_buf.c
    ${MMB4L_SRC_DIR}/common/serial.c
    ${MMB4L_SRC_DIR}/common/sprite.c
    ${MMB4L_SRC_DIR}/common/streamio.c
    ${MMB4L_SRC_DIR}/common/termgfx.c
    ${MMB4L_SRC_DIR}/common/utility.c
    ${MMB4L_SRC_DIR}/common/xmodem.c
)

set(MMB4L_THIRD_PARTY_SOURCE_FILES
    ${MMB4L_SRC_DIR}/third_party/cJSON.c
    ${MMB4L_SRC_DIR}/third_party/dr_audio.c
    ${MMB4L_SRC_DIR}/third_party/hxcmod.c
    ${MMB4L_SRC_DIR}/third_party/spbmp.c
    ${MMB4L_SRC_DIR}/third_party/upng.c
)

set(MMB4L_SOURCE_FILES
    ${MMB4L_COMMANDS_SOURCE_FILES}
    ${MMB4L_FUNCTIONS_SOURCE_FILES}
    ${MMB4L_OPERATORS_SOURCE_FILES}
    ${MMB4L_COMMON_SOURCE_FILES}
    ${MMB4L_THIRD_PARTY_SOURCE_FILES}
    ${MMB4L_SRC_DIR}/main.c)

# Create our main library
add_library(main SHARED
     android.cpp
     file_android.cpp
     saf_bridge.cpp
     ${MMB4L_CORE_SOURCE_FILES}
     ${MMB4L_SOURCE_FILES}
)

# Add this line to tell the compiler to show includes
target_compile_options(main PRIVATE -H)

# Link libraries - SDL2 and SDL2main will be available after add_subdirectory
target_link_libraries(main
    SDL2
    SDL2main
    ${LOG_LIBRARY}
    ${GLES_LIBRARY}
    ${ANDROID_LIBRARY}
)

# Include SDL2 headers
target_include_directories(main PRIVATE
    .
    ${MMB4L_SRC_DIR}
    ${SDL2_DIR}/include
)
