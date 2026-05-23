# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_NAME central_station_firmware)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion GCC_ARM_RPI_2_2_0_2)
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
    src/central_station_firmware.c 
    src/functions.c
    src/callbacks.c
    src/hazards.c
    src/alerts.c
    src/bme680_port.c
    src/network.c
    src/utils.c
    src/oled_display.c
    lib/pico-ssd1306/ssd1306.c
    src/monocypher.c
    src/iaq.c
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
    hardware_i2c
    hardware_pio
    hardware_pwm
    pico_rand
    pico_bootrom
    pico_mbedtls
    hardware_flash
    BME68x_SensorAPI
    nrf24_driver
    tiny-ECDH-c
    tiny-aes
    mongoose
)

add_library(BME68x_SensorAPI STATIC
    ${CMAKE_CURRENT_LIST_DIR}/lib/BME68x_SensorAPI/bme68x.c
)

# Add the necessary include directories to the target.
target_include_directories(${PROJECT_NAME} 
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include/
    ${CMAKE_CURRENT_LIST_DIR}/lib/BME68x_SensorAPI/
    ${CMAKE_CURRENT_LIST_DIR}/lib/pico-nrf24-driver/lib/nrf24l01/
    ${CMAKE_CURRENT_LIST_DIR}/lib/tiny-ECDH-c/
    ${CMAKE_CURRENT_LIST_DIR}/lib/tiny-AES-c/
    ${CMAKE_CURRENT_LIST_DIR}/lib/mongoose/
    ${CMAKE_CURRENT_LIST_DIR}/lib/pico-ssd1306/
)

# Get compile definitions from environmental variables 
# and add them to the target.
target_compile_definitions(${PROJECT_NAME} PRIVATE
    STATION_ID=\"$ENV{STATION_ID}\"
    WIFI_SSID=\"$ENV{WIFI_SSID}\"
    WIFI_PASSWORD=\"$ENV{WIFI_PASSWORD}\"
    CYW43_HOST_NAME="Pico2W-Web"
    MQTT_HOST=\"$ENV{MQTT_HOST}\"
    MQTT_PORT=\"$ENV{MQTT_PORT}\"
    MQTT_TOPIC=\"$ENV{MQTT_TOPIC}\"
    TB_CLIENT_ID=\"$ENV{TB_CLIENT_ID}\"
    TB_USERNAME=\"$ENV{TB_USERNAME}\"
    TB_PASSWORD=\"$ENV{TB_PASSWORD}\"
    TB_ACCESS_TOKEN=\"$ENV{TB_ACCESS_TOKEN}\"
)

# Add subdirectories to the build
add_subdirectory(lib/pico-nrf24-driver/lib)
add_subdirectory(lib/tiny-ECDH-c)
add_subdirectory(lib/tiny-AES-c)

pico_add_extra_outputs(${PROJECT_NAME})

# ====================================================================================
# Mongoose configuration
SET(MONGOOSE_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/mongoose" CACHE STRING "Common Lib")
include(mongoose.cmake)

#Set up files for the release packages
install(CODE "execute_process(COMMAND $ENV{HOME}/bin/picoDeploy.sh ${CMAKE_CURRENT_BINARY_DIR}/src/${NAME}.elf)")

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/src/${NAME}.uf2
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
