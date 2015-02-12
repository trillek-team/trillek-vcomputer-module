# Options
SET(BUILD_TOOLS_SCREEN TRUE CACHE BOOL "Build Trillek VCOMPUTER tools with virtual screen")
SET(BUILD_TOOLS_AUDIO  TRUE CACHE BOOL "Build Trillek VCOMPUTER tools with audio")

# Give these some dummy values and if the platform is LINUX or OSX they will be set accordingly.
SET(X11_LIBRARIES "")
SET(OSX_LIBRARIES "")

# FIND GLFW3 AND OPENGL libs
FIND_PACKAGE(GLFW3)
IF (NOT GLFW3_FOUND)
    MESSAGE(WARNING "GLFW3 not found! toy emulator will not display screen and virtual keyboard")
    SET (GLFW3_ENABLE 0)
ELSE (NOT GLFW3_FOUND)
    SET (GLFW3_ENABLE 1)
ENDIF (NOT GLFW3_FOUND)

FIND_PACKAGE(OpenGL)
IF (NOT OPENGL_FOUND)
    MESSAGE(WARNING "OpenGL not found! toy emulator will not display screen and virtual keyboard")
    SET (GLFW3_ENABLE 0)
ENDIF (NOT OPENGL_FOUND)

FIND_PACKAGE(GLEW)
IF(NOT GLEW_FOUND)
    MESSAGE(WARNING "GLEW not found! toy emulator  will not display screen and virtual keyboard")
    SET (GLFW3_ENABLE 0)
ENDIF(NOT GLEW_FOUND)

FIND_PACKAGE(GLM)
IF(NOT GLM_FOUND)
    MESSAGE(WARNING "GLM not found! toy emulator will not display screen and virtual keyboard")
    SET (GLFW3_ENABLE 0)
ELSE()
    # Force using radians as degrees is deprecated
    # Bullet is built using doubles
    ADD_DEFINITIONS(-DGLM_FORCE_RADIANS -DBT_USE_DOUBLE_PRECISION)
ENDIF(NOT GLM_FOUND)

FIND_PACKAGE(OpenAL)
IF(NOT OPENAL_FOUND)
    MESSAGE(WARNING "OpenAL not found! toy emulator will not be hable to play the beeper")
    SET (OPENAL_ENABLE 0)
ELSE (NOT OPENAL_FOUND)

    FIND_PACKAGE(ALURE)
    IF(NOT ALURE_FOUND)
        MESSAGE(WARNING "ALure not found! toy emulator will not be hable to play the beeper")
        SET (OPENAL_ENABLE 0)
    ELSE(NOT ALURE_FOUND)
        SET (OPENAL_ENABLE 1)
    ENDIF(NOT ALURE_FOUND)

ENDIF(NOT OPENAL_FOUND)

IF (NOT BUILD_TOOLS_SCREEN)
    SET(GLFW3_ENABLE 0)
ENDIF (NOT BUILD_TOOLS_SCREEN)

IF (NOT BUILD_TOOLS_AUDIO)
    SET(OPENAL_ENABLE 0)
ENDIF (NOT BUILD_TOOLS_AUDIO)
# Main executable

# Main executable config file
CONFIGURE_FILE (./config_main.hpp.in
    "${PROJECT_BINARY_DIR}/config_main.hpp"
    )

