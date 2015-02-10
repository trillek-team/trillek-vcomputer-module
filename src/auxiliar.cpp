/**
 * \brief       Virtual Computer Auxiliar functions
 * \file        auxiliar.cpp
 * \copyright   LGPL v3
 *
 * Some auciliar functions and methods
 */

#include "auxiliar.hpp"

#include "vcomputer.hpp"
#include "vs_fix.hpp"

#include <fstream>
#include <ios>
#include <cassert>

namespace trillek {
namespace computer {

int LoadROM (const std::string& filename, Byte* rom) {
    assert(rom != nullptr);

    int count;
    try {
        std::fstream f(filename, std::ios::in | std::ios::binary);
        if ( !f.is_open() ) {
            return -1;
        }
        count = LoadROM(f, rom);

    }
    catch (...) {
        count = -1;
    }

    return count;
} // LoadROM


int LoadROM (std::istream& stream, Byte* rom) {
    assert(rom != nullptr);
    
    int count;
    try {
        size_t size;
        
        auto begin = stream.tellg();
        stream.seekg (0, std::ios::end);
        auto end = stream.tellg();
        stream.seekg (0, std::ios::beg);
        
        size = end - begin;
        size = size > (MAX_ROM_SIZE) ? (MAX_ROM_SIZE) : size;
        
        stream.read(reinterpret_cast<char*>(rom), size);
        count = size;
    }
    catch (...) {
        count = -1;
    }
    
    return count;
} // LoadROM

} // end of namespace computer
} // end of namespace trillek
