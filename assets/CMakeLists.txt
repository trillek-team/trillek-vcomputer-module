# Shaders

# Vertex Shaders
configure_file(shaders/basic_vs.vert ${CMAKE_CURRENT_BINARY_DIR}/shaders/basic_vs.vert COPYONLY)
configure_file(shaders/mvp_template.vert ${CMAKE_CURRENT_BINARY_DIR}/shaders/mvp_template.vert COPYONLY)

# Fragment Shaders
configure_file(shaders/basic_fs.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/basic_fs.frag COPYONLY)
configure_file(shaders/basic_texture.frag ${CMAKE_CURRENT_BINARY_DIR}/shaders/basic_texture.frag COPYONLY)

