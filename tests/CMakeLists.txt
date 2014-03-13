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

