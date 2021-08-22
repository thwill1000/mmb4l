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
    DOS/Source/commands/inc.c
    DOS/Source/commands/longstring.c
    DOS/Source/commands/poke.c
    DOS/Source/common/console.c
    DOS/Source/common/utility.c
    DOS/Source/functions/choice.c
    DOS/Source/functions/info.c
    DOS/Source/functions/lgetbyte.c
    DOS/Source/functions/peek.c
    DOS/Source/Main.c
    DOS/Source/Memory.c
    DOS/Source/DOS_Misc.c
    DOS/Source/editor_stubs.c
    DOS/Source/file_io_stubs.c)

# add the executable
add_executable(MMBasic ${MMBASIC_CORE_SOURCE_FILES} ${MMBASIC_LINUX_SOURCE_FILES})

target_link_libraries(MMBasic m)
