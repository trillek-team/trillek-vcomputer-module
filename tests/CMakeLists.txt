# idea take from : http://mifrosu.blogspot.com.es/2013/02/cmake-and-google-test-framework.html

set(test_EXECUTABLE module_test)

include_directories( module_test
  ${VCOMPUTER_INCLUDE_DIRS}
	$ENV{GTEST_ROOT}/include
	$ENV{GTEST_ROOT}
  )

add_executable( module_test
	VComputer_test.cpp
  )

target_link_libraries( module_test
  VCOMPUTER
  gtest
	gtest_main
	pthread
	)

# Benchmark executable
#add_executable( benchmark
#  benchmark.cpp
#  )
#
#include_directories( benchmark
#  ${VCOMPUTER_INCLUDE_DIRS}
#  )
#
#target_link_libraries( benchmark
#  VCOMPUTER
#  )
#
#
#if(SDL2_ENABLE)
#
#  # Test CDA vram to texture and dispaly in OpenGL 
#  add_executable( test_cda
#    test_cda.cpp
#    )
#
#  include_directories( test_cda
#    ${VCOMPUTER_INCLUDE_DIRS}
#    ${OPENGL_INCLUDE_DIR}
#    ${GLEW_INCLUDE_DIR}
#    ${GLM_INCLUDE_DIR}
#    ${SDL2_INCLUDE_DIR}
#    )
#
#  target_link_libraries( test_cda
#    VCOMPUTER
#    ${OPENGL_LIBRARIES}
#    ${GLEW_LIBRARIES}
#    ${GLM_LIBRARIES}
#    ${SDL2_LIBRARY}
#    )
#
#endif(SDL2_ENABLE)
