#pragma once
/**
 * RC3200 VM - types.hpp
 * Defines base types for the virtual CPU
 */

#ifndef __TYPES_HPP__
#define __TYPES_HPP__ 1

#include "endian.h"

#include <cstdint>

namespace vm {

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
