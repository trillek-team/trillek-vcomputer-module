#include(GL)
include(SFML)

set(rc1600_SRCS
    ./cpu.cpp
    ./ram.cpp
    ./dis_rc1600.cpp
    )

#set(dcpu-vm_SFML_SRCS
#    ./sfml/square_gen.cpp
#    ./sfml/KeyboardWindow.cpp
#    ./sfml/MonitorWindow.cpp
#    )


# RC1600 VM core lib
add_library( rc1600_lib
    SHARED
    ${rc1600_SRCS}
)

add_library( rc1600_lib_static
    STATIC
    ${rc1600_SRCS}
    )
    
include_directories(rc1600_lib
    ${rc1600_INCLUDE_DIRS}
)

    
# Main executable
add_executable(vm
    cpu_vm.cpp
    )

include_directories(vm
    ${rc1600_INCLUDE_DIRS}
#    ${OPENGL_INCLUDE_DIR}
    ${SFML_INCLUDE_DIR}
)

target_link_libraries(vm
    rc1600_lib_static
#    ${OPENGL_LIBRARIES}
    ${SFML_LIBRARIES}
)


