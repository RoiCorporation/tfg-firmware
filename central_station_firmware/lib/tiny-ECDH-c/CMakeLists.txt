cmake_minimum_required(VERSION 3.12)

project(tiny-ECDH-c C)

add_library(${PROJECT_NAME} "")
target_sources(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ecdh.c
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/ecdh.h
)

target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
)
