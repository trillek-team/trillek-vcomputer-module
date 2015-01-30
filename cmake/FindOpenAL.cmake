# FindOpenAL.cmake
#
# Copyright (c) 2013, Meisaka Yukara

include(FindPackageHandleStandardArgs)

find_path(OPENAL_INCLUDE_DIR
	NAMES al.h alc.h
	PATH_SUFFIXES AL
	)
if (NOT OPENAL_INCLUDE_DIR)
    message(WARNING "Could not find the OpenAL headers" )
endif (NOT OPENAL_INCLUDE_DIR)

FIND_LIBRARY(OPENAL_LIBRARY
  NAMES OpenAL al openal OpenAL32 libopenal libopenal.so
  HINTS
    ENV OPENALDIR
  PATH_SUFFIXES x86 x64 amd64 lib64
	)
if (NOT OPENAL_LIBRARY)
    message(WARNING "Could not find the OpenAL library" )
endif (NOT OPENAL_LIBRARY)
SET(OPENAL_LIBRARIES ${OPENAL_LIBRARY})

find_package_handle_standard_args(OPENAL DEFAULT_MSG OPENAL_INCLUDE_DIR OPENAL_LIBRARY)
mark_as_advanced(OPENAL_INCLUDE_DIR OPENAL_LIBRARY)

