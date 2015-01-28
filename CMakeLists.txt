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
IF (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    MESSAGE(STATUS "Debug build")
    ADD_DEFINITIONS(-DDEBUG)
ENDIF (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")

## Options !!!
SET(BUILD_STATIC_VCOMPUTER TRUE CACHE BOOL "Build Trillek VCOMPUTER library - static version")
SET(BUILD_DYNAMIC_VCOMPUTER TRUE CACHE BOOL "Build Trillek VCOMPUTER library - dynamic version")

# Options for optional compiling stuff
SET(BUILD_TOOLS_VCOMPUTER TRUE CACHE BOOL "Build Trillek VCOMPUTER tools")
SET(BUILD_TESTS_VCOMPUTER TRUE CACHE BOOL "Build Trillek VCOMPUTER tests")

# Optiones that affect functionality
SET(BRKPOINTS_ENABLED 1 CACHE INT "Enables Break Points functionality")

IF (BRKPOINTS_ENABLED)
    MESSAGE(STATUS "Breakpoints functionality enabled")
ENDIF (BRKPOINTS_ENABLED)

IF (NOT BUILD_STATIC_VCOMPUTER AND NOT BUILD_DYNAMIC_VCOMPUTER)
    IF(NOT WIN32)
        STRING(ASCII 27 Esc)
    ENDIF(NOT WIN32)
    MESSAGE(FATAL_ERROR "${Esc}[31mWTF! Choose Dynamic or Static or both!!!${Esc}[m")
ENDIF()

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
IF(BUILD_STATIC_VCOMPUTER)
    MESSAGE(STATUS "Building VComputer library - Static version")
    ADD_LIBRARY( VCOMPUTER_STATIC STATIC
        ${VCOMPUTER_SRC}
        )

    INCLUDE_DIRECTORIES(VCOMPUTER_STATIC
        ${VCOMPUTER_INCLUDE_DIRS}
        )
ENDIF(BUILD_STATIC_VCOMPUTER)

IF(BUILD_DYNAMIC_VCOMPUTER)
    MESSAGE(STATUS "Building VComputer library - Dynamic (DLL) version")
    ADD_LIBRARY( VCOMPUTER SHARED
        ${VCOMPUTER_SRC}
        )

    INCLUDE_DIRECTORIES(VCOMPUTER
        ${VCOMPUTER_INCLUDE_DIRS}
        )
ENDIF(BUILD_DYNAMIC_VCOMPUTER)

# Enforces to use dllexport on shared library on Windows
IF(WIN32)
    SET_TARGET_PROPERTIES (VCOMPUTER PROPERTIES COMPILE_DEFINITIONS
        "DLL_EXPORT;BUILD_DLL_VCOMPUTER"
        )
ENDIF(WIN32)

# Version of the libs
IF(BUILD_DYNAMIC_VCOMPUTER)
SET_TARGET_PROPERTIES(VCOMPUTER PROPERTIES
    VERSION ${VCOMP_VERSION_STRING}
    SOVERSION ${VCOMP_VERSION_MAJOR}
    )
ENDIF(BUILD_DYNAMIC_VCOMPUTER)

IF(BUILD_STATIC_VCOMPUTER)
    SET_TARGET_PROPERTIES(VCOMPUTER_STATIC PROPERTIES
        VERSION ${VCOMP_VERSION_STRING}
        SOVERSION ${VCOMP_VERSION_MAJOR}
        )
ENDIF(BUILD_STATIC_VCOMPUTER)

IF(BUILD_TOOLS_VCOMPUTER)
    MESSAGE(STATUS "Procesing Tools")
    ADD_SUBDIRECTORY(tools)

    MESSAGE(STATUS "Procesing Assets")
    ADD_SUBDIRECTORY(assets)

ENDIF(BUILD_TOOLS_VCOMPUTER)

IF(BUILD_TESTS_VCOMPUTER)
    MESSAGE(STATUS "Procesing Tests")
    ENABLE_TESTING()

    FIND_PACKAGE(GTest)
    IF(NOT GTEST_FOUND AND DEFINED ENV{GTEST_ROOT})
        # findGTEST could fail if gtest is not compiled
        ADD_SUBDIRECTORY($ENV{GTEST_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/gtest)
    ENDIF(NOT GTEST_FOUND AND DEFINED ENV{GTEST_ROOT})

    ADD_SUBDIRECTORY(tests)

ENDIF(BUILD_TESTS_VCOMPUTER)

# Install of library
INSTALL(CODE "MESSAGE(\"Installing library headers\")")
INSTALL(DIRECTORY include/
    DESTINATION include/tr-vcomputer
    PATTERN "*.in" EXCLUDE
    PATTERN "*.*~" EXCLUDE
    )

IF(BUILD_DYNAMIC_VCOMPUTER)
    INSTALL(CODE "MESSAGE(\"Installing dinamic library\")")
    INSTALL(TARGETS VCOMPUTER
        EXPORT VCOMPUTER
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib/static
        LIBRARY DESTINATION lib
        )
ENDIF(BUILD_DYNAMIC_VCOMPUTER)

IF(BUILD_STATIC_VCOMPUTER)
    INSTALL(CODE "MESSAGE(\"Installing static library\")")
    INSTALL(TARGETS VCOMPUTER_STATIC
        EXPORT VCOMPUTER
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib/static
        LIBRARY DESTINATION lib
        )
ENDIF(BUILD_STATIC_VCOMPUTER)

