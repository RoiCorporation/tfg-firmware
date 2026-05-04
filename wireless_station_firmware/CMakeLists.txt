# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_NAME wireless_station_firmware)

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

project(${PROJECT_NAME} C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add source files to the project.
add_executable(${PROJECT_NAME} 
    src/wireless_station_firmware.c 
    src/functions.c 
    src/callbacks.c
    src/hazards.c
    src/alerts.c
    src/bme680_port.c
    src/utils.c
    src/oled_display.c
    lib/pico-ssd1306/ssd1306.c
    src/monocypher.c
    src/low_power.c
)

pico_set_program_name(${PROJECT_NAME} "${PROJECT_NAME}")
pico_set_program_version(${PROJECT_NAME} "0.1")

# Modify the below lines to enable/disable output over UART/USB.
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

# Add the necessary libraries to the target.
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    pico_stdlib
    pico_cyw43_arch_none
    pico_rand
    hardware_i2c
    hardware_pio
    hardware_pwm
    hardware_powman
    hardware_clocks
    pico_multicore
    BME68x_SensorAPI
    nrf24_driver
    tiny-ECDH-c
    tiny-aes
)

add_library(BME68x_SensorAPI STATIC
    ${CMAKE_CURRENT_LIST_DIR}/lib/BME68x_SensorAPI/bme68x.c
)

# Add the necessary include directories to the target.
target_include_directories(${PROJECT_NAME} 
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/lib/BME68x_SensorAPI/
    ${CMAKE_CURRENT_LIST_DIR}/lib/pico-nrf24-driver/lib/nrf24l01/
    ${CMAKE_CURRENT_LIST_DIR}/lib/tiny-ECDH-c/
    ${CMAKE_CURRENT_LIST_DIR}/lib/tiny-AES-c/
    ${CMAKE_CURRENT_LIST_DIR}/lib/pico-ssd1306/
)

# Get compile definitions from environmental variables 
# and add them to the target.
target_compile_definitions(${PROJECT_NAME} PRIVATE
    STATION_ID=\"$ENV{STATION_ID}\"
)

# Add subdirectories to the build
add_subdirectory(lib/pico-nrf24-driver/lib)
add_subdirectory(lib/tiny-ECDH-c)
add_subdirectory(lib/tiny-AES-c)

pico_add_extra_outputs(${PROJECT_NAME})
