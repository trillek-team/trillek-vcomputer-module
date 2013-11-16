
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
