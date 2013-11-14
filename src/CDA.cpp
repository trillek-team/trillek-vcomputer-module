#include "CDA.hpp"

namespace vm {
namespace cda {

void RGBATextureCDA (CDA& cda, dword_t* texture) {
  assert(texture != nullptr);

  const byte_t* buffer = cda.VRAM();
  assert(buffer != nullptr);

  dword_t fg, bg;
  unsigned addr;

  if (cda.isTextMode()) {
    // Text mode
    if (cda.VideoMode() == 0) {
      // Mode 0 40x30 -> 320x240

    } // else -> Unknow videomode. Not supported
  } else {
    // Graphics mode
    byte_t attr, pixels, pix;
    if (cda.VideoMode() == 0) {
      // Mode 8 256x192 8x8 atr. cells
      // (X,Y) = ((X % 256)/8) + (256 * Y / 8)     pixel bit (X) = X%8
      // attribute (X,Y) = 0x1800 + ((X % 256)/8) + (256 * Y / 8)/8
      

    } else if (cda.VideoMode() == 1) {
      // Mode 9 256x192 4x4 atr. cells
      // (X,Y) = ((X % 256)/8) + (256 * Y / 8)     pixel bit (X) = X%8
      // attribute (X,Y) = 0x1800 + ((X % 256)/4) + (256 * Y / 4)/4


    } else if (cda.VideoMode()  == 2) {
      // Mode 10 320x240 b&w
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
}

void RGBATexture (const byte_t* vram, unsigned vmode, bool userfont, bool userpal, bool textmode, dword_t* texture) {
  assert(texture != nullptr);

  const byte_t* buffer = vram;
  assert(buffer != nullptr);

  dword_t fg, bg;
  unsigned addr;

  if (textmode) {
    // Text mode
    if (vmode == 0) {
      // Mode 0 40x30 -> 320x240

    } // else -> Unknow videomode. Not supported
  } else {
    // Graphics mode
    byte_t attr, pixels, pix;
    if (vmode == 0) {
      // Mode 8 256x192 8x8 atr. cells
      // (X,Y) = ((X % 256)/8) + (256 * Y / 8)     pixel bit (X) = X%8
      // attribute (X,Y) = 0x1800 + ((X % 256)/8) + (256 * Y / 8)/8
      

    } else if (vmode == 1) {
      // Mode 9 256x192 4x4 atr. cells
      // (X,Y) = ((X % 256)/8) + (256 * Y / 8)     pixel bit (X) = X%8
      // attribute (X,Y) = 0x1800 + ((X % 256)/4) + (256 * Y / 4)/4


    } else if (vmode  == 2) {
      // Mode 10 320x240 b&w
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
}

} // End of namespace cda
} // End of namespace vm

