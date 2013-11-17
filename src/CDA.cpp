#include "CDA.hpp"

namespace vm {
namespace cda {

void RGBATextureCDA (const CDA* cda, dword_t* texture) {
  RGBATexture (cda->VRAM(), cda->VideoMode(), cda->isUserFont(), cda->isUserPalette(), cda->isTextMode() , texture);
}

void RGBATexture (const byte_t* buffer, unsigned vmode, bool userfont, bool userpal, bool textmode, dword_t* texture) {
  assert(texture != nullptr);
  assert(buffer != nullptr);

#define BORDER_OFFSET (320*24)

  dword_t fg, bg;
  unsigned addr;

  if (textmode) {
    // Text mode
    if (vmode == 0) { 
      // Text Mode 0 40x30 8x8 -> 320x240
      // address = video ram address + (column % 40)*2 + (40*2 * row)
      const dword_t* pal = (userpal) ? (dword_t*)(buffer + 0x960) : palette;
      const byte_t* font = (userfont) ? (buffer + 0x990) : def_font;

      for (unsigned row = 0; row < 30; row++) {
        for (unsigned col = 0; col < 40; col++) {
          addr = col*2 + (40*2 * row);
          byte_t c = buffer[addr];
          fg = buffer[addr+1]  & 0xF;
          bg = (buffer[addr+1] >> 4) & 0xF;
            // Grab colors
#if (BYTE_ORDER != LITTLE_ENDIAN)
            fg = pal[fg] << 8 | 0xFF;
            bg = pal[bg] << 8 | 0xFF;
#else
            fg = pal[fg] | 0xFF000000;
            bg = pal[bg] | 0xFF000000;
#endif
          
          // Copy the glyph to the buffer
          byte_t pixels;
          for (unsigned y= 0; y < 8 ; y++) {
            pixels = font[c*8 + y];
            for (unsigned x= 0; x < 8 ; x++) {
              addr = x + col*8 + (40*8 * (y + row*8)); // Addres of the pixel in the buffer
              if ((pixels & (1 << x)) != 0) { // Active     -> Fg
                texture[addr] = fg;
              } else {                        // Unactive   -> Bg
                texture[addr] = bg;
              }
            }
          }

        } 
      }

    } // else -> Unknow videomode. Not supported
  } else {
    // Graphics mode
    byte_t pixels, pix;
    if (vmode == 0) {
      // Video Mode 0 256x192 8x8 atr. cells
      // (X,Y) = ((X % 256)/8) + (256 * Y / 8)     pixel bit (X) = X%8
      // attribute (X,Y) = 0x1800 + ((X % 256)/8) + (256 * Y / 8)/8
      const dword_t* pal = (userpal) ? (dword_t*)(buffer + 0x1B01) : palette;
      dword_t b_color = pal[buffer[0x1B00] & 0x0F];

      if (userpal) { // Add Alpha to border color
#if (BYTE_ORDER != LITTLE_ENDIAN)
        b_color = b_color << 8 | 0xFF;
#else
        b_color = b_color | 0xFF000000;
#endif
      }

      for (unsigned y = 24; y < 240-24; y++ ) {
        for (unsigned x = 32; x < 320-32; x++ ) {
          pix = x % 8;

          if (x % 8 == 0) { // Grab only the next 8 bits when is necesary
            addr = (x-32)/8 + (y-24)*32;
            pixels = buffer[addr];

            // Grab color index from attribute vram
            addr = 0x1800 + (x-32)/8 + ((y-24)>>3)*32;
            fg = buffer[addr] & 0xF;
            bg = buffer[addr] >> 4;
            // Grab colors
#if (BYTE_ORDER != LITTLE_ENDIAN)
            fg = pal[fg] << 8 | 0xFF;
            bg = pal[bg] << 8 | 0xFF;
#else
            fg = pal[fg] | 0xFF000000;
            bg = pal[bg] | 0xFF000000;
#endif
          }

          // Put the pixel in the texture 
          
          if (pixels & (1 << pix)) {  // Active   -> Foreground (Ink)
            texture[x + y*320] = fg;
          } else {                    // Unactive -> Background (Paper)
            texture[x + y*320] = bg;
          }

        }
      }

      // Fill borders
      for (unsigned y = 0; y < 24; y++ ) {
        for (unsigned x = 0; x < 320; x++ ) {
          texture[x + y*320] = b_color;
          texture[x + (239-y)*320] = b_color;
        }
      }
      for (unsigned y = 24; y < (240-24); y++ ) {
        for (unsigned x = 0; x < 32; x++ ) {
          texture[x + y*320] = b_color;
          texture[(319-x) + y*320] = b_color;
        }
      }


    } else if (vmode == 1) {
      // Video Mode 1 256x192 4x4 atr. cells
      // (X,Y) = ((X % 256)/8) + (256 * Y / 8)     pixel bit (X) = X%8
      // attribute (X,Y) = 0x1800 + ((X % 256)/4) + (256 * Y / 4)/4

      const dword_t* pal = (userpal) ? (dword_t*)(buffer + 0x2401) : palette;
      dword_t b_color = pal[buffer[0x2400] & 0x0F];

      if (userpal) { // Add Alpha to border color
#if (BYTE_ORDER != LITTLE_ENDIAN)
        b_color = b_color << 8 | 0xFF;
#else
        b_color = b_color | 0xFF000000;
#endif
      }

      for (unsigned y = 24; y < 240-24; y++ ) {
        for (unsigned x = 32; x < 320-32; x++ ) {
          pix = x % 8;

          if (x % 8 == 0) { // Grab only the next 8 bits when is necesary
            addr = (x-32) / 8 + (256/8) * (y-24);
            pixels = buffer[addr];
          }
          if (x % 4 == 0 ) { // We only need to get new color every 4 pixels
            // Grab color index from vram
            addr = 0x1800 + (x-32)/4 + ((y-24)>>2)*64; 
            fg = buffer[addr] & 0xF;
            bg = buffer[addr] >> 4;
            // Grab colors
#if (BYTE_ORDER != LITTLE_ENDIAN)
            fg = pal[fg] << 8 | 0xFF;
            bg = pal[bg] << 8 | 0xFF;
#else
            fg = pal[fg] | 0xFF000000;
            bg = pal[bg] | 0xFF000000;
#endif
          }

          // Put the pixel in the texture
          if (pixels & (1 << pix)) {  // Active   -> Foreground (Ink)
            texture[x + y*320] = fg;
          } else {                    // Unactive -> Background (Paper)
            texture[x + y*320] = bg;
          }

        }
      }

      // Fill borders
      for (unsigned y = 0; y < 24; y++ ) {
        for (unsigned x = 0; x < 320; x++ ) {
          texture[x + y*320] = b_color;
          texture[x + (239-y)*320] = b_color;
        }
      }
      for (unsigned y = 24; y < (240-24); y++ ) {
        for (unsigned x = 0; x < 32; x++ ) {
          texture[x + y*320] = b_color;
          texture[(319-x) + y*320] = b_color;
        }
      }

    } else if (vmode  == 2) {
      // Video Mode 2 320x240 b&w
      // (X,Y) = ((X % 320)/8) + (320/8 * Y)     pixel bit (X) = X%8

      for (unsigned y = 0; y < 240; y++ ) {
        for (unsigned x = 0; x < 320; x++ ) {
          pix = x % 8;

          if (x % 8 == 0) { // Grab only the next 8 bits when is necesary
            addr = x / 8 + (320/8) * y;
            pixels = buffer[addr];
          }

          // Put the pixel in the texture
          if (pixels & (1 << pix)) {  // Active
            texture[x + y*320] = 0xFFFFFFFF;
          } else {                    // Unactive
#if (BYTE_ORDER != LITTLE_ENDIAN)
            texture[x + y*320] = 0x000000FF;
#else
            texture[x + y*320] = 0xFF000000;
#endif
          }
        }
      }

    }// else -> Unknow videomode. Not supported
  }
#undef BORDER_OFFSET 
}

} // End of namespace cda
} // End of namespace vm

