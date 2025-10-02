cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion RISCV_ZCB_RPI_2_1_1_3)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

project(tfg_firmware C CXX ASM)

# Pull in the SDK.
include(${CMAKE_CURRENT_SOURCE_DIR}/pico_sdk_import.cmake)
pico_sdk_init()

# Define the target (must be "tfg_firmware")
add_executable(tfg_firmware
    src/wireless_station.c
    src/sensors_functions.c
)

set_target_properties(tfg_firmware PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} SUFFIX ".elf")

# Define the folder to look for .h files.
target_include_directories(tfg_firmware PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

pico_set_program_name(tfg_firmware "wireless_station")
pico_set_program_version(tfg_firmware "0.1")

# Enable USB stdio, disable UART
pico_enable_stdio_uart(tfg_firmware 0)
pico_enable_stdio_usb(tfg_firmware 1)

# Link required libraries
target_link_libraries(tfg_firmware
    pico_stdlib
    hardware_i2c
    hardware_pio
)

# Generate .elf, .bin, .hex, .uf2
pico_add_extra_outputs(tfg_firmware)
