cmake_minimum_required(VERSION 2.6)

project(VCOMPUTER)

# Project generic variables
set(VCOMP_VERSION_MAJOR 0)
set(VCOMP_VERSION_MINOR 2)

# Debug build ?
IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Debug build")
    set(DEBUG_BUILD 1)

else (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEBUG_BUILD 0)

endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# Options for optional compiling stuff
set(BUILD_TOOLS_VCOMPUTER TRUE CACHE BOOL "Build Trillek VCOMPUTER tools")
set(BUILD_TESTS_VCOMPUTER TRUE CACHE BOOL "Build Trillek VCOMPUTER tests")
set(BUILD_STATIC_VCOMPUTER FALSE CACHE BOOL "Build Trillek VCOMPUTER as a static library")

# Optiones that affect functionality
set(BRKPOINTS_ENABLED 1 CACHE INT "Enables Break Points functionality")

IF (BRKPOINTS_ENABLED)
    message("Breakpoints functionality enabled")
ENDIF (BRKPOINTS_ENABLED)

# Set the directory where to find cmake "modules"
set(CMAKE_MODULE_PATH ${VCOMPUTER_SOURCE_DIR}/cmake)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Include dirs
set(VCOMPUTER_INCLUDE_DIRS
    ${PROJECT_BINARY_DIR}
    ${VCOMPUTER_SOURCE_DIR}/include
    ${VCOMPUTER_SOURCE_DIR}/include/TR3200
    ${VCOMPUTER_SOURCE_DIR}/include/DCPU16N
    ${VCOMPUTER_SOURCE_DIR}/include/devices
    ${VCOMPUTER_SOURCE_DIR}/src
    ${VCOMPUTER_SOURCE_DIR}/src/TR3200
    ${VCOMPUTER_SOURCE_DIR}/src/DCPU16N
    ${VCOMPUTER_SOURCE_DIR}/src/devices
    )

# Source dirs
file(GLOB VCOMPUTER_SRC
    "src/*.cpp"
    "src/TR3200/*.cpp"
    "src/DCPU16N/*.cpp"
    "src/devices/*.cpp"
    "src/*.hpp"
    "src/TR3200/*.hpp"
    "src/DCPU16N/*.hpp"
    "src/devices/*.hpp"
    "src/*.inc"
    "src/TR3200/*.inc"
    "src/DCPU16N/*.inc"
    "src/devices/*.inc"
    "include/*.hpp"
    "include/TR3200/*.hpp"
    "include/DCPU16N/*.hpp"
    "include/devices/*.hpp"
    )

# Apply all configuration stuff to Config.hpp.in
configure_file (src/Config.hpp.in
    "${PROJECT_BINARY_DIR}/Config.hpp" )

if (EMSCRIPTEN)
    # We are compiling with emscripten

    set(CMAKE_CXX_FLAGS "-std=c++11 ${MAKE_CXX_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX ".js")

    message(STATUS "Procesing Source Code - Build Library JS Wrapper")
    add_subdirectory(emscripten)

else (EMSCRIPTEN)

    include(Platform)

    message(STATUS "Procesing Source Code - Build library")
    # VCOMPUTER VM core lib
    if(BUILD_STATIC_VCOMPUTER)
        add_library( VCOMPUTER STATIC
            ${VCOMPUTER_SRC}
            )

        include_directories(VCOMPUTER
            ${VCOMPUTER_INCLUDE_DIRS}
            )

    else(BUILD_STATIC_VCOMPUTER)
        add_library( VCOMPUTER SHARED
            ${VCOMPUTER_SRC}
            )

        include_directories(VCOMPUTER
            ${VCOMPUTER_INCLUDE_DIRS}
            )

    endif(BUILD_STATIC_VCOMPUTER)

    if(BUILD_TOOLS_VCOMPUTER OR BUILD_TESTS_VCOMPUTER)

        # Find GLFW3 and OpenGL libs
        find_package(GLFW3)
        if (NOT GLFW3_FOUND)
            message("GLFW3 not found! main executable will not display screen and virtual keyboard")
            set (GLFW3_ENABLE 0)
        else (NOT GLFW3_FOUND)
            set (GLFW3_ENABLE 1)
        endif (NOT GLFW3_FOUND)

        find_package(OpenGL)
        if (NOT OPENGL_FOUND)
            message("OpenGL not found! main executable will not display screen and virtual keyboard")
            set (GLFW3_ENABLE 0)
        endif (NOT OPENGL_FOUND)

        find_package(GLEW)
        if(NOT GLEW_FOUND)
            message("GLEW not found! main executable will not display screen and virtual keyboard")
            set (GLFW3_ENABLE 0)
        endif(NOT GLEW_FOUND)

        find_package(GLM)
        if(NOT GLM_FOUND)
            message("GLM not found! main executable will not display screen and virtual keyboard")
            set (GLFW3_ENABLE 0)
        endif(NOT GLM_FOUND)

        find_package(OpenAL)
        if(NOT OPENAL_FOUND)
            message("OpenAL not found! main executable will not be hable to play the beeper")
            set (OPENAL_ENABLE 0)
        else (NOT OPENAL_FOUND)

            find_package(Alure)
            if(NOT ALURE_FOUND)
                message("ALure not found! main executable will not be hable to play the beeper")
                set (OPENAL_ENABLE 0)
            else(NOT ALURE_FOUND)
                set (OPENAL_ENABLE 1)
            endif(NOT ALURE_FOUND)

        endif(NOT OPENAL_FOUND)



        if(BUILD_TOOLS_VCOMPUTER)
            message(STATUS "Procesing Tools")
            add_subdirectory(tools)
        endif(BUILD_TOOLS_VCOMPUTER)

        message(STATUS "Procesing Assets")
        add_subdirectory(assets)

    endif(BUILD_TOOLS_VCOMPUTER OR BUILD_TESTS_VCOMPUTER)

    if(BUILD_TESTS_VCOMPUTER)
        message(STATUS "Procesing Tests")
        enable_testing()

        find_package(GTest)
        if(NOT $GTEST_FOUND AND DEFINED ENV{GTEST_ROOT})
            # findGTEST could fail if gtest is not compiled
            add_subdirectory($ENV{GTEST_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/gtest)
        endif(NOT $GTEST_FOUND AND DEFINED ENV{GTEST_ROOT})

    add_subdirectory(tests)

    endif(BUILD_TESTS_VCOMPUTER)

endif (EMSCRIPTEN)


