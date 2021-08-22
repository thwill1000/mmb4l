cmake_minimum_required(VERSION 3.10)

# set the project name
project(MMBasic)

add_definitions(-funsigned-char)

set(MMBASIC_CORE_SOURCE_FILES
    MMBasic/MMBasic.c
    MMBasic/Operators.c
    MMBasic/Functions.c
    MMBasic/Commands.c)

set(MMBASIC_LINUX_SOURCE_FILES
    linux-src/commands/inc.c
    linux-src/commands/longstring.c
    linux-src/commands/pause.c
    linux-src/commands/poke.c
    linux-src/commands/timer_cmd.c
    linux-src/common/console.c
    linux-src/common/utility.c
    linux-src/functions/choice.c
    linux-src/functions/device.c
    linux-src/functions/dir.c
    linux-src/functions/info.c
    linux-src/functions/lgetbyte.c
    linux-src/functions/peek.c
    linux-src/functions/timer_fun.c
    linux-src/editor.c
    linux-src/file_io.c
    linux-src/main.c
    linux-src/memory.c
    linux-src/misc.c)

# add the executable
add_executable(MMBasic ${MMBASIC_CORE_SOURCE_FILES} ${MMBASIC_LINUX_SOURCE_FILES})

target_link_libraries(MMBasic m)
