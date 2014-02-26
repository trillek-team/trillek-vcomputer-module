#pragma once
/**
 * Trillek Virtual Computer - AddrListener.hpp
 * Defines an abstract class for Address Listeners
 */

#ifndef __ADDRLISTENER_HPP__
#define __ADDRLISTENER_HPP__

#include "Types.hpp"

namespace vm {

	/**
	 * Abstract class for a Address Listener
	 */
	class AddrListener {
		byte_t ReadB (dword_t addr) = 0;
		word_t ReadW (dword_t addr) = 0;
		dword_t ReadDW (dword_t addr) = 0;

		void WriteB (dword_t addr, byte_t val) = 0;
		void WriteW (dword_t addr, word_t val) = 0;
		void WriteDW (dword_t addr, dword_t val) = 0;

	};

} // End of namespace vm

#endif // __ADDRLISTENER_HPP__

