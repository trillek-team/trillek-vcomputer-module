# Main executable

# Main executable config file
configure_file (./config_main.hpp.in
  "${PROJECT_BINARY_DIR}/config_main.hpp" )

add_executable( vm
  ./main.cpp
  )

IF (GLFW3_ENABLE EQUAL 0)
	include_directories( vm
		${VCOMPUTER_INCLUDE_DIRS}
		)

	target_link_libraries( vm
		VCOMPUTER
		)

ELSE (GLFW3_ENABLE EQUAL 0)
	include_directories( vm
		${VCOMPUTER_INCLUDE_DIRS}
		${OPENGL_INCLUDE_DIR}
		${GLEW_INCLUDE_DIR}
		${GLM_INCLUDE_DIR}
		${GLFW3_INCLUDE_DIR}
		)

	target_link_libraries( vm
		VCOMPUTER
		${OPENGL_LIBRARIES}
		${GLEW_LIBRARIES}
		${GLM_LIBRARIES}
		${GLFW3_LIBRARY}
		)

  add_executable( tda_view
    ./tda_view.cpp
    )
	
  include_directories( tda_view
		${VCOMPUTER_INCLUDE_DIRS}
		${OPENGL_INCLUDE_DIR}
		${GLEW_INCLUDE_DIR}
		${GLM_INCLUDE_DIR}
		${GLFW3_INCLUDE_DIR}
		)

	target_link_libraries( tda_view
		VCOMPUTER
		${OPENGL_LIBRARIES}
		${GLEW_LIBRARIES}
		${GLM_LIBRARIES}
		${GLFW3_LIBRARY}
		)


ENDIF (GLFW3_ENABLE EQUAL 0)

#pbm2font tool
add_executable( pbm2font
  ./pbm2font.cpp
  )

include_directories( pbm2font
  ${VCOMPUTER_INCLUDE_DIRS}
  )
