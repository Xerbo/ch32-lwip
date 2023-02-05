#!/usr/bin/env bash
set -e

URL="https://github.com/hydrausb3/riscv-none-elf-gcc-xpack/releases/download/12.2.0-1/xpack-riscv-none-elf-gcc-12.2.0-1-linux-x64.tar.gz"
FILE="/tmp/xpack-riscv-none-elf-gcc-12.2.0-1-linux-x64.tar.gz"

# Download
if [ ! -f "$FILE" ]; then
    wget -O "$FILE" "$URL"
fi

# Extract
if [ -d "riscv-none-elf-gcc" ]; then
    rm -rf "riscv-none-elf-gcc"
fi
tar -xf "$FILE"
mv "xpack-riscv-none-elf-gcc-12.2.0-1" "riscv-none-elf-gcc"
