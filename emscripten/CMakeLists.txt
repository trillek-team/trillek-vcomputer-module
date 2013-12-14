
# Fake executable
add_executable( vm_wrap
    ../src/vcomputer.cpp
    ../src/IDevice.cpp
    ../src/cpu.cpp
    ../src/CDA.cpp
    ../src/GKeyboard.cpp
    ../src/dis_rc3200.cpp
    ./vm_wrap.cpp
)

include_directories( vm
    ${rc3200_INCLUDE_DIRS}
)

# GOTCHA: If your project has custom link flags, these must be set *before* calling any of the em_link_xxx functions!
set_target_properties(vm_wrap PROPERTIES LINK_FLAGS " --bind -s EXPORTED_FUNCTIONS=\"['_multiply_array']\" ")

