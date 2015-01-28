# Options
set(BUILD_TOOLS_SCREEN TRUE CACHE BOOL "Build Trillek VCOMPUTER tools with virtual screen")
set(BUILD_TOOLS_AUDIO  TRUE CACHE BOOL "Build Trillek VCOMPUTER tools with audio")

# Find GLFW3 and OpenGL libs
find_package(GLFW3)
if (NOT GLFW3_FOUND)
    message(WARNING "GLFW3 not found! toy emulator will not display screen and virtual keyboard")
    set (GLFW3_ENABLE 0)
else (NOT GLFW3_FOUND)
    set (GLFW3_ENABLE 1)
endif (NOT GLFW3_FOUND)

find_package(OpenGL)
if (NOT OPENGL_FOUND)
    message(WARNING "OpenGL not found! toy emulator will not display screen and virtual keyboard")
    set (GLFW3_ENABLE 0)
endif (NOT OPENGL_FOUND)

find_package(GLEW)
if(NOT GLEW_FOUND)
    message(WARNING "GLEW not found! toy emulator  will not display screen and virtual keyboard")
    set (GLFW3_ENABLE 0)
endif(NOT GLEW_FOUND)

find_package(GLM)
if(NOT GLM_FOUND)
    message(WARNING "GLM not found! toy emulator will not display screen and virtual keyboard")
    set (GLFW3_ENABLE 0)
endif(NOT GLM_FOUND)

find_package(OpenAL)
if(NOT OPENAL_FOUND)
    message(WARNING "OpenAL not found! toy emulator will not be hable to play the beeper")
    set (OPENAL_ENABLE 0)
else (NOT OPENAL_FOUND)

    find_package(Alure)
    if(NOT ALURE_FOUND)
        message(WARNING "ALure not found! toy emulator will not be hable to play the beeper")
        set (OPENAL_ENABLE 0)
    else(NOT ALURE_FOUND)
        set (OPENAL_ENABLE 1)
    endif(NOT ALURE_FOUND)

endif(NOT OPENAL_FOUND)


IF (NOT BUILD_TOOLS_SCREEN)
    SET(GLFW3_ENABLE 0)
ENDIF (NOT BUILD_TOOLS_SCREEN)

IF (NOT BUILD_TOOLS_AUDIO)
    SET(OPENAL_ENABLE 0)
ENDIF (NOT BUILD_TOOLS_AUDIO)
# Main executable

# Main executable config file
configure_file (./config_main.hpp.in
  "${PROJECT_BINARY_DIR}/config_main.hpp" )

# Setup include and libs to be used in main/vm
set(VM_INCLUDE_DIRS
  ${VCOMPUTER_INCLUDE_DIRS}
  "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  )


# Links agains the static version if is enabled
IF(BUILD_STATIC_VCOMPUTER)
    SET(VM_LINK_LIBS
      VCOMPUTER_STATIC
      )
ELSEIF(BUILD_DYNAMIC_VCOMPUTER)
    SET(VM_LINK_LIBS
      VCOMPUTER
      )
ENDIF(BUILD_STATIC_VCOMPUTER)

# If we have OpenGL / GLFW3 / GLM / GLEW libs
IF (GLFW3_ENABLE EQUAL 1)
    MESSAGE(STATUS "vm executable have Virtual Screen enabled")

  # Some auxiliar libs
  file(GLOB TOOL_OTHER_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/devices/*.hpp"
    )

  set(VM_INCLUDE_DIRS
    ${VM_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${GLFW3_INCLUDE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    )

  set(VM_LINK_LIBS
    ${VM_LINK_LIBS}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLM_LIBRARIES}
    ${GLFW3_LIBRARY}
    )

  # TDA viewer can only be build with OpenGL stuff
  add_executable( tda_view
    ./tda_view.cpp
    )

  include_directories( tda_view
    ${VM_INCLUDE_DIRS}
    )

  target_link_libraries( tda_view
    ${VM_LINK_LIBS}
    )

ENDIF (GLFW3_ENABLE EQUAL 1)

# If we have OpenAL
IF (OPENAL_ENABLE EQUAL 1)
    MESSAGE(STATUS "vm executable have Sound enabled")

  set(VM_INCLUDE_DIRS
    ${VM_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    ${ALURE_INCLUDE_DIR}
    )

  set(VM_LINK_LIBS
    ${VM_LINK_LIBS}
    ${OPENAL_LIBRARY}
    ${ALURE_LIBRARY}
    )

ENDIF (OPENAL_ENABLE EQUAL 1)

# Main executable
add_executable( vc
  ./main.cpp
  ${TOOL_OTHER_SRC}
  )

include_directories( vc
  ${VM_INCLUDE_DIRS}
  )

target_link_libraries( vc
  ${VM_LINK_LIBS}
  )

#pbm2font tool
add_executable( pbm2font
  ./pbm2font.cpp
  )

include_directories( pbm2font
  ${VCOMPUTER_INCLUDE_DIRS}
  )

# makedisk executable
add_executable( makedisk
  ./makedisk.cpp
  ${TOOL_OTHER_SRC}
  )

include_directories( makedisk
  ${VM_INCLUDE_DIRS}
  )

target_link_libraries( makedisk
  ${VM_LINK_LIBS}
  )
