#pragma once
/**
 * RC3200 VM - GKeyboard.hpp
 * Generic Keyboard device
 *
 */

#ifndef __GKEYBOARD_HPP__
#define __GKEYBOARD_HPP__ 1

#include "types.hpp"
#include "ram.hpp"
#include "IDevice.hpp"

#include <vector>
#include <algorithm>
#include <memory>
#include <iostream>

#include <cassert>


namespace vm {
namespace keyboard {

static const dword_t INT_MSG[] = {                   /// Interrupt message value
    0x00000009, 
    0x00000109, 
    0x00000209, 
    0x00000309};

static const dword_t BASE_ADDR[] = {                 /// VRAM base address
    0xFF000060, 
    0xFF000160, 
    0xFF000260, 
    0xFF000360};

class GKeyboard : public IDevice {
public:

GKeyboard (dword_t j1 = 0, dword_t j2 = 0) : IDevice(j1, j2) {

}

virtual ~GKeyboard() {

}

byte_t DevClass() const     {return 0x03;}   // Graphics device
word_t Builder() const      {return 0x0000;} // Generic builder
word_t DevId() const        {return 0x0001;} // Keyboard standard
word_t DevVer() const       {return 0x0000;} // Ver 0

virtual void Tick (cpu::RC3200& cpu, unsigned n=1, long delta = 0) {

}


virtual std::vector<ram::AHandler*> MemoryBlocks() const { 
  auto handlers = IDevice::MemoryBlocks(); 
  //handlers.push_back((ram::AHandler*)&vram);
  //handlers.push_back((ram::AHandler*)&setupr);

  return handlers;
}

private:

};


} // End of namespace keyboard
} // End of namespace vm

#endif  // __GKEYBOARD_HPP__

