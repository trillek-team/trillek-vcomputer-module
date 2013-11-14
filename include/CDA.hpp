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

static const unsigned VSYNC = 25;              /// Vertical refresh frecuency
static const unsigned VRAM_SIZE = 0x2580;      /// VRAM size
static const dword_t  SETUP_OFFSET = 0xCC00;   /// SETUP register offset

static const dword_t INT_MSG[] = {                   /// Interrupt message value
    0x0000005A, 
    0x0000105A, 
    0x0000205A, 
    0x0000305A};

static const dword_t BASE_ADDR[] = {                 /// VRAM base address
    0xFF0A0000, 
    0xFF0B0000, 
    0xFF0C0000, 
    0xFF0D0000};

class CDA;

typedef void (*VSync_CBack)(CDA*, const byte_t*); /// Callback for virtual VSync (to upload video data to GPU or send it by network)

/**
 * Color Display Adapter device
 */
class CDA : public IDevice {
public:

CDA() : count(0), videomode(0), textmode(true), userpal(false), userfont(false), e_vsync(false), vram(this), setupr(this), buffer(nullptr), vsync_cb(nullptr), v_sync_int(false) {
    buffer = new byte_t[VRAM_SIZE]();
}

virtual ~CDA() {
    if (buffer != nullptr)
      delete[] buffer;
}

byte_t DevClass() const     {return 0x0E;}   // Graphics device
word_t Builder() const      {return 0x0000;} // Generic builder
word_t DevId() const        {return 0x0001;} // CDA standard
word_t DevVer() const       {return 0x0000;} // Ver 0 -> CDA base standard

virtual void Tick (cpu::RC3200& cpu, unsigned n=1) {
  // TODO Make VSync async respect CPU clock speed and use realtime 25 Hz
  count += n;
  if (count >= (cpu.Clock()/VSYNC)) { // V-Sync Event
    count -= cpu.Clock()/VSYNC;
       
    // try to call to the callback
    if (vsync_cb != nullptr)
      vsync_cb (this, buffer);

    if (e_vsync || v_sync_int) {
      auto ret = cpu.ThrowInterrupt(INT_MSG[this->Jmp1() &3]);
      if (!ret) // If the CPU not accepts the interrupt, try again in the next tick
        v_sync_int = true;
      else
        v_sync_int = false;
    }
  }
}

virtual std::vector<ram::AHandler*> MemoryBlocks() const { 
  auto handlers = IDevice::MemoryBlocks(); 
  handlers.push_back((ram::AHandler*)&vram);
  handlers.push_back((ram::AHandler*)&setupr);

  return handlers;
}

/**
 * Video RAM buffer
 */
const byte_t* VRAM() const {
  return buffer;
}

/**
 * Actual videomode
 */
unsigned VideoMode() const {
  return videomode;
}

/**
 * Is using a Text Videomode ?
 */
bool isTextMode() const {
  return textmode;
}

/**
 * Is using User defined palette ?
 */
bool isUserPalette() const {
  return userpal;
}

/**
 * Is using User defined font in a Text videomode ?
 */
bool isUserFont() const {
  return userfont;
}

/**
 * Sets the VSync callback function
 * Set it to allow do stuff every time that a vsync event happens, like upload 
 * vram to GPU or send VRAM to a remote client
 * @param new_cb Callback function. Set to nullptr to disable it.
 */
void VSyncCallBack (VSync_CBack new_cb) {
  vsync_cb = new_cb;
}

protected:

  /**
   * Address Handler that manages the VideoRAM
   */
  class VideoRAM : public ram::AHandler {
  public:
    VideoRAM (CDA* cda) {
      this->cda = cda;
      this->begin = BASE_ADDR[cda->Jmp1() &3];
      this->size = VRAM_SIZE;
    }

    virtual ~VideoRAM() {
    }

    byte_t RB (dword_t addr) {
      addr -= this->begin;
      assert(addr < this->size);
      assert(addr >= 0);

      return cda->buffer[addr];
    }

    void WB (dword_t addr, byte_t val) {
      addr -= this->begin;
      assert(addr < this->size);
      assert(addr >= 0);
      
      cda->buffer[addr] = val;
    }


    CDA* cda; // Self-reference
  };

  /**
   * Address block that manages the SETUP register
   */
  class SETUPreg : public ram::AHandler {
  public:
    SETUPreg (CDA* cda) {
      this->begin = BASE_ADDR[cda->Jmp1() &3] + SETUP_OFFSET;
      this->size = 1;
      this->cda = cda;
    }

    byte_t RB (dword_t addr) {
      return reg;
    }

    void WB (dword_t addr, byte_t val) {
      reg = val;
      cda->videomode = val & 7; 
      cda->textmode = (val & 8) == 0;
      cda->userfont = (val & 0x20 ) != 0;
      cda->userpal = (val & 0x40 ) != 0;
      cda->e_vsync = (val & 0x80 ) != 0;
    }

    byte_t reg;
    CDA* cda; // Self-reference

  };

unsigned count;         /// Cycle counter

unsigned videomode;     /// Actual video mode
bool textmode;          /// Is text mode ?
bool userpal;           /// User palette ?
bool userfont;          /// User Font ?
bool e_vsync;           /// Enable V-Sync interrupt ?

CDA::VideoRAM vram;
CDA::SETUPreg setupr;

byte_t* buffer;        /// Visible video ram buffer for grpahics representation of screen

VSync_CBack vsync_cb;

bool v_sync_int;

};


/**
 * Generates/Updates a RGBA texture (4 byte per pixel) of the screen state
 * @param cda Ref. to the CDA card from were generate the texture.
 * @param texture Ptr. to the texture. Must have a size enought to containt a 320x240 RGB texture.
 */
void RGBATextureCDA (CDA& cda, dword_t* texture);
void RGBATexture (const byte_t* vram, unsigned vmode, bool userfont, bool userpal, bool textmode, dword_t* texture);

static const dword_t pallete[] = {  /// Default color palette  
#if (BYTE_ORDER != LITTLE_ENDIAN)
// Big Endian -> RGBA
    0x000000FF, // Black
    0x0000CDFF, // Dark Blue
    0x00CD00FF, // Dark Green
    0x00CDCDFF, // Dark Cyan
    0xCD0000FF, // Dark Red
    0xCD00CDFF, // Dark Magenta
    0xAA5500FF, // Brown
    0xCDCDCDFF, // Light Gray
    // Bright colors
    0x555555FF, // Dark Grey
    0x0000FFFF, // Blue
    0x00FF00FF, // Green
    0x00FFFFFF, // Cyan
    0xFF0000FF, // Red
    0xFF00FFFF, // Magenta
    0xFFFF00FF, // Yellow
    0xFFFFFFFF, // White
};
#else
// Little Endian -> ABGR
    0xFF000000, // Black
    0xFFCD0000, // Dark Blue
    0xFF00CD00, // Dark Green
    0xFFCDCD00, // Dark Cyan
    0xFF0000CD, // Dark Red
    0xFFCD00CD, // Dark Magenta
    0xFF0055AA, // Brown
    0xFFCDCDCD, // Light Gray
    // Bright colors
    0xFF555555, // Dark Grey
    0xFFFF0000, // Blue
    0xFF00FF00, // Green
    0xFFFFFF00, // Cyan
    0xFF0000FF, // Red
    0xFFFF00FF, // Magenta
    0xFF00FFFF, // Yellow
    0xFFFFFFFF, // White
};
#endif

} // End of namespace cda
} // End of namespace vm

#endif
