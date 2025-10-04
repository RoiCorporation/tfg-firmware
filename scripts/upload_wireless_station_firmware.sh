#!/bin/bash

set -e  # Stop if any command fails.

# Delete the build folder and create a new one.
echo "🗑️ Removing the old build folder"
rm -rf build

echo "♻️ Recreating the build folder"
mkdir build
cd build

# Generate the build files to be used by Ninja.
echo "🧱 Configuring CMake"
REPO_ROOT="$(cd .. && pwd)"
SDK_PATH="$REPO_ROOT/pico-sdk"
TOOLCHAIN_PATH="$(brew --prefix riscv-gnu-toolchain)"
PATH="$(brew --prefix ninja)/bin:$PATH" \
PICO_SDK_PATH="$SDK_PATH" \
PICO_TOOLCHAIN_PATH="$TOOLCHAIN_PATH" \
cmake "$REPO_ROOT" -G Ninja \
  -DPICO_SDK_PATH="$SDK_PATH" \
  -DPICO_BOARD=pico2_w \
  -DPICO_PLATFORM=rp2350-riscv \
  -DCMAKE_SYSTEM_NAME=Generic \
  -DCMAKE_C_COMPILER="$TOOLCHAIN_PATH/bin/riscv64-unknown-elf-gcc" \
  -DCMAKE_CXX_COMPILER="$TOOLCHAIN_PATH/bin/riscv64-unknown-elf-g++" \
  -DCMAKE_OBJDUMP="$TOOLCHAIN_PATH/bin/riscv64-unknown-elf-objdump" \
  -DCMAKE_OBJCOPY="$TOOLCHAIN_PATH/bin/riscv64-unknown-elf-objcopy" \
  -DCMAKE_C_FLAGS="-march=rv32gc_zba_zbb_zbs_zbkb -mabi=ilp32" \
  -DCMAKE_CXX_FLAGS="-march=rv32gc_zba_zbb_zbs_zbkb -mabi=ilp32" \
  -DCMAKE_ASM_FLAGS="-march=rv32gc_zba_zbb_zbs_zbkb -mabi=ilp32" \
  -DPICO_STDIO_USB=1 \
  -DPICO_STDIO_UART=0 \
  -DPICO_STDIO_USB_RESET_INTERFACE_SUPPORT=1 \
  -DPICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1

# Run the Ninja build executer tool.
echo "⛏️ Building the executable file"
ninja

# Upload the .elf file to the board using picotool.
echo "🚀 Uploading the program to the board"
picotool load "$PWD/wireless_station_firmware/tfg_firmware.uf2" -f -F
