cmake_minimum_required(VERSION 3.15)
project(prg32_basic C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT DEFINED ENV{ESP_PYTHON})
    message(SEND_ERROR "ESP SDK not initialized")
endif ()

set(PYTHON_EXE "$ENV{ESP_PYTHON}")

if(NOT DEFINED ENV{PRG32_HOME})
    message(SEND_ERROR "PRG32_HOME not set")
endif ()

set(PRG32_HOME "$ENV{PRG32_HOME}")

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/portable_stubs.S
        DEPENDS hacks/generate-portable-stubs.py
        COMMAND ${PYTHON_EXE} ${PROJECT_SOURCE_DIR}/hacks/generate-portable-stubs.py ${PRG32_HOME} ${PROJECT_BINARY_DIR}/portable_stubs.S
)

# source files for the BASIC interpreter core
set(BASIC_CORE_SOURCES
        ${PROJECT_BINARY_DIR}/portable_stubs.S
        src/arena.c
        src/basic_rt.c
        src/lexer.c
        src/logging.c
        src/parser.c
        src/strpool.c
        src/symbol_table.c
)

# desktop Host Executable Launcher
add_executable(prg32-basic
        src/main.c
        ${BASIC_CORE_SOURCES}
)

target_include_directories(prg32-basic PRIVATE src)
