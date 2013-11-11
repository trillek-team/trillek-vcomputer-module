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

unsigned const VSYNC = 25;              /// Vertical refresh frecuency
unsigned const VRAM_SIZE = 0x4400;      /// VRAM size
dword_t  const SETUP_OFFSET = 0xCC00;   /// SETUP register offset

dword_t INT_MSG[] = {                   /// Interrupt message value
		0x0000005A, 
		0x0000105A, 
		0x0000205A, 
		0x0000305A};

dword_t BASE_ADDR[] = {                 /// VRAM base address
		0xFF0A0000, 
		0xFF0B0000, 
		0xFF0C0000, 
		0xFF0D0000};

class CDA : public IDevice {
public:

CDA() : count(0), videomode(0), textmode(true), blink(false), userfont(false), e_vsync(false), vram(this), setupr(this), buffer(NULL) {
    buffer = new byte_t[VRAM_SIZE]();
}

virtual ~CDA() {
    if (buffer != NULL)
	    delete[] buffer;
}

byte_t DevClass() const     {return 0x0E;}   // Graphics device
word_t Builder() const      {return 0x0000;} // Generic builder
word_t DevId() const        {return 0x0001;} // CDA standard
word_t DevVer() const       {return 0x0000;} // Ver 0 -> CDA base standard

virtual void Tick (cpu::RC3200& cpu, unsigned n=1) {
    count += n;
	if (count >= (cpu.Clock()/VSYNC)) { // V-Sync Event
	    count -= cpu.Clock()/VSYNC;
				
		    if (e_vsync)
			    cpu.ThrowInterrupt(INT_MSG[this->Jmp1() &3]);
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
 * Bright background color attribute is blink, for Text videomode ?
 */
bool isBlinkAttr() const {
		return blink;
}

/**
 * Is using User defined font in a Text videomode ?
 */
bool isUserFont() const {
		return userfont;
}

/**
 * Generates/Updates a RGBA texture (4 byte per pixel) of the screen state
 * @param texture Ptr. to the texture. Must have a size enought to containt a 640x200 RGBA texture.
 */
void RGBATexture (dword_t* texture) const {
		assert(texture != NULL);

		dword_t fg, bg;
		unsigned addr;

		if (textmode) {
				// Text mode
				if (videomode == 0) {
						// 40x25 -> 320x200

				} else if (videomode == 1) {
						// 80x25 -> 640x200

				} // else -> Unknow videomode. Not supported
		} else {
				// Graphics mode
				byte_t attr, pixels, pix;
				if (videomode == 0) {
						// 320x200
						// (X,Y) = ((X % 320)/8) + (320 * Y / 8)     pixel bit (X) = X%8
						// attribute (X,Y) = 8000 + ((X % 320)/8) + (320 * Y / 8)/8
						
						for (unsigned y = 0; y < 200; y++ ) {
								for (unsigned x = 0; x < 320; x++ ) {
										pix = x % 8;

										if (x % 8 == 0) { // Update color only when we fin the next 8x8 cell
												// 1 - Get Fg and Bg colors
												addr = 8000 + (x / 8) + (320/8) * y /8;
												attr = buffer[addr];
												fg = attr & 7;
												if (!blink) {
														fg = (attr & 8) ? fg +9 : fg;
												} // TODO Blink
												bg = attr & 0xF;

												// 2 - Get the 8 pixel row from VRAM
												addr = x / 8 + (320/8) * y;
												pixels = buffer[addr];
										}

										// 3 - Put the pixel in the texture
										if (pixels & (1 << pix)) {  // Active   -> Foreground
												texture[x + y*320] = fg;
										} else {                    // Unactive -> Background
												texture[x + y*320] = bg;
										}
								}
						}

				} else if (videomode == 1) {
						// 640x200
						// (X,Y) = ((X % 640)/8) + (640 * Y / 8) pixel bit (X) = X%8
						// attribute (X,Y) = 16000 + ((X % 640)/8) + (640 * Y / 8)/8

						for (unsigned y = 0; y < 200; y++ ) {
								for (unsigned x = 0; x < 640; x++ ) {
										pix = x % 8;

										if (x % 8 == 0) { // Update color only when we fin the next 8x8 cell
												// 1 - Get Fg and Bg colors
												addr = 16000 + (x / 8) + (640/8) * y /8;
												attr = buffer[addr];
												fg = attr & 7;
												if (!blink) {
														fg = (attr & 8) ? fg +9 : fg;
												} // TODO Blink
												bg = attr & 0xF;

												// 2 - Get the 8 pixel row from VRAM
												addr = x / 8 + (640/8) * y;
												pixels = buffer[addr];
										}

										// 3 - Put the pixel in the texture
										if (pixels & (1 << pix)) {  // Active   -> Foreground
												texture[x + y*640] = fg;
										} else {                    // Unactive -> Background
												texture[x + y*640] = bg;
										}
								}
						}

				} // else -> Unknow videomode. Not supported
		}
}

const static dword_t pallete[]; /// Fixed Color pallette

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
				//std::printf("CDA -> ADDR: %08Xh VAL : 0x%02X\n", addr, val);
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
				cda->blink = (val & 0x20 ) != 0;
				cda->userfont = (val & 0x40 ) != 0;
				cda->e_vsync = (val & 0x80 ) != 0;
				
		}

		byte_t reg;
		CDA* cda; // Self-reference

};

unsigned count;         /// Cycle counter

unsigned videomode;     /// Actual video mode
bool textmode;          /// Is text mode ?
bool blink;             /// Blink attribute in textmode ?
bool userfont;          /// User Font ?
bool e_vsync;           /// Enable V-Sync interrupt ?

CDA::VideoRAM vram;
CDA::SETUPreg setupr;

byte_t* buffer;        /// Visible video ram buffer for grpahics representation of screen

};


} // End of namespace cda
} // End of namespace vm

#endif
