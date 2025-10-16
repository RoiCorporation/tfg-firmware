# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion RISCV_ZCB_RPI_2_2_0_2)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(wireless_station_firmware C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(wireless_station_firmware 
    src/wireless_station_firmware.c 
    src/functions.c 
    src/utils.c
)

pico_set_program_name(wireless_station_firmware "wireless_station_firmware")
pico_set_program_version(wireless_station_firmware "0.1")

# Generate PIO header
pico_generate_pio_header(wireless_station_firmware ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(wireless_station_firmware 0)
pico_enable_stdio_usb(wireless_station_firmware 1)

# Add the standard library to the build
target_link_libraries(wireless_station_firmware
    pico_stdlib
    hardware_i2c
    hardware_pio
    hardware_pwm
)

# Add the standard include files to the build
target_include_directories(wireless_station_firmware PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
)

pico_add_extra_outputs(wireless_station_firmware)

