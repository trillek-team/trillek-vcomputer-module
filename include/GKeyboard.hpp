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

GKeyboard (dword_t j1 = 0, dword_t j2 = 0) : IDevice(j1, j2), k_status(0), e_int(false), do_int(false), reg_handler(this) {
  keybuffer.clear();
}

virtual ~GKeyboard() {

}

byte_t DevClass() const     {return 0x03;}   // Graphics device
word_t Builder() const      {return 0x0000;} // Generic builder
word_t DevId() const        {return 0x0001;} // Keyboard standard
word_t DevVer() const       {return 0x0000;} // Ver 0

virtual void Tick (cpu::RC3200& cpu, unsigned n=1, long delta = 0) {
  if (e_int && do_int) { // Try to thorow a interrupt
    auto ret = cpu.ThrowInterrupt(INT_MSG[this->Jmp1() &3]);
    if (ret) // If the CPU not accepts the interrupt, try again in the next tick
      do_int = false;
  }
}


virtual std::vector<ram::AHandler*> MemoryBlocks() const { 
  auto handlers = IDevice::MemoryBlocks(); 
  handlers.push_back((ram::AHandler*)&reg_handler);

  return handlers;
}

void PushKeyEvent (bool keydown, byte_t scancode) {
  if (keybuffer.size() < BSIZE ) {
    word_t k = scancode | ( keydown ? 0x0100 : 0x0000 );
    keybuffer.push_front(k);
    do_int = e_int; // Will try to throw a interrupt
    std::printf("\tPush Key %u, %u\n", scancode, keydown);
  }
}

protected:
  std::deque<uint16_t> keybuffer;   /// Stores the key events

  byte_t k_status;
  bool e_int, do_int;

  /**
   * Address Handler that manages 3 registers
   */
  class KeybReg : public ram::AHandler {
  public:
    KeybReg (GKeyboard* gkey): read(0) {
      this->gkey = gkey;
      this->begin = BASE_ADDR[gkey->Jmp1() &3];
      this->size = 3;
    }

    virtual ~KeybReg() {
    }

    byte_t RB (dword_t addr) {
      addr &= 3; // We only are interesed in the two least significant bits
      if (addr == 0) { // KEY_REG
        auto ret = gkey->keybuffer.back();
        gkey->keybuffer.pop_back();
        return ret;

      } else if (addr == 1) { // KEY_STATUS
        return gkey->k_status;

      } else if (addr == 2) { // KEY_CMD
        return read;

      } else
        return 0;
    }

    void WB (dword_t addr, byte_t val) {
      addr &= 3; // We only are interesed in the two least significant bits
      if (addr == 0) { // KEY_REG
        if (gkey->keybuffer.size() < BSIZE ) {
          gkey->keybuffer.push_back(val);
        }

      } else if (addr == 1) { // KEY_STATUS
        gkey->k_status = val;

      } else if (addr == 2) { // KEY_CMD
        read = 0;
        if (val == 0) { // Clear buffer command
          gkey->keybuffer.clear();

        } else if (val == 1) { // Disable Interrupt
          gkey->e_int = false;
        
        } else if (val == 2) { // Enable Interrupt
          gkey->e_int = true;
        
        } else if (val == 3) { // Num of elements in buffer
          read = (byte_t) gkey->keybuffer.size();
        }
      }
    }

    GKeyboard* gkey;
    byte_t read; // What return when we read KEY_CMD
  };

  GKeyboard::KeybReg reg_handler;

};


} // End of namespace keyboard
} // End of namespace vm

#endif  // __GKEYBOARD_HPP__

