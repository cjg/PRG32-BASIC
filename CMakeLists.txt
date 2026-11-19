cmake_minimum_required(VERSION 3.15)
project(prg32_basic C)

enable_language(C ASM)

set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_COMPILER riscv32-esp-elf-gcc)
set(CMAKE_ASM_COMPILER riscv32-esp-elf-gcc)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=rv32imc_zicsr -mabi=ilp32 -ffreestanding -fno-builtin -Os")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcmodel=medany -msmall-data-limit=0 -fno-jump-tables -fno-tree-switch-conversion")

set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -march=rv32imc_zicsr -mabi=ilp32 -x assembler-with-cpp")

set(CMAKE_C_LINK_FLAGS "-Wl,--no-relax -Wl,-T,${PROJECT_BINARY_DIR}/cart.ld -nostdlib")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT DEFINED ENV{ESP_PYTHON})
    message(SEND_ERROR "ESP SDK not initialized")
endif ()


set(PYTHON_EXE "$ENV{IDF_PYTHON_ENV_PATH}/bin/$ENV{ESP_PYTHON}")

if(NOT DEFINED ENV{PRG32_HOME})
    message(SEND_ERROR "PRG32_HOME not set")
endif ()

set(PRG32_HOME "$ENV{PRG32_HOME}")

add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/portable_stubs.S
        DEPENDS hacks/generate-stubs-and-linker.py
        COMMAND ${PYTHON_EXE} -u ${PROJECT_SOURCE_DIR}/hacks/generate-stubs-and-linker.py ${PRG32_HOME} ${PROJECT_BINARY_DIR}/portable_stubs.S ${PROJECT_BINARY_DIR}/cart.ld
)

add_executable(prg32-basic
        ${PROJECT_BINARY_DIR}/portable_stubs.S
        src/arena.c
        src/basic_rt.c
        src/lexer.c
        src/logging.c
        src/parser.c
        src/strpool.c
        src/symbol_table.c
        src/stdlib.c
)

target_include_directories(prg32-basic PRIVATE src ${PRG32_HOME}/components/prg32/include)
