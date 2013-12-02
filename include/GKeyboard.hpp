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

static const dword_t INT_KDOWN_MSG[] = {                  /// Interrupt message value
    0x00000009, 
    0x00001009, 
    0x00002009, 
    0x00003009};

static const dword_t INT_KUP_MSG[] = {                  /// Interrupt message value
    0x00000109, 
    0x00001109, 
    0x00002109, 
    0x00003109};

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

enum KEY_COMMANDS {   /// Generic Keyboard commands
  K_CLEAR,
  K_COUNT,
  K_D_INT_DOWN,
  K_E_INT_DOWN,
  K_D_INT_UP,
  K_E_INT_UP
};

class GKeyboard : public IDevice {
public:

GKeyboard (dword_t j1 = 0, dword_t j2 = 0) : IDevice(j1, j2), k_status(0), e_kd_int(false), do_kd_int(false), e_ku_int(false), do_ku_int(false), reg_handler(this) {
  keybuffer.clear();
}

virtual ~GKeyboard() {

}

byte_t DevClass() const     {return 0x03;}   // Graphics device
word_t Builder() const      {return 0x0000;} // Generic builder
word_t DevId() const        {return 0x0001;} // Keyboard standard
word_t DevVer() const       {return 0x0000;} // Ver 0

virtual void Tick (cpu::RC3200& cpu, unsigned n=1, const double delta = 0) {
  if (e_kd_int && do_kd_int) { // Try to thorow KeyDown interrupt
    auto ret = cpu.ThrowInterrupt(INT_KDOWN_MSG[this->Jmp1() &3]);
    if (ret) // If the CPU not accepts the interrupt, try again in the next tick
      do_kd_int = false;

  } else if (e_ku_int && do_ku_int) { // Try to thorow KeyUp interrupt
    auto ret = cpu.ThrowInterrupt(INT_KUP_MSG[this->Jmp1() &3]);
    if (ret) // If the CPU not accepts the interrupt, try again in the next tick
      do_ku_int = false;

  } 
}


virtual std::vector<ram::AHandler*> MemoryBlocks() const { 
  auto handlers = IDevice::MemoryBlocks(); 
  handlers.push_back((ram::AHandler*)&reg_handler);

  return handlers;
}

void PushKeyEvent (bool keydown, byte_t scancode) {
  if (keybuffer.size() < BSIZE ) {
    word_t k = scancode | ( keydown ? 0x100 : 0x000 );
    keybuffer.push_front(k);
    // Will try to throw a interrupt
    do_kd_int = e_kd_int && keydown;
    do_ku_int = e_ku_int && !keydown;
  }
}

protected:
  std::deque<word_t> keybuffer;   /// Stores the key events

  byte_t k_status;                /// Keyboard status byte
  bool e_kd_int;                  /// Enable KeyDown interrupt ?
  bool do_kd_int;                 /// Try to thorow KeyDown interrupt ?

  bool e_ku_int;                  /// Enable KeyUp interrupt ?
  bool do_ku_int;                 /// Try to thorow KeyUp interrupt ?

  /**
   * Address Handler that manages 3 registers
   */
  class KeybReg : public ram::AHandler {
  public:
    KeybReg (GKeyboard* gkey): read(0) {
      this->gkey = gkey;
      this->begin = BASE_ADDR[gkey->Jmp1() &3];
      this->size = 4;
    }

    virtual ~KeybReg() {
    }

    byte_t RB (dword_t addr) {
      addr &= 3; // We only are interesed in the two least significant bits
      if (addr == 0 && gkey->keybuffer.size() > 0) { // KEY_REG LSB
        auto ret = gkey->keybuffer.back();
        gkey->keybuffer.pop_back();
        return ret & 0xFF;

      } else if (addr == 1 && gkey->keybuffer.size() > 0) { // KEY_REG MSB
        auto ret = gkey->keybuffer.back();
        return ret >> 8;

      } else if (addr == 2) { // KEY_STATUS
        return gkey->k_status;

      } else if (addr == 3) { // KEY_CMD
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
        if (val == KEY_COMMANDS::K_CLEAR ) { // Clear buffer command
          gkey->keybuffer.clear();

        } else if (val == KEY_COMMANDS::K_COUNT ) { // Num of elements in buffer
          read = (byte_t) gkey->keybuffer.size();

        } else if (val == KEY_COMMANDS::K_D_INT_DOWN ) { // Disable KeyDown Interrupt
          gkey->e_kd_int = false;
        
        } else if (val == KEY_COMMANDS::K_E_INT_DOWN) { // Enable KeyDown Interrupt
          gkey->e_kd_int = true;
        
        } else if (val == KEY_COMMANDS::K_D_INT_UP ) { // Disable KeyUp Interrupt
          gkey->e_ku_int = false;
        
        } else if (val == KEY_COMMANDS::K_E_INT_UP) { // Enable KeyUp Interrupt
          gkey->e_ku_int = true;
        
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

