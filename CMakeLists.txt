cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/toolchain.cmake)
set(SDK_PREFIX           ${CMAKE_CURRENT_LIST_DIR}/CH32V307-SDK)

# SDK paths
file(GLOB SDK_SOURCE_FILES
    ${SDK_PREFIX}/startup.S
    ${SDK_PREFIX}/Core/core_riscv.c
    ${SDK_PREFIX}/Debug/debug.c
    ${SDK_PREFIX}/Peripheral/src/*.c
    src/lwip/src/core/*.c
    src/lwip/src/core/ipv4/*.c
    src/lwip/src/netif/*.c
    src/lwip/src/apps/http/httpd.c
    src/lwip/src/apps/http/fs.c
)
set(SDK_INCLUDE_PATHS
    ${SDK_PREFIX}/Core
    ${SDK_PREFIX}/Debug
    ${SDK_PREFIX}/Peripheral/inc
    src/lwip/src/include
)

# Compile/link options
# Most of these options are just to get smaller binaries
add_compile_options(-march=rv32imac_zicsr -mabi=ilp32 -ffunction-sections -fdata-sections -Os)
add_link_options(-T ${SDK_PREFIX}/Ld/Link.ld -nostartfiles --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections)

# The actual project
project(ch32-lwip C ASM)
file(GLOB SOURCE_FILES src/*.c)
add_executable(ch32-lwip ${SOURCE_FILES} ${SDK_SOURCE_FILES})
target_include_directories(ch32-lwip PRIVATE src ${SDK_INCLUDE_PATHS})

# Some options you might want to set
set_source_files_properties(${SOURCE_FILES} -Wall -Wextra -pedantic -Wno-comment)
target_link_options(ch32-lwip PRIVATE -Wl,--print-memory-usage)
