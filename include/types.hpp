/**
 * \brief       Base types of Virtual Computer
 * \file        types.hpp
 * \copyright   The MIT License (MIT)
 */
#ifndef __TYPES_HPP_
#define __TYPES_HPP_ 1

#include <cstdint>
#include <cstddef> // std::size_t

namespace vm {

typedef uint64_t qword_t; /// Quad WORD
typedef uint32_t dword_t; /// Double WORD
typedef uint16_t word_t;  /// WORD
typedef uint8_t byte_t;   /// BYTE

typedef int64_t sqword_t; /// Signed Quad WORD
typedef int32_t sdword_t; /// Signed Double WORD
typedef int16_t sword_t;  /// Signed WORD
typedef int8_t sbyte_t;   /// Signed BYTE

} // End of namespace vm

#endif // __TYPES_HPP_
