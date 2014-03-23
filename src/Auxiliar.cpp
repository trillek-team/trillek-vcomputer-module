/**
 * Trillek Virtual Computer - Auxiliar.cpp
 * Some auciliar functions and methods
 */

#include "Auxiliar.hpp"

#include "VComputer.hpp"
#include "VSFix.hpp"

#include <fstream>
#include <ios>

namespace vm {
  namespace aux {

    int LoadROM (const std::string& filename, byte_t* rom) {
      int count;
      try {
        std::fstream f(filename, std::ios::in | std::ios::binary);
        if (!f.is_open())
          return -1;

        size_t size;

        auto begin = f.tellg();
        f.seekg (0, std::ios::end);
        auto end = f.tellg();
        f.seekg (0, std::ios::beg);

        size = end - begin;
        size = size > (MAX_ROM_SIZE) ? (MAX_ROM_SIZE) : size;

        f.read(reinterpret_cast<char*>(rom), size);
        count = size;
      } catch (...) {
        count = -1;
      }

      return count;
    }

  } // end of namespace aux
} // end of namespace vm

