#include(GL)
#include(SDL2)

set(rc3200_SRCS
    ./vcomputer.cpp
    ./cpu.cpp
    ./dis_rc3200.cpp
    ./CDA.cpp
)


# RC3200 VM core lib
add_library( rc3200_lib
    SHARED
    ${rc3200_SRCS}
)

add_library( rc3200_lib_static
    STATIC
    ${rc3200_SRCS}
    )
    
include_directories(rc3200_lib
    ${rc3200_INCLUDE_DIRS}
)

include_directories(rc3200_lib_static
    ${rc3200_INCLUDE_DIRS}
)

    
# Main executable
add_executable( vm
    main.cpp
)


include_directories( vm
    ${rc3200_INCLUDE_DIRS}
)

target_link_libraries( vm
    rc3200_lib_static
)

# Benchmark executable
add_executable( benchmark
    benchmark.cpp
)

include_directories( benchmark
    ${rc3200_INCLUDE_DIRS}
)

target_link_libraries( benchmark
    rc3200_lib_static
)


