#include(GL)

set(rc3200_SRCS
    ./vcomputer.cpp
    ./cpu.cpp
    ./dis_rc3200.cpp
    ./CDA.cpp
)

# General Configuration file
configure_file (./config.hpp.in
                "${rc3200_SOURCE_DIR}/include/config.hpp" )
#                "${PROJECT_BINARY_DIR}/config.hpp" )

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


# Main executable

# Main executable config file
configure_file (./config_main.hpp.in
                "${PROJECT_BINARY_DIR}/config_main.hpp" )

add_executable( vm
    main.cpp
)

include_directories( vm
    ${rc3200_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
)

target_link_libraries( vm
    rc3200_lib_static
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLM_LIBRARIES}
    ${SDL2_LIBRARY}
)


