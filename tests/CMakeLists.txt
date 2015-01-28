# Unit Tests
# idea take from : http://mifrosu.blogspot.com.es/2013/02/cmake-and-google-test-framework.html

FILE(GLOB unit_test_files_src
    "*_test.cpp"
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

IF(GTEST_FOUND)
    message(" ... using gtest found by findGTest")
    FIND_PACKAGE(Threads REQUIRED)

    set(test_EXECUTABLE unit_test)

    include_directories( unit_test
        ${VCOMPUTER_INCLUDE_DIRS}
        ${GTEST_INCLUDE_DIRS}
        )

    add_executable( unit_test
        ${unit_test_files_src}
        )

    target_link_libraries( unit_test
        ${VM_LINK_LIBS}
        ${GTEST_BOTH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

    add_test(unit_tests ../unit_test)

ELSEIF(DEFINED ENV{GTEST_ROOT})  # Note we omit the $ here!
    message(" ... using gtest found in $ENV{GTEST_ROOT}")
    FIND_PACKAGE(Threads REQUIRED)

    set(test_EXECUTABLE unit_test)

    include_directories( unit_test
        ${VCOMPUTER_INCLUDE_DIRS}
        $ENV{GTEST_ROOT}/include
        $ENV{GTEST_ROOT}
        )

    add_executable( unit_test
        ${unit_test_files_src}
        )


    target_link_libraries( unit_test
        ${VM_LINK_LIBS}
        gtest
        gtest_main
        ${CMAKE_THREAD_LIBS_INIT}
        )

    add_test(unit_tests ../unit_test)

ELSEIF(GTEST_ROOT)
    message(" ... using gtest in ${GTEST_ROOT}")
    FIND_PACKAGE(Threads REQUIRED)

    set(test_EXECUTABLE unit_test)

    include_directories( unit_test
        ${VCOMPUTER_INCLUDE_DIRS}
        ${GTEST_ROOT}/include
        ${GTEST_ROOT}
        )

    add_executable( unit_test
        ${unit_test_files_src}
        )


    target_link_libraries( unit_test
        ${VM_LINK_LIBS}
        gtest
        gtest_main
        ${CMAKE_THREAD_LIBS_INIT}
        )

    add_test(unit_tests ../unit_test)

ELSE()
    message(STATUS "findGTest failed and GTEST_ROOT is not defined. You must tell CMake where to find the gtest source. For example :
    GTEST_ROOT=path/gtest-1.6.0 ;
    export GTEST_ROOT ")
    set(GTEST_ROOT "" CACHE PATH "GTest root path")
endif()

# Benchmark executable
add_executable( benchmark
    benchmark.cpp
    )

include_directories( benchmark
    ${VCOMPUTER_INCLUDE_DIRS}
    )

target_link_libraries( benchmark
    ${VM_LINK_LIBS}
    )