# Setup include and libs to be used in main/vm
SET(VM_INCLUDE_DIRS
    ${VCOMPUTER_INCLUDE_DIRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    )

# Links agains the static version if is enabled
IF(BUILD_STATIC_VCOMPUTER)
    SET(VM_LINK_LIBS
        VCOMPUTER_STATIC
        )
ELSEIF(BUILD_DYNAMIC_VCOMPUTER)
    SET(VM_LINK_LIBS
        VCOMPUTER
        )
ENDIF(BUILD_STATIC_VCOMPUTER)

SET(MEDIA_INCLUDE_DIRS
    ${VM_INCLUDE_DIRS}
    )
SET(MEDIA_LINK_LIBS
    ${VM_LINK_LIBS}
    )

# If we have OpenGL / GLFW3 / GLM / GLEW libs
IF (GLFW3_ENABLE EQUAL 1)
    IF (NOT APPLE) # X11 and GLEW are not needed on OSX.
        FIND_PACKAGE(X11)
        SET(USE_STATIC_GLEW CACHE BOOL "Build against GLEW static (default no)")
        FIND_PACKAGE(GLEW REQUIRED) # We find GLEW here as OSX doesn't need it.
    ENDIF (NOT APPLE)

    IF (APPLE) # Mac OSX
        SET(GLEW_LIBRARY "") # Set a dummy value for GLEW.

        SET(CMAKE_XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS macosx)

        # Need the 10.7 SDK or later.
        EXECUTE_PROCESS(COMMAND xcodebuild -sdk macosx -version SDKVersion OUTPUT_VARIABLE OSX_SDK_VERSION)
        IF (NOT (OSX_SDK_VERSION VERSION_GREATER 10.7 OR OSX_SDK_VERSION VERSION_EQUAL 10.7))
            MESSAGE(FATAL_ERROR "The installed version of Xcode does not support the 10.7 SDK or later.  Please upgrade Xcode and try again.")
        ENDIF (NOT (OSX_SDK_VERSION VERSION_GREATER 10.7 OR OSX_SDK_VERSION VERSION_EQUAL 10.7))

        # Configure the project to use the correct SDK.
        IF (XCODE_VERSION)
            SET(CMAKE_OSX_SYSROOT macosx)
        ELSE (XCODE_VERSION)
            # Non-Xcode generators need the full path.
            EXECUTE_PROCESS(COMMAND xcodebuild -sdk macosx -version Path | head -n 1 OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
            STRING(REGEX REPLACE "(\r?\n)+$" "" CMAKE_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}")
        ENDIF (XCODE_VERSION)

        # Can deploy back to 10.7, the first OS X to support the GL Core.
        SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.7)

        # Need Core Foundation and libobjc.
        SET(OSX_LIBRARIES "-framework CoreFoundation /usr/lib/libobjc.dylib")
    ENDIF (APPLE)

    MESSAGE(STATUS "vc executable have Virtual Screen enabled")
    # Some auxiliar libs
    FILE(GLOB TOOL_OTHER_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/devices/*.hpp"
        )

    SET(MEDIA_INCLUDE_DIRS
        ${MEDIA_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
        ${GLFW3_INCLUDE_PATH}
        )

    SET(MEDIA_LINK_LIBS
        ${X11_LIBRARIES}
        ${OSX_LIBRARIES}
        ${MEDIA_LINK_LIBS}
        ${OPENGL_LIBRARIES}
        ${GLEW_LIBRARIES}
        ${GLM_LIBRARIES}
        ${GLFW3_LIBRARIES}
        )

    # TDA VIEWER Can only be build with OpenGL stuff
    ADD_EXECUTABLE( tda_view
        ./tda_view.cpp
        )
    SET(TARGETS ${TARGETS} "tda_view")

    INCLUDE_DIRECTORIES( tda_view
        ${MEDIA_INCLUDE_DIRS}
        )

    TARGET_LINK_LIBRARIES( tda_view
        ${MEDIA_LINK_LIBS}
        )

ENDIF (GLFW3_ENABLE EQUAL 1)

# If we have OpenAL
IF (OPENAL_ENABLE EQUAL 1)
    MESSAGE(STATUS "vc executable have Sound enabled")

    SET(MEDIA_INCLUDE_DIRS
        ${MEDIA_INCLUDE_DIRS}
        ${OPENAL_INCLUDE_DIR}
        ${ALURE_INCLUDE_DIR}
        )

    SET(MEDIA_LINK_LIBS
        ${MEDIA_LINK_LIBS}
        ${OPENAL_LIBRARIES}
        ${ALURE_LIBRARIES}
        )

ENDIF (OPENAL_ENABLE EQUAL 1)

# Main executable
ADD_EXECUTABLE( vc
    ./main.cpp
    ${TOOL_OTHER_SRC}
    )
SET(TARGETS ${TARGETS} "vc")

INCLUDE_DIRECTORIES( vc
    ${MEDIA_INCLUDE_DIRS}
    )

TARGET_LINK_LIBRARIES( vc
    ${MEDIA_LINK_LIBS}
    )

#pbm2font tool
ADD_EXECUTABLE( pbm2tdafont
    ./pbm2font.cpp
    )
SET(TARGETS ${TARGETS} "pbm2tdafont")

INCLUDE_DIRECTORIES( pbm2tdafont
    ${VM_INCLUDE_DIRS}
    )
TARGET_LINK_LIBRARIES( pbm2tdafont
    ${VM_LINK_LIBS}
    )

# makedisk executable
ADD_EXECUTABLE( maketrdisk
    ./makedisk.cpp
    )
SET(TARGETS ${TARGETS} "maketrdisk")

INCLUDE_DIRECTORIES( maketrdisk
    ${VM_INCLUDE_DIRS}
    )

TARGET_LINK_LIBRARIES( maketrdisk
    ${VM_LINK_LIBS}
    )

INSTALL(CODE "MESSAGE(\"Installing tools\")")
INSTALL(TARGETS ${TARGETS}
    COMPONENT toolsbin
    RUNTIME DESTINATION bin
    )

