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
#include <deque>
#include <algorithm>
#include <memory>
#include <iostream>

#include <cassert>


namespace vm {
namespace keyboard {

static const dword_t INT_MSG[] = {                  /// Interrupt message value
    0x00000009, 
    0x00000109, 
    0x00000209, 
    0x00000309};

static const dword_t BASE_ADDR[] = {                /// VRAM base address
    0xFF000060, 
    0xFF000160, 
    0xFF000260, 
    0xFF000360};

static const size_t BSIZE = 64;                     /// Internal buffer size

enum SCANCODES {                                    /// Scancodes of keyevents
  BACKSPACE = 0x10,
  RETURN,
  INSERT,
  DELETE,

  ESC = 0x1B,
  
  // 0x20-0x7f: 7-bit ASCII characters
  
  ARROW_UP = 0x80,
  ARROW_DOWN,
  ARROW_LEFT,
  ARROW_RIGHT,
  
  SHIFT = 0x90,
  CONTROL,
};

class GKeyboard : public IDevice {
public:

GKeyboard (dword_t j1 = 0, dword_t j2 = 0) : IDevice(j1, j2) {
  keybuffer.clear();
}

virtual ~GKeyboard() {

}

byte_t DevClass() const     {return 0x03;}   // Graphics device
word_t Builder() const      {return 0x0000;} // Generic builder
word_t DevId() const        {return 0x0001;} // Keyboard standard
word_t DevVer() const       {return 0x0000;} // Ver 0

virtual void Tick (cpu::RC3200& cpu, unsigned n=1, long delta = 0) {
  // Does nothing
}


virtual std::vector<ram::AHandler*> MemoryBlocks() const { 
  auto handlers = IDevice::MemoryBlocks(); 
  // TODO create a handler for the 3 byte registers
  //handlers.push_back((ram::AHandler*)&vram);
  //handlers.push_back((ram::AHandler*)&setupr);

  return handlers;
}

void PushKeyEvent (bool keydown, byte_t scancode) {
  if (keybuffer.size() < BSIZE ) {
    word_t k = scancode | ( keydown ? 0x0100 : 0x0000 );
    keybuffer.push_front(k);
    // TODO Throw interrupt here if are enable
  }
}

protected:
  std::deque<uint16_t> keybuffer;   /// Stores the key events

private:

};


} // End of namespace keyboard
} // End of namespace vm

#endif  // __GKEYBOARD_HPP__

