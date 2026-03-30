# Add library cpp files

set(NAME mongoose)

if (NOT DEFINED MONGOOSE_PATH)
    set(MONGOOSE_PATH "${CMAKE_CURRENT_LIST_DIR}/mongoose")
endif()
if (NOT DEFINED MONGOOSE_PORT)
    set(MONGOOSE_PORT "${CMAKE_CURRENT_LIST_DIR}/port/mongoose")
endif()

message("Using MONGOOSE from ${MONGOOSE_PATH} with Port folder ${MONGOOSE_PORT}")

#file(GLOB_RECURSE SOURCES ${MONGOOSE_PATH}/src/*.c)

add_library(${NAME} STATIC)

target_sources(${NAME} PRIVATE
#	${SOURCES}
	${MONGOOSE_PATH}/mongoose.c
)

# Add include directory
target_include_directories(${NAME} PUBLIC 
    ${MONGOOSE_PATH}
    ${MONGOOSE_PORT}
	${PROJECT_SOURCE_DIR}/include/
)

target_link_libraries(${NAME} PUBLIC 
	pico_stdlib
	hardware_pio 
	hardware_dma 
	pico_rand 
	pico_cyw43_driver 
	pico_cyw43_arch_none
    pico_mbedtls
)

target_compile_definitions(${NAME} PUBLIC
    WIZARD_WIFI_NAME=\"$ENV{WIFI_SSID}\"
    WIZARD_WIFI_PASS=\"$ENV{WIFI_PASSWORD}\"
)
