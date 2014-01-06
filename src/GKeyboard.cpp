/**
 * TR3200 VM - GKeyboard.cpp
 * Generic Keyboard device
 *
 */

#include "GKeyboard.hpp"
#include "VSFix.hpp"

#include <cassert>

namespace vm {
namespace keyboard {

GKeyboard::GKeyboard (dword_t j1, dword_t j2) : IDevice(j1, j2), k_status(0), e_kd_int(false), do_kd_int(false), e_ku_int(false), do_ku_int(false), mods(0), reg_handler(this) {
  keybuffer.clear();
}

GKeyboard::~GKeyboard() {
}

void GKeyboard::Tick (cpu::ICpu& cpu, unsigned n, const double delta) {
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

std::vector<ram::AHandler*> GKeyboard::MemoryBlocks() const { 
  auto handlers = IDevice::MemoryBlocks(); 
  handlers.push_back((ram::AHandler*)&reg_handler);

  return handlers;
}

void GKeyboard::PushKeyEvent (bool keydown, byte_t scancode) {
  if (keybuffer.size() < BSIZE ) {
   
    // Stores the modifiers
    if (keydown) { // KeyDown : Adds modifier
      switch (scancode) {
        case KEY_SHIFT :
          mods |= MOD_SHIFT;
          break;

        case KEY_CONTROL :
          mods |= MOD_CTRL;
          break;
        
        case KEY_ALT_GR :
          mods |= MOD_ALTGR;
          break;

        default:
          break;
      }
    } else { // KeyUp : Removes modifier
      switch (scancode) {
        case KEY_SHIFT :
          mods &= ~MOD_SHIFT;
          break;

        case KEY_CONTROL :
          mods &= ~MOD_CTRL;
          break;
        
        case KEY_ALT_GR :
          mods &= ~MOD_ALTGR;
          break;

        default:
          break;
      }
    }

    word_t k = scancode | ( keydown ? 0x100 : 0x000 );
    k |= (mods << 8); // Appends the modifiers bits

    keybuffer.push_front(k);
    // Will try to throw a interrupt
    do_kd_int = e_kd_int && keydown;
    do_ku_int = e_ku_int && !keydown;
  }
}

// KeybReg class inside GKeyboard class
    
GKeyboard::KeybReg::KeybReg (GKeyboard* gkey): read(0) {
  this->gkey = gkey;
  this->begin = BASE_ADDR[gkey->Jmp1() &3];
  this->size = 4;
}

GKeyboard::KeybReg::~KeybReg() {
}

byte_t GKeyboard::KeybReg::RB (dword_t addr) {
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

void GKeyboard::KeybReg::WB (dword_t addr, byte_t val) {
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

} // End of namespace keyboard
} // End of namespace vm
