#!/bin/bash

set -e  # Stop if any command fails.

# Delete the build folder and create a new one.
echo 🗑️ Removing old build folder
rm -rf build

echo ♻️ Recreating build folder
mkdir build
cd build

# Configure CMake, creating the build files that 
# will be used by the ninja build tool.
echo 🧱 Configuring CMake
PATH=$HOME/.pico-sdk/ninja/v1.12.1:$PATH \
cmake .. -G Ninja \
  -DPICO_SDK_PATH=$HOME/.pico-sdk/sdk/2.2.0 \
  -DPICO_BOARD=pico2_w \
  -DPICO_PLATFORM=rp2350-riscv \
  -DCMAKE_SYSTEM_NAME=Generic \
  -DCMAKE_C_COMPILER=$HOME/.pico-sdk/toolchain/RISCV_ZCB_RPI_2_1_1_3/bin/riscv32-unknown-elf-gcc \
  -DCMAKE_CXX_COMPILER=$HOME/.pico-sdk/toolchain/RISCV_ZCB_RPI_2_1_1_3/bin/riscv32-unknown-elf-g++ \
  -DCMAKE_OBJDUMP=$HOME/.pico-sdk/toolchain/RISCV_ZCB_RPI_2_1_1_3/bin/riscv32-unknown-elf-objdump \
  -DCMAKE_OBJCOPY=$HOME/.pico-sdk/toolchain/RISCV_ZCB_RPI_2_1_1_3/bin/riscv32-unknown-elf-objcopy \

# Run the ninja build tool. This tool will run the 
# compiler on every .c file, link the .o files into
# the final .elf and runs picotool to generate files
# in derivative formats of the .elf.
echo ⛏️ Building executable file
ninja

# Uploads the .elf file to the board.
echo 🚀 Uploading program
$HOME/.pico-sdk/picotool/2.2.0/picotool/picotool load tfg_firmware.elf -fx