/**
 * Auxiliar functions
 * \file aux.cpp
 */

#include "aux.hpp"

#include <fstream>
#include <ios>

namespace vm {
  namespace aux {

    unsigned LoadROM (const std::string& filename, vm::VirtualComputer& vcomp) {
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

  } // end of namespace aux
} // end of namespace vm

