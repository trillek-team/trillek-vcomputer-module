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

CDA() {
}

virtual ~CDA() {
}

byte_t DevClass() const     {return 0x0E;}   // Graphics device
word_t Builder() const      {return 0x0000;} // Generic builder
word_t DevId() const        {return 0x0001;} // CDA standard
word_t DevVer() const       {return 0x0000;} // Ver 0 -> CDA base standard

virtual void Tick (cpu::RC3200& cpu, unsigned n=1) {
    // TODO V-Sync interrupt generation
}

virtual std::vector<ram::AHandler*> MemoryBlocks() const { 
    auto handlers = IDevice::MemoryBlocks(); 
    handlers.push_back((ram::AHandler*)&vram);
    handlers.push_back((ram::AHandler*)&setupr);

    return handlers;
}

protected:

/**
 * Address Handler that manages the VideoRAM
 */
class VideoRAM : public ram::AHandler {
public:
    VideoRAM () : AHandler(0xFF0A0000, 0x4400) {
    }

    byte_t RB (dword_t addr) {
        return 0x55;
    }

    void WB (dword_t addr, byte_t val) {
        std::printf("CDA -> ADDR: %08Xh VAL : 0x%08X\n", addr, val);
    }
};

/**
 * Address block that manages the SETUP register
 */
class SETUPreg : public ram::AHandler {
public:
    SETUPreg () : AHandler(0xFF0ACC00, 1) {
    }

    byte_t RB (dword_t addr) {
        return 0x55;
    }

    void WB (dword_t addr, byte_t val) {
        std::printf("CDA -> ADDR: %08Xh VAL : 0x%08X\n", addr, val);
    }
};

unsigned videomode;     /// Actual video mode

CDA::VideoRAM vram;
CDA::SETUPreg setupr;

};


} // End of namespace cda
} // End of namespace vm

#endif
