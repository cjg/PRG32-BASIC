cmake_minimum_required(VERSION 3.15)
project(prg32_basic C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# source files for the BASIC interpreter core
set(BASIC_CORE_SOURCES
        src/arena.c
)

# desktop Host Executable Launcher
add_executable(prg32-basic
        src/main.c
        ${BASIC_CORE_SOURCES}
)

target_include_directories(prg32-basic PRIVATE src)
