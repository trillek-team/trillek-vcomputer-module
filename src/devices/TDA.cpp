/**
 * Trillek Virtual Computer - TDA.cpp
 * Nya Elektriska Text Display Adapter
 * @see https://github.com/trillek-team/trillek-computer/blob/master/TDA.md
 */

#include "devices/TDA.hpp"

#include <algorithm>
#include <cassert>

namespace vm {
	namespace dev {
		namespace tda {

      void TDAtoRGBATexture (const TDAState& state, dword_t* texture) {
        assert(texture != nullptr);

        if (state.buffer_ptr == 0) {
          std::fill_n(texture, TEXTURE_SIZE, 0xFF000000); // fill with black
          return;
        }

        const byte_t* font = ROM_FONT;
        if (state.font_ptr != 0) {
          font = (byte_t*) state.font_buffer;
        }
				
        // TODO Rewrite this to be more efficient and cache friendly, as now 
        // writes "jumping" in the output texture
        
        // Reads one by one each character of the text buffer
        for (unsigned row = 0; row < HEIGHT_CHARS; row++) {
          for (unsigned col = 0; col < WIDTH_CHARS; col++) {

            std::size_t addr = col + (WIDTH_CHARS * row);
            byte_t c = state.txt_buffer[addr]; // character

            // Get Ink (fg) and Paper (bg) colors
            dword_t fg = (state.txt_buffer[addr] >> 8) & 0x0F; // Bits 8-11
            dword_t bg = (state.txt_buffer[addr] >> 12)& 0x0F; // bits 12-15

            // Paint the texture
            byte_t pixels;
            for (unsigned y= 0; y < 8 ; y++) {
              pixels = font[c*8 + y];
              for (unsigned x= 0; x < 8 ; x++) {
                addr = x + col*8 + (40*8 * (y + row*8)); // Addres of the pixel in the buffer
                if ((pixels & (1 << (7-x))) != 0) { // Active, uses the Ink (fg)
                  texture[addr] = PALETTE[fg];
                } else {                        // Unactive, uses the Paper (bg)
                  texture[addr] = PALETTE[bg];
                }
              }
            }

          }
        }
			}

		} // End of namespace tda
	} // End of namespace dev
} // End of namespace vm

