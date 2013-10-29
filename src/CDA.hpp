
#pragma once
/**
 * RC3200 VM - CDA.hpp
 * Color Display Adapter device
 *
 */

#ifndef __CDA_HPP__
#define __CDA_HPP__ 1

#include "types.hpp"

#include <vector>
#include <algorithm>
#include <memory>
#include <iostream>

#include <cassert>


namespace vm {
namespace cda {

class TextRAM : public vm::ram::AHandler {
public:
    TextRAM () : AHandler(0xFF0A0000, 0xA3E80) 
    { }

    byte_t rb (dword_t addr)
    {
        return 0x55;
    }

    void wb (dword_t addr, byte_t val)
    {
        std::printf("CDA -> ADDR: %08Xh VAL : 0x%08X\n", addr, val);
    }
};

} // End of namespace cda
} // End of namespace vm

#endif
