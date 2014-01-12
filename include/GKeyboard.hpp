#pragma once
/**
 * TR3200 VM - GKeyboard.hpp
 * Generic Keyboard device
 *
 */

#ifndef __GKEYBOARD_HPP__
#define __GKEYBOARD_HPP__ 1

#include "Types.hpp"
#include "Ram.hpp"
#include "IDevice.hpp"

#include <vector>
#include <deque>
#include <algorithm>
#include <memory>

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

static const size_t BSIZE = 64; /// Internal buffer size

enum KEYCODES { /// Key codes of the events
  KEY_DELETE          = 0x05,
  KEY_ALT_GR          = 0x06,
  KEY_BACKSPACE       = 0x08,
  KEY_TAB             = 0x09,
  KEY_RETURN          = 0x0D,
  KEY_SHIFT           = 0x0E,
  KEY_CONTROL         = 0x0F,
  KEY_INSERT          = 0x10,
  KEY_ARROW_UP        = 0x12,
  KEY_ARROW_DOWN      = 0x13,
  KEY_ARROW_LEFT      = 0x14,
  KEY_ARROW_RIGHT     = 0x15,
  KEY_ESC             = 0x1B,
  KEY_SPACEBAR        = 0x20,
  KEY_APOSTROPHE      = 0x27,
  KEY_COMA            = 0x2C,
  KEY_MINUS           = 0x2D,
  KEY_PERIOD          = 0x2E,
  KEY_SLASH           = 0x2F,

  KEY_0               = 0X30,
  KEY_1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,

  KEY_SEMICOLON       = 0X3A,
  KEY_EQUAL           = 0X3B,

  KEY_A               = 'A',
  KEY_B               = 'B',
  KEY_C               = 'C',
  KEY_D               = 'D',
  KEY_E               = 'E',
  KEY_F               = 'F',
  KEY_G               = 'G',
  KEY_H               = 'H',
  KEY_I               = 'I',
  KEY_J               = 'J',
  KEY_K               = 'K',
  KEY_L               = 'L',
  KEY_M               = 'M',
  KEY_N               = 'N',
  KEY_O               = 'O',
  KEY_P               = 'P',
  KEY_Q               = 'Q',
  KEY_R               = 'R',
  KEY_S               = 'S',
  KEY_T               = 'T',
  KEY_U               = 'U',
  KEY_V               = 'V',
  KEY_W               = 'W',
  KEY_X               = 'X',
  KEY_Y               = 'Y',
  KEY_Z               = 'Z',

  KEY_LEFT_BRACKET    = 0x5B,
  KEY_BACKSLASH       = 0x5C,
  KEY_RIGHT_BRACKET   = 0x5D,
  KEY_GRAVE_ACCENT    = 0x60,

  KEY_UNKNOW          = 0xFF
  
};

enum KEY_MODS { /// Key modifier
  MOD_SHIFT           = 0x02,
  MOD_CTRL            = 0x04,
  MOD_ALTGR           = 0x08,
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

	DeviceFlags Flags () const	{return DeviceFlags::WITH_INTERRUPTS;}
  
	virtual void Tick (unsigned n=1, const double delta = 0);

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
  bool e_ku_int;                  /// Enable KeyUp interrupt ?

  byte_t mods;                    /// Actual modifiers being pressed. Set by buffer state

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

