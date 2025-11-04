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
cmake -G Ninja ..

# Run the Ninja build executer tool.
echo "⛏️ Building the executable file"
ninja

# Upload the .elf file to the board using picotool.
echo "🚀 Uploading the program to the board"
picotool load "central_station_firmware.elf" -fx

echo "✅ Upload complete!"