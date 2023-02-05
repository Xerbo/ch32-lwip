# Some systems will set this to linux-gnu automatically
# Which adds incompatible linker flags
set(CMAKE_SYSTEM_NAME none)

set(TOOLCHAIN_PREFIX ${CMAKE_CURRENT_LIST_DIR}/riscv-none-elf-gcc/bin/riscv-none-elf)
set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_AR           ${TOOLCHAIN_PREFIX}-ar)
