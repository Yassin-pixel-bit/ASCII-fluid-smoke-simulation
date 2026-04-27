cmake_minimum_required(VERSION 3.10)

project(ASCII_fluid)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Requires C++20 or higher. Built and tested with C++23.
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
    target_compile_options(${PROJECT_NAME} PRIVATE /O2 /EHsc /fp:fast)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -ffast-math)
    target_link_options(${PROJECT_NAME} PRIVATE -static)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/bin"
)

# PYTHON UI PACKAGER (PyInstaller)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import PyInstaller"
    RESULT_VARIABLE PYINSTALLER_STATUS
    ERROR_QUIET
)

if(NOT PYINSTALLER_STATUS EQUAL 0)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        message(FATAL_ERROR 
            " PYINSTALLER NOT FOUND!\n"
            " The Python UI Packager requires PyInstaller and other dependencies.\n"
            " Please install 'pip' and then install the required packages:\n"
            "    1. sudo apt install python3-pip\n"
            "    2. pip3 install -r requirements.txt\n"
            "       (Add --break-system-packages if Ubuntu blocks it)\n"
        )
    else()
        message(FATAL_ERROR 
            " PYINSTALLER NOT FOUND!\n"
            " The Python UI Packager requires PyInstaller and other dependencies.\n"
            " Please install them by running the following command in your terminal:\n"
            "    ${Python3_EXECUTABLE} -m pip install -r requirements.txt\n\n"
        )
    endif()
endif()

add_custom_target(launcher ALL
    COMMAND Python3::Interpreter -m PyInstaller
        --clean
        --noconfirm
        --onefile
        --windowed
        --collect-all customtkinter
        --name launcher
        --distpath ${CMAKE_SOURCE_DIR}
        --workpath ${CMAKE_CURRENT_BINARY_DIR}/build
        --specpath ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/src/launcher/launcher_py/main.py
    COMMENT ">>> Packaging Python UI into a standalone executable..."
    VERBATIM
)

add_dependencies(launcher ${PROJECT_NAME})