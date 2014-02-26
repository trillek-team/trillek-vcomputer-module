cmake_minimum_required(VERSION 2.6)

if (VCOMPUTER_SUB_PROJECT)
  message ("Setting SRC and INCLUDE for Trillek VCOMPUTER")

  # Include dirs
  set(VCOMPUTER_INCLUDE_DIRS 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    PARENT_SCOPE
    )

  # Source dirs
  file(GLOB VCOMPUTER_SRC_ 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    )

  set(VCOMPUTER_SRC
    ${VCOMPUTER_SRC_}
    PARENT_SCOPE
    )

else (VCOMPUTER_SUB_PROJECT)

  project(VCOMPUTER)

	# We like having options
	set(BUILD_TOOLS_VCOMPUTER TRUE CACHE BOOL "Build Trillek VCOMPUTER tools")
	set(BUILD_TESTS_VCOMPUTER TRUE CACHE BOOL "Build Trillek VCOMPUTER tests")
	set(BUILD_STATIC_VCOMPUTER FALSE CACHE BOOL "Build Trillek VCOMPUTER as a static library")

  # Set the directory where to find FindSLD2.cmake
  set(CMAKE_MODULE_PATH ${VCOMPUTER_SOURCE_DIR}/cmake)

  set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

  # Include dirs
  set(VCOMPUTER_INCLUDE_DIRS
    ${PROJECT_BINARY_DIR}
    ${VCOMPUTER_SOURCE_DIR}/include
    )

  # Source dirs
  file(GLOB VCOMPUTER_SRC 
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

    message( "Procesing Source Code - Build library")
		# VCOMPUTER VM core lib
		if(BUILD_STATIC_VCOMPUTER)
			add_library( VCOMPUTER	STATIC
				${VCOMPUTER_SRC}
				)
    
			include_directories(VCOMPUTER
				${VCOMPUTER_INCLUDE_DIRS}
				)

		else(BUILD_STATIC_VCOMPUTER)
			add_library( VCOMPUTER	SHARED
				${VCOMPUTER_SRC}
				)
    
			include_directories(VCOMPUTER
				${VCOMPUTER_INCLUDE_DIRS}
				)
		
		endif(BUILD_STATIC_VCOMPUTER)


    if(BUILD_TOOLS_VCOMPUTER OR BUILD_TESTS_VCOMPUTER)

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

      find_package(GLM)
      if(NOT GLM_FOUND)
        message("GLM not found! main executable will not display screen and virtual keyboard")
        set (SDL2_ENABLE 0)
      endif(NOT GLM_FOUND)

      if(BUILD_TOOLS_VCOMPUTER)
        message( "Procesing Tools")
        add_subdirectory(tools)
      endif(BUILD_TOOLS_VCOMPUTER)

      message( "Procesing Assets")
      add_subdirectory(assets)


      if(BUILD_TESTS_VCOMPUTER)
        message( "Procesing Tests")
        add_subdirectory(tests)
      endif(BUILD_TESTS_VCOMPUTER)

    endif(BUILD_TOOLS_VCOMPUTER OR BUILD_TESTS_VCOMPUTER)

  endif (EMSCRIPTEN)

endif (VCOMPUTER_SUB_PROJECT)

