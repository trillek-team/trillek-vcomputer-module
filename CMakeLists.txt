cmake_minimum_required(VERSION 2.6)

if (VCOMPUTER_SUB_PROJECT)
	message (STATUS "Setting SRC and INCLUDE for Trillek VCOMPUTER")

	# Include dirs
	set(VCOMPUTER_INCLUDE_DIRS
		"${CMAKE_CURRENT_SOURCE_DIR}/include/"
		PARENT_SCOPE
		)

	# Source dirs
	file(GLOB VCOMPUTER_SRC_
		"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/TR3200/*.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/src/devices/*.cpp"
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
    "src/TR3200/*.cpp"
		"src/devices/*.cpp"
		)


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

			if(BUILD_TOOLS_VCOMPUTER)
				message(STATUS "Procesing Tools")
				add_subdirectory(tools)
			endif(BUILD_TOOLS_VCOMPUTER)

			message(STATUS "Procesing Assets")
			add_subdirectory(assets)

		endif(BUILD_TOOLS_VCOMPUTER OR BUILD_TESTS_VCOMPUTER)

		if(BUILD_TESTS_VCOMPUTER)
			message(STATUS "Procesing Tests")

			if(DEFINED ENV{GTEST_ROOT})  # Note we omit the $ here!
			  add_subdirectory($ENV{GTEST_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/gtest)
			endif()

			add_subdirectory(tests)
		endif(BUILD_TESTS_VCOMPUTER)

	endif (EMSCRIPTEN)

endif (VCOMPUTER_SUB_PROJECT)

