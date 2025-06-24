cmake_minimum_required(VERSION 3.25)
project(Gigno LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file (GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_link_options(--static -static-libgcc -static-libstdc++) # Required for program to work outside of IDE.

add_executable(giap ${SOURCES})

target_include_directories(giap PUBLIC
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/src )
