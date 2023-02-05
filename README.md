# LwIP on the CH32V307

Example using LwIP on the CH32V307 with an entirely FOSS toolchain.

By default the WCH SDKs use a binary block called `libwchnet` as a TCP/IP stack, this project replaces that with LwIP.

NOTE: You cannot use the "standard" `Delay_Ms`/`Delay_Us` functions in this project, use the provided `usleep` instead.

## Building

```sh
# Get dependencies
./scripts/toolchain_dl.sh
./scripts/sdk_dl.sh ch32v307
# Set FLASH to 256K and RAM to 64K in CH32V307-SDK/Ld/Link.ld
# Build it
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
# Flash it
riscv-openocd-wch/src/openocd -f snippets/wch-riscv.cfg -c "init;halt;program build/ch32-lwip;exit"
# Press the reset button
```

## Licensing issues

From (limited) observations it seems the SDKs are licensed under `Apache-2.0` however YMMV.

The following files in this repository are **unlicensed**:
 - `snippets/50-wch.rules`
 - `snippets/wch-riscv.conf`

## Toolchain

### GCC

This project uses the [xPack GNU RISC-V Embedded GCC](https://github.com/hydrausb3/riscv-none-elf-gcc-xpack/) toolchain from the HydraUSB3 project.

### OpenOCD

This project uses this public snapshot of WCH's OpenOCD: https://github.com/Seneral/riscv-openocd-wch

```sh
git clone https://github.com/Seneral/riscv-openocd-wch
cd riscv-openocd-wch
./bootstrap
./configure --disable-werror --enable-wlink
make -j$(nproc)
```

## References

 - https://github.com/hydrausb3
 - https://github.com/openwch/ch32v307
 - https://git.minori.work/Embedded_Projects/CH32V307_Template
 - https://www.wch.cn/ (contains more information than the English website)
