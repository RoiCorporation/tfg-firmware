#!/bin/bash

set -e  # Stop if any command fails.

# Delete the build folder and create a new one.
echo "🗑️ Deleting existing build directory"
rm -rf build

# Create a clean build directory and switch into it.
echo "📁 Creating and entering build directory"
mkdir build
cd build

# Configure the project and generate Ninja build files using CMake.
echo "🧱 Generating Ninja build system with CMake"
cmake -G Ninja ..

# Compile and link the project using Ninja.
echo "⛏️ Compiling project with Ninja"
ninja

# Flash the ELF firmware to the Pico and start execution.
echo "🚀 Flashing firmware to Pico and starting execution"
picotool load "wireless_station_firmware.elf" -fx

echo "✅ Firmware successfully built and flashed"
