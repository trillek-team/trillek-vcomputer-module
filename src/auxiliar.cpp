/**
 * \brief       Virtual Computer Auxiliar functions
 * \file        auxiliar.cpp
 * \copyright   The MIT License (MIT)
 *
 * Some auciliar functions and methods
 */

#include "auxiliar.hpp"

#include "vcomputer.hpp"
#include "vs_fix.hpp"

#include <fstream>
#include <ios>
#include <cassert>

namespace vm {
namespace aux {

int LoadROM (const std::string& filename, Byte* rom) {
    assert(rom != nullptr);

    int count;
    try {
        std::fstream f(filename, std::ios::in | std::ios::binary);
        if ( !f.is_open() ) {
            return -1;
        }

        size_t size;

        auto begin = f.tellg();
        f.seekg (0, std::ios::end);
        auto end = f.tellg();
        f.seekg (0, std::ios::beg);

        size = end - begin;
        size = size > (MAX_ROM_SIZE) ? (MAX_ROM_SIZE) : size;

        f.read(reinterpret_cast<char*>(rom), size);
        count = size;
    }
    catch (...) {
        count = -1;
    }

    return count;
} // LoadROM
} // end of namespace aux
} // end of namespace vm
