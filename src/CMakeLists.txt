#include(GL)

set(rc3200_SRCS
    ./vcomputer.cpp
    ./cpu.cpp
    ./dis_rc3200.cpp
    ./CDA.cpp
)

# General Configuration file
#configure_file (./config.hpp.in
#                "${rc3200_SOURCE_DIR}/include/config.hpp" )
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


