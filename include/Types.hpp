#pragma once
/**
 * Trillek Virtual Computer - types.hpp
 * Defines base types used by the virtual computer
 */

#ifndef __TYPES_HPP__
#define __TYPES_HPP__ 1

#if (! defined(_MSC_VER) || (_MSC_VER >= 1700))
	#include <cstdint>
namespace std {
	typedef size_t size_t;
}
#endif

namespace vm {

	// Visual Studio 2010 fix
#if (defined(_MSC_VER) && (_MSC_VER < 1700))
	typedef signed __int8				int8_t;
	typedef signed __int16			int16_t;
	typedef signed __int32			int32_t;
	typedef unsigned __int8			uint8_t;
	typedef unsigned __int16		uint16_t;
	typedef unsigned __int32		uint32_t;
	typedef signed __int64      int64_t;
	typedef unsigned __int64    uint64_t;
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
