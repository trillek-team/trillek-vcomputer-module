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

  GKeyboard (dword_t j1 = 0, dword_t j2 = 0);

  virtual ~GKeyboard();

  byte_t DevClass() const     {return 0x03;}   // Graphics device
  word_t Builder() const      {return 0x0000;} // Generic builder
  word_t DevId() const        {return 0x0001;} // Keyboard standard
  word_t DevVer() const       {return 0x0000;} // Ver 0

  virtual void Tick (cpu::RC3200& cpu, unsigned n=1, const double delta = 0);

  virtual std::vector<ram::AHandler*> MemoryBlocks() const;

  /**
   * Push a new key event to the keyboard buffer
   * @param keydown True if is a KeyDown event
   * @param scancode key id code
   */
  void PushKeyEvent (bool keydown, byte_t scancode);

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
    KeybReg (GKeyboard* gkey);

    virtual ~KeybReg();

    byte_t RB (dword_t addr);

    void WB (dword_t addr, byte_t val);

  protected:
    GKeyboard* gkey;
    byte_t read; // What return when we read KEY_CMD
  };

  GKeyboard::KeybReg reg_handler;

};


} // End of namespace keyboard
} // End of namespace vm

#endif  // __GKEYBOARD_HPP__

