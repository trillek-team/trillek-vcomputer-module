if (CMAKE_BUILD_TYPE STREQUAL Debug)
  SET(linkFlags "-g4")
else() # Either MinSizeRel, RelWithDebInfo or Release, all which run with optimizations enabled.
  SET(linkFlags "-O2")
endif()
SET(linkFlags " --bind ${linkFlags}")

# Fake executable
add_executable( vm
  ${RC3200_SRC}
  ./vm_wrap.cpp
  )

include_directories( vm
  ${RC3200_INCLUDE_DIRS}
  
  )

configure_file(./index.html ${CMAKE_BINARY_DIR}/index.html COPYONLY)
configure_file(./js/jquery-1.10.2.min.js ${CMAKE_BINARY_DIR}/js/jquery-1.10.2.min.js COPYONLY)
configure_file(./js/glMatrix-0.9.5.min.js ${CMAKE_BINARY_DIR}/js/glMatrix-0.9.5.min.js COPYONLY)
configure_file(./js/webgl-utils.js ${CMAKE_BINARY_DIR}/js/webgl-utils.js COPYONLY)
configure_file(./js/main.js ${CMAKE_BINARY_DIR}/js/main.js COPYONLY)

# GOTCHA: If your project has custom link flags, these must be set *before* calling any of the em_link_xxx functions!
set_target_properties(vm PROPERTIES LINK_FLAGS  "${linkFlags}" )

em_link_pre_js(vm  ${CMAKE_CURRENT_SOURCE_DIR}/pre.js)



