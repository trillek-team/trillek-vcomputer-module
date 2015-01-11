cmake_minimum_required(VERSION 2.6)

PROJECT(VCOMPUTER)
# Set the directory where to find cmake "modules"
SET (CMAKE_MODULE_PATH ${VCOMPUTER_SOURCE_DIR}/cmake)
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

INCLUDE(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

# Project generic variables
SET(VCOMP_VERSION_MAJOR 0)
SET(VCOMP_VERSION_MINOR 5)
SET(VCOMP_VERSION_PATCH 0)
SET(VCOMP_VERSION_STRING ${VCOMP_VERSION_MAJOR}.${VCOMP_VERSION_MINOR}.${VCOMP_VERSION_PATCH})

SET(VCOMP_BUILD "${GIT_REFSPEC}-${GIT_SHA1}") # Try to use git files to get refspec and hash

MESSAGE(STATUS "Trillek Virtual Computer version "
    ${VCOMP_VERSION_STRING} " Build " ${VCOMP_BUILD})

# Debug build ?
IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    MESSAGE("Debug build")
    SET(DEBUG_BUILD 1)

ELSE (CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(DEBUG_BUILD 0)

ENDIF (CMAKE_BUILD_TYPE STREQUAL "Debug")

# Options for optional compiling stuff
SET(BUILD_TOOLS_VCOMPUTER TRUE CACHE BOOL "Build Trillek VCOMPUTER tools")
SET(BUILD_TESTS_VCOMPUTER TRUE CACHE BOOL "Build Trillek VCOMPUTER tests")

# Optiones that affect functionality
set(BRKPOINTS_ENABLED 1 CACHE INT "Enables Break Points functionality")

IF (BRKPOINTS_ENABLED)
    message("Breakpoints functionality enabled")
ENDIF (BRKPOINTS_ENABLED)


# Include dirs
SET(VCOMPUTER_INCLUDE_DIRS
    ${PROJECT_BINARY_DIR}
    ${VCOMPUTER_SOURCE_DIR}/include
    ${VCOMPUTER_SOURCE_DIR}/src
    )

# Source dirs
FILE(GLOB VCOMPUTER_SRC
    "src/*.cpp"
    "src/tr3200/*.cpp"
    "src/dcpu16n/*.cpp"
    "src/devices/*.cpp"
    "src/*.hpp"
    "src/tr3200/*.hpp"
    "src/dcpu16n/*.hpp"
    "src/devices/*.hpp"
    "src/*.inc"
    "src/tr3200/*.inc"
    "src/dcpu16n/*.inc"
    "src/devices/*.inc"
    "include/*.hpp"
    "include/tr3200/*.hpp"
    "include/dcpu16n/*.hpp"
    "include/devices/*.hpp"
    )

# Apply all configuration stuff to config.hpp.in
CONFIGURE_FILE (src/config.hpp.in
    "${PROJECT_BINARY_DIR}/config.hpp" )
# Apply all configuration stuff to vc_dll.hpp.in
CONFIGURE_FILE (include/vc_dll.hpp.in
    "${VCOMPUTER_SOURCE_DIR}/include/vc_dll.hpp" )


INCLUDE(Platform)

MESSAGE(STATUS "Procesing Source Code - Build library")
# VCOMPUTER VM core lib
ADD_LIBRARY( VCOMPUTER_STATIC STATIC
    ${VCOMPUTER_SRC}
    )

INCLUDE_DIRECTORIES(VCOMPUTER_STATIC
    ${VCOMPUTER_INCLUDE_DIRS}
    )

ADD_LIBRARY( VCOMPUTER SHARED
    ${VCOMPUTER_SRC}
    )
# Enforces to use dllexport on shared library on Windows
IF(WIN32)
    MESSAGE("Advice. If you try to link against dinamic library on windows, don't forgot to add \"BUILD_DLL_VCOMPUTER\" preprocesor declaration to your proyect")
    SET_TARGET_PROPERTIES (VCOMPUTER PROPERTIES COMPILE_DEFINITIONS
        "DLL_EXPORT;BUILD_DLL_VCOMPUTER"
        )
ENDIF(WIN32)

INCLUDE_DIRECTORIES(VCOMPUTER
    ${VCOMPUTER_INCLUDE_DIRS}
    )

# Version of the libs
SET_TARGET_PROPERTIES(VCOMPUTER PROPERTIES
    VERSION ${VCOMP_VERSION_STRING}
    SOVERSION ${VCOMP_VERSION_MAJOR}
    )
SET_TARGET_PROPERTIES(VCOMPUTER_STATIC PROPERTIES
    VERSION ${VCOMP_VERSION_STRING}
    SOVERSION ${VCOMP_VERSION_MAJOR}
    )

IF(BUILD_TOOLS_VCOMPUTER)

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

endif(BUILD_TOOLS_VCOMPUTER)

if(BUILD_TESTS_VCOMPUTER)
    message(STATUS "Procesing Tests")
    enable_testing()

    find_package(GTest)
    if(NOT GTEST_FOUND AND DEFINED ENV{GTEST_ROOT})
        # findGTEST could fail if gtest is not compiled
        add_subdirectory($ENV{GTEST_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/gtest)
    endif(NOT GTEST_FOUND AND DEFINED ENV{GTEST_ROOT})

add_subdirectory(tests)

endif(BUILD_TESTS_VCOMPUTER)

# Install of library
INSTALL(CODE "MESSAGE(\"Installing library headers\")")
INSTALL(DIRECTORY include/
    DESTINATION include/tr-vcomputer
    PATTERN "*.in" EXCLUDE
    PATTERN "*.*~" EXCLUDE
    )

INSTALL(CODE "MESSAGE(\"Installing dinamic library\")")
INSTALL(TARGETS VCOMPUTER
    EXPORT VCOMPUTER
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib/static
    LIBRARY DESTINATION lib
    )

INSTALL(CODE "MESSAGE(\"Installing static library\")")
INSTALL(TARGETS VCOMPUTER_STATIC
    EXPORT VCOMPUTER
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib/static
    LIBRARY DESTINATION lib
    )




