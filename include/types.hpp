/**
 * \brief       Basic types used by the Virtual Computer
 * \file        types.hpp
 * \copyright   LGPL v3
 */
#ifndef __TYPES_HPP_
#define __TYPES_HPP_ 1

#include <cstdint>
#include <cstddef> // std::size_t

namespace trillek {

typedef uint64_t QWord;   /// unsigned Quad Word data
typedef uint32_t DWord;   /// unsigned Double Word data
typedef uint16_t Word;    /// unsigned Word data
typedef uint8_t  Byte;    /// unsigned Byte data

typedef int64_t  SQWord;  /// signed Quad Word
typedef int32_t  SDWord;  /// signed Double Word
typedef int16_t  SWord;   /// signed Word
typedef int8_t   SByte;   /// signed Byte

} // End of namespace trillek

#endif // __TYPES_HPP_
