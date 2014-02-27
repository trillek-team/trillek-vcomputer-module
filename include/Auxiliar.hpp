#pragma once
#ifndef __AUX_HPP__
#define __AUX_HPP__ 1
/**
 * Some semi-generic auxiliar functions related to the vm
 */

#ifdef __NOT_REWRITE_YET_

#include "VComputer.hpp"

#include <string>
#include <fstream>
#include <ios>

namespace vm {
namespace aux {

  /**
   * Load a raw binary file as ROM
   * \param filename Binary file with the ROM, max size 64KiB
   * \param vcomp Virtual Computer
   * \return Read size or negative value if fails
   */
	template <typename CPU_t> 
  unsigned LoadROM (const std::string& filename, vm::VirtualComputer<CPU_t>& vcomp) {
		vm::byte_t rom[64*1024]; // Temporal buffer

		std::fstream f(filename, std::ios::in | std::ios::binary);
		if (!f.is_open())
			return -1;

		unsigned count = 0;
		while (f.good() && count < 64*1024) {
			f.read(reinterpret_cast<char*>(rom + count++), 1); // Read byte at byte, so the file must be in Little Endian
		}
		vcomp.WriteROM(rom, count);

		return count;
	}

  /**
   * Maps GLFW3 Key codes to TR3200 keycodes
   */
  byte_t GLFWKeyToTR3200 (int key);

  /**
   * Maps SDL2 Scancodes to TR3200 keycodes
   */
  byte_t SDL2KeyToTR3200 (int key);

} // end of namespace aux
} // end of namespace vm

#endif //__NOT_REWRITE_YET_

#endif // __AUX_HPP__

