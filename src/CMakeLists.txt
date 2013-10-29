#include(GL)
#include(SFML)
#include(SDL2)

set(rc3200_SRCS
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

    
# Main executable
add_executable(vm
    vm.cpp
)

include_directories(vm
    ${rc3200_INCLUDE_DIRS}
#    ${OPENGL_INCLUDE_DIR}
#    ${SFML_INCLUDE_DIR}
)

target_link_libraries(vm
    rc3200_lib_static
#    ${OPENGL_LIBRARIES}
#    ${SFML_LIBRARIES}
)


