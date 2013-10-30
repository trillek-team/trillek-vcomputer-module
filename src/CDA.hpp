#pragma once
/**
 * RC3200 VM - CDA.hpp
 * Color Display Adapter device
 *
 */

#ifndef __CDA_HPP__
#define __CDA_HPP__ 1

#include "types.hpp"
#include "ram.hpp"
#include "IDevice.hpp"

#include <vector>
#include <algorithm>
#include <memory>
#include <iostream>

#include <cassert>


namespace vm {
namespace cda {

class CDA : public IDevice {
public:

CDA()
{ }

virtual ~CDA()
{ }

byte_t dev_class() const    {return 0x0E;}   // Graphics device
word_t builder() const      {return 0x0000;} // Generic builder
word_t dev_id() const       {return 0x0001;} // CDA standard
word_t dev_ver() const      {return 0x0000;} // Ver 0 -> CDA base standard

private:
unsigned videomode;


};

class TextRAM : public ram::AHandler {
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
