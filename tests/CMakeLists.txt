# Unit Tests
 # idea take from : http://mifrosu.blogspot.com.es/2013/02/cmake-and-google-test-framework.html
if(DEFINED ENV{GTEST_ROOT})  # Note we omit the $ here!
  message(" ... using gtest found in $ENV{GTEST_ROOT}")

  set(test_EXECUTABLE unit_test)
  
  include_directories( unit_test
    ${VCOMPUTER_INCLUDE_DIRS}
    $ENV{GTEST_ROOT}/include
    $ENV{GTEST_ROOT}
    )

  add_executable( unit_test
    VComputer_test.cpp
    GKeyb_test.cpp
    )

  target_link_libraries( unit_test
    VCOMPUTER
    gtest
    gtest_main
    pthread
    )

ELSE()
  message("GTEST_ROOT is not defined. You must tell CMake where
  to find the gtest source. For example :
  GTEST_ROOT=path/gtest-1.6.0 ;
  export GTEST_ROOT ")
endif()

# Benchmark executable
add_executable( benchmark
  benchmark.cpp
  )

include_directories( benchmark
  ${VCOMPUTER_INCLUDE_DIRS}
  )

target_link_libraries( benchmark
  VCOMPUTER
  )

