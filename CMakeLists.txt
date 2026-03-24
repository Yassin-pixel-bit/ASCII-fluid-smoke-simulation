cmake_minimum_required(VERSION 3.10)

project(ASCII_fluid)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(main ${SOURCES})

if(WIN32)
    target_link_libraries(main PRIVATE winmm)
endif()

target_include_directories(main PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/fluid
    ${CMAKE_SOURCE_DIR}/src/terminal
)

target_compile_options(main PRIVATE -O3)
target_link_options(main PRIVATE -static)

set_target_properties(main PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
)