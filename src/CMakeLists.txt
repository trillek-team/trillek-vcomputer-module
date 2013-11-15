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
find_package(SDL2)
if (NOT SDL2_FOUND)
    message("SDL2 not found! main executable will not display screen and virtual keyboard")
    set (SDL2_ENABLE 0)
else (NOT SDL2_FOUND)
    set (SDL2_ENABLE 1)
endif (NOT SDL2_FOUND)

find_package(OpenGL)
if (NOT OPENGL_FOUND)
    message("OpenGL not found! main executable will not display screen and virtual keyboard")
    set (SDL2_ENABLE 0)
endif (NOT OPENGL_FOUND)

find_package(GLEW)
if(NOT GLEW_FOUND)
  message("GLEW not found! main executable will not display screen and virtual keyboard")
  set (SDL2_ENABLE 0)
endif(NOT GLEW_FOUND)

#find_package(GLM)
#if(NOT GLM_FOUND)
#  message("GLM not found! main executable will not display screen and virtual keyboard")
#  set (SDL2_ENABLE 0)
#endif(NOT GLM_FOUND)

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

if(BUILD_tests) # Build tests
  if(SDL2_ENABLE)
    
    # Test CDA vram to texture and dispaly in OpenGL 
    add_executable( test_cda
        test_cda.cpp
    )

    include_directories( test_cda
        ${rc3200_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
        ${SDL2_INCLUDE_DIR}
    )

    target_link_libraries( test_cda
        rc3200_lib_static
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${GLM_LIBRARIES}
        ${SDL2_LIBRARY}
    )

  endif(SDL2_ENABLE)
endif(BUILD_tests)

