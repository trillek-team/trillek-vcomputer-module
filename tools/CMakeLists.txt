# Main executable

# Main executable config file
configure_file (./config_main.hpp.in
  "${PROJECT_BINARY_DIR}/config_main.hpp" )

add_executable( vm
  ./main.cpp
  )

IF (SDL2_ENABLE EQUAL 0)
	include_directories( vm
		${TR3200_INCLUDE_DIRS}
		)

	target_link_libraries( vm
		TR3200
		)

ELSE (SDL2_ENABLE EQUAL 0)
	include_directories( vm
		${TR3200_INCLUDE_DIRS}
		${OPENGL_INCLUDE_DIR}
		${GLEW_INCLUDE_DIR}
		${GLM_INCLUDE_DIR}
		${SDL2_INCLUDE_DIR}
		)

	target_link_libraries( vm
		TR3200
		${OPENGL_LIBRARIES}
		${GLEW_LIBRARIES}
		${GLM_LIBRARIES}
		${SDL2_LIBRARY}
		)
ENDIF (SDL2_ENABLE EQUAL 0)

#pbm2font tool
add_executable( pbm2font
  ./pbm2font.cpp
  )

include_directories( pbm2font
  ${TR3200_INCLUDE_DIRS}
  )
