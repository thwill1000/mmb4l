cmake_minimum_required(VERSION 3.10)

# set the project name
project(MMBasic)

add_definitions(-funsigned-char)

# add the executable
add_executable(MMBasic DOS/Source/Main.c DOS/Source/console.c DOS/Source/Memory.c DOS/Source/DOS_Misc.c DOS/Source/editor_stubs.c DOS/Source/file_io_stubs.c MMBasic/MMBasic.c MMBasic/Operators.c MMBasic/Functions.c MMBasic/Commands.c)

target_link_libraries(MMBasic m)
