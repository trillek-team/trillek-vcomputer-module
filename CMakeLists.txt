cmake_minimum_required(VERSION 2.6)


if (RC3200_SUB_PROJECT)
  message ("Setting SRC and INCLUDE for RC3200")

  # Include dirs
  set(RC3200_INCLUDE_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    PARENT_SCOPE
    )

  # Source dirs
  file(GLOB RC3200_SRC_ 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    )

  set(RC3200_SRC
    ${RC3200_SRC_}
    PARENT_SCOPE
    )

else (RC3200_SUB_PROJECT)

  project(RC3200)

  set(PACKAGE_BUGREPORT "luis.panader@gmail.com")
  set(PACKAGE_NAME "RC3200_vm")
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
  set(CMAKE_MODULE_PATH ${RC3200_SOURCE_DIR}/cmake)

  # Outpit the compiler ID.
  message("Compiler ID: " ${CMAKE_CXX_COMPILER_ID})

  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

  # Include dirs
  set(RC3200_INCLUDE_DIRS
    ${PROJECT_BINARY_DIR}
    ${RC3200_SOURCE_DIR}/include
    )

  # Source dirs
  file(GLOB RC3200_SRC 
    "src/*.cpp" 
    "include/*.hpp" 
    )


  if (EMSCRIPTEN)
    # We are compiling with emscripten

    set(CMAKE_CXX_FLAGS "-std=c++11 ${MAKE_CXX_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX ".js")

    message( "Procesing Source Code - Build Library JS Wrapper")
    add_subdirectory(emscripten)

  else (EMSCRIPTEN)

    include(Platform)

    message( "Procesing Source Code - Build libraries")
    # RC3200 VM core lib
    add_library( RC3200_lib
      SHARED
      ${RC3200_SRC}
      )

    add_library( RC3200_lib_static
      STATIC
      ${RC3200_SRC}
      )

    include_directories(RC3200_lib
      ${RC3200_INCLUDE_DIRS}
      )

    include_directories(RC3200_lib_static
      ${RC3200_INCLUDE_DIRS}
      )

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

endif (RC3200_SUB_PROJECT)

