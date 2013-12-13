cmake_minimum_required(VERSION 2.6)

project(rc3200)

set(PACKAGE_BUGREPORT "luis.panader@gmail.com")
set(PACKAGE_NAME "rc3200_vm")
set(PACKAGE_VERSION "0.1")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE_NAME}")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message( "Release mode")
    set (LOG_ENABLE 0)
else (CMAKE_BUILD_TYPE STREQUAL "Release")
    set (LOG_ENABLE 1)
endif (CMAKE_BUILD_TYPE STREQUAL "Release")

# Set the directory where to find FindSLD2.cmake
set(CMAKE_MODULE_PATH ${rc3200_SOURCE_DIR}/cmake)

# Outpit the compiler ID.
message("Compiler ID: " ${CMAKE_CXX_COMPILER_ID})

# Include dirs
set(rc3200_INCLUDE_DIRS
    ${PROJECT_BINARY_DIR}
    ${rc3200_SOURCE_DIR}/include
)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (EMSCRIPTEN)
  # We are compiling with emscripten
  
  set(CMAKE_CXX_FLAGS "-std=c++11 ${MAKE_CXX_FLAGS}")
  set(CMAKE_EXECUTABLE_SUFFIX ".js")

  message( "Procesing Source Code - Build Library JS Wrapper")
  add_subdirectory(emscripten)

else (EMSCRIPTEN)

  include(Platform)

  message( "Procesing Source Code - Build library")
  add_subdirectory(src)

  if(BUILD_tools OR BUILD_tests)

    # Find SDL2 and OpenGL libs
    find_package(SDL2)
    if (NOT SDL2_FOUND)
        message("SDL2 not found! main executable will not display screen and virtual keyboard")
        set (SDL2_ENABLE 0)
    else (NOT SDL2_FOUND)
        set (SDL2_ENABLE 1)
    endif (NOT SDL2_FOUND)

    find_package(OpenGL)
    if (NOT OPENGL_FOUND)
        message("OpenGL not found! main executable will not display screen and virtual keyboard")
        set (SDL2_ENABLE 0)
    endif (NOT OPENGL_FOUND)

    find_package(GLEW)
    if(NOT GLEW_FOUND)
      message("GLEW not found! main executable will not display screen and virtual keyboard")
      set (SDL2_ENABLE 0)
    endif(NOT GLEW_FOUND)

    #find_package(GLM)
    #if(NOT GLM_FOUND)
    #  message("GLM not found! main executable will not display screen and virtual keyboard")
    #  set (SDL2_ENABLE 0)
    #endif(NOT GLM_FOUND)

    if(BUILD_tools)
      message( "Procesing Tools")
      add_subdirectory(tools)
    endif(BUILD_tools)

    message( "Procesing Assets")
    add_subdirectory(assets)


    if(BUILD_tests)
      message( "Procesing Tests")
      add_subdirectory(tests)
    endif(BUILD_tests)

  endif(BUILD_tools OR BUILD_tests)

endif (EMSCRIPTEN)
