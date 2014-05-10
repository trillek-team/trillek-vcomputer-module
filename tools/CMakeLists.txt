# Main executable

# Main executable config file
configure_file (./config_main.hpp.in
  "${PROJECT_BINARY_DIR}/config_main.hpp" )

# Setup include and libs to be used in main/vm
set(VM_INCLUDE_DIRS
  ${VCOMPUTER_INCLUDE_DIRS}
  "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  )

set(VM_LINK_LIBS
  VCOMPUTER
  )

# If we have OpenGL / GLFW3 / GLM / GLEW libs
IF (GLFW3_ENABLE EQUAL 1)
  MESSAGE("vm executable have Virtual Screen enabled")

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
  MESSAGE("vm executable have Sound enabled")

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
