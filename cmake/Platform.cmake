if (CMAKE_HOST_APPLE)
    set(rc32000_SEARCH_PATHS
        /usr
        /Applications/Xcode.app/Contents/Developer/Platforms.MacOSX.platform/Developer/SDKs
        /Library/Frameworks
        /usr/local
        /opt/local
    )
else (CMAKE_HOST_APPLE)
    # OS X is a Unix, but it's not a normal Unix as far as search paths go.
    if (CMAKE_HOST_UNIX)
        set(rc32000_SEARCH_PATHS
            /usr
            /usr/local
            /opt/local
        )
    endif (CMAKE_HOST_UNIX)
endif (CMAKE_HOST_APPLE)



if (MSVC)
	if(MSVC_VERSION GREATER 1650)
    message("This build has not yet been well tested with VC")
	else(MSVC_VERSION GREATER 1650)
	message(WARNING "This version of VC does not contain all needed C++11 features")
	endif(MSVC_VERSION GREATER 1650)
    set(PLATFORM_FLAGS)
    set(DCPU_VM_SEARCH_PATHS
        "C:/Program Files (x86)/Microsoft Visual Studio 11.0/VC"
        "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC"
    )
else (MSVC)
    set(PLATFORM_FLAGS "-Wall" ) # Generic flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
    
    if (CMAKE_HOST_WIN32)
      set(DCPU_VM_SEARCH_PATHS
          C:/MinGW
          )
    endif (CMAKE_HOST_WIN32)
    
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
endif (MSVC)

add_definitions(${PLATFORM_FLAGS})



