if (CMAKE_HOST_APPLE)
    set(trillek_vcomputer_modulue_SEARCH_PATHS
        /usr
        /Applications/Xcode.app/Contents/Developer/Platforms.MacOSX.platform/Developer/SDKs
        /Library/Frameworks
        /usr/local
        /opt/local
    )
    INCLUDE_DIRECTORIES(/usr/include)
    INCLUDE_DIRECTORIES(/usr/local/include)
else (CMAKE_HOST_APPLE)
    # OS X is a Unix, but it's not a normal Unix as far as search paths go.
    if (CMAKE_HOST_UNIX)
        set(trillek_vcomputer_modulue_SEARCH_PATHS
            /usr
            /usr/local
            /opt/local
        )
    endif (CMAKE_HOST_UNIX)
endif (CMAKE_HOST_APPLE)

#------------------------------------------------------------------------------
# VS201x stuff

# If we are on windows add in the local search directories as well.
IF (WIN32 AND NOT MINGW) # Windows
    SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${CMAKE_SOURCE_DIR}/lib/include/)
    INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/lib/include/")
    IF (CMAKE_CL_64)
    LINK_DIRECTORIES("${CMAKE_SOURCE_DIR}/lib/x64/debug" "${CMAKE_SOURCE_DIR}/lib/x64/release")
    SET(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/lib/x64/debug ${CMAKE_SOURCE_DIR}/lib/x64/release)
    ELSE (CMAKE_CL_64)
    LINK_DIRECTORIES("${CMAKE_SOURCE_DIR}/lib/x86/debug" "${CMAKE_SOURCE_DIR}/lib/x86/release")
    SET(CMAKE_LIBRARY_PATH ${CMAKE_SOURCE_DIR}/lib/x86/debug ${CMAKE_SOURCE_DIR}/lib/x86/release)
    ENDIF (CMAKE_CL_64)
ENDIF (WIN32 AND NOT MINGW)
