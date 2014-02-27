#pragma once
/**
 * Trillek Virtual Computer - AddrListener.hpp
 * Defines an interface for Address Listeners
 */

#ifndef __ADDRLISTENER_HPP__
#define __ADDRLISTENER_HPP__

#include "Types.hpp"

namespace vm {

	/**
	 * Interface for a Address Listener
	 */
	class AddrListener {
		virtual byte_t ReadB (dword_t addr) = 0;
		virtual word_t ReadW (dword_t addr) = 0;
		virtual dword_t ReadDW (dword_t addr) = 0;

		virtual void WriteB (dword_t addr, byte_t val) = 0;
		virtual void WriteW (dword_t addr, word_t val) = 0;
		virtual void WriteDW (dword_t addr, dword_t val) = 0;

	};

} // End of namespace vm

#endif // __ADDRLISTENER_HPP__

