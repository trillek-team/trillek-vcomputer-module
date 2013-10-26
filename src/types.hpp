#pragma once
/**
 * RC3200 VM - types.hpp
 * Defines base types for the virtual CPU
 */

#ifndef __TYPES_HPP__
#define __TYPES_HPP__ 1

#include "endian.h"

#if (! defined(_MSC_VER) || (_MSC_VER >= 1700))
#include <cstdint>
#endif

namespace vm {

// Visual Studio 6 and Embedded Visual C++ 4 doesn't
// realize that, e.g. char has the same size as __int8
// so we give up on __intX for them.
#if (defined(_MSC_VER) && (_MSC_VER < 1700))
    #if (_MSC_VER < 1300)
        typedef signed char       int8_t;
        typedef signed short      int16_t;
        typedef signed int        int32_t;
        typedef unsigned char     uint8_t;
        typedef unsigned short    uint16_t;
        typedef unsigned int      uint32_t;
    #else
        typedef signed __int8     int8_t;
        typedef signed __int16    int16_t;
        typedef signed __int32    int32_t;
        typedef unsigned __int8   uint8_t;
        typedef unsigned __int16  uint16_t;
        typedef unsigned __int32  uint32_t;
    #endif
    typedef signed __int64       int64_t;
    typedef unsigned __int64     uint64_t;
#endif


typedef uint64_t qword_t;   /// Quad WORD
typedef uint32_t dword_t;   /// Double WORD
typedef uint16_t word_t;    /// WORD
typedef uint8_t  byte_t;    /// BYTE

typedef int64_t sqword_t;   /// Signed Quad WORD
typedef int32_t sdword_t;   /// Signed Double WORD
typedef int16_t sword_t;    /// Signed WORD
typedef int8_t  sbyte_t;    /// Signed BYTE

} //End of namespace vm

#endif // __TYPES_HPP__
