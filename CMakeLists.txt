cmake_minimum_required(VERSION 3.10)

project(ASCII_fluid)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(FMT_HEADER_ONLY)

# MAIN ENGINE (ASCII_fluid)

file(GLOB_RECURSE ENGINE_SOURCES 
    "src/core/*.cpp" 
    "src/fluid/*.cpp" 
    "src/terminal/*.cpp" 
    "src/render/*.cpp" 
)
list(APPEND ENGINE_SOURCES "src/main.cpp")

add_executable(${PROJECT_NAME} ${ENGINE_SOURCES})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/fluid
    ${CMAKE_SOURCE_DIR}/src/terminal
    ${CMAKE_SOURCE_DIR}/src/render
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /O2 /EHsc)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)
    target_link_options(${PROJECT_NAME} PRIVATE -static)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
)

# BOOTSTRAPPER (launcher)

find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

add_executable(launcher src/launcher/launcher.cpp)
target_link_libraries(launcher PRIVATE Python3::Python)

add_dependencies(launcher ${PROJECT_NAME})

if(MSVC)
    set_target_properties(launcher PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

set_target_properties(launcher PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
)

# POST-BUILD AUTOMATION (Direct to Root)

add_custom_command(TARGET launcher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            ${CMAKE_SOURCE_DIR}/src/launcher/launcher_py 
            ${CMAKE_SOURCE_DIR}/launcher_py
    COMMENT ">>> Copying Python UI Source to Root Directory..."
)

if(WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/vendor/python_embed")
    add_custom_command(TARGET launcher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
                ${CMAKE_SOURCE_DIR}/vendor/python_embed 
                ${CMAKE_SOURCE_DIR}/
        COMMENT ">>> Copying Windows portable Python environment to Root Directory..."
    )
elseif(WIN32)
    add_custom_command(TARGET launcher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo ">>> Skipping Python DLL copy: 'vendor/python_embed' folder not found."
    )
endif()