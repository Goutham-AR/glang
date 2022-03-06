cmake_minimum_required(VERSION 3.20)

project(
    glang            
    DESCRIPTION "A Programming Language"
    LANGUAGES C CXX
    VERSION 0.1.0
)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS, On)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

# prevent in-source build
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR “In-source build detected!”)
endif()


add_subdirectory(src)