#pragma once
/**
 * Trillek Virtual Computer - Auxiliar.hpp
 * Some auciliar functions and methods
 */

#ifndef __AUX_HPP__
#define __AUX_HPP__ 1

#include "Types.hpp"
#include "VComputer.hpp"

#include <string>
#include <fstream>
#include <ios>

namespace vm {
  namespace aux {

    /**
     * Load a raw binary file as ROM
     * @param filename Binary file with the ROM
     * @param rom buffer were to write it
     * @return Read size or negative value if fails
     */
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


#endif // __AUX_HPP__

