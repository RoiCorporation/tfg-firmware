cmake_minimum_required(VERSION 3.13)
project(tfg_firmware C CXX ASM)

include(${CMAKE_CURRENT_SOURCE_DIR}/wireless_station_firmware/pico_sdk_import.cmake)

# Point to the wireless sensor firmware folder.
add_subdirectory(wireless_station_firmware)