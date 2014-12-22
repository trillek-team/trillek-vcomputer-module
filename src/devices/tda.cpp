/**
 * \brief       Virtual Computer Text Display Adapter
 * \file        tda.cpp
 * \copyright   The MIT License (MIT)
 *
 * Nya Elektriska Text Display Adapter
 * @see https://github.com/trillek-team/trillek-computer/blob/master/TDA.md
 */

#include "devices/tda.hpp"
#include "vs_fix.hpp"

#include <algorithm>
#include <cassert>

namespace trillek {
namespace computer {
namespace tda {

void TDAtoRGBATexture (const TDAScreen& screen, DWord* texture, unsigned& frames) {
    assert(texture != nullptr);

    const Byte* font = ROM_FONT;
    if (screen.user_font) {
        font = (Byte*) screen.font_buffer;
    }

    // TODO Rewrite this to be more efficient and cache friendly, as now
    // writes "jumping" in the output texture

    // Reads one by one each character of the text buffer
    for (unsigned row = 0; row < HEIGHT_CHARS; row++) {
        for (unsigned col = 0; col < WIDTH_CHARS; col++) {

            std::size_t addr = col + (WIDTH_CHARS * row);
            Byte c         = screen.txt_buffer[addr]; // character

            // Get Ink (fg) and Paper (bg) colors
            DWord fg = (screen.txt_buffer[addr] >> 8) & 0x0F; // Bits 8-11
            DWord bg = (screen.txt_buffer[addr] >> 12)& 0x0F; // bits 12-15

            // Paint the texture
            Byte pixels;
            for (unsigned y = 0; y < 8; y++) {
                pixels = font[c*8 + y];
                for (unsigned x = 0; x < 8; x++) {
                    addr = x + col*8 + ( 40*8 * (y + row*8) ); // Addres of the
                                                               // pixel in the
                                                               // buffer
                    if ( ( pixels & ( 1 << (7-x) ) ) != 0 ) {
                        // Active, uses the Ink (fg)
                        texture[addr] = PALETTE[fg];
                    }
                    else {
                        // Unactive, uses the Paper (bg)
                        texture[addr] = PALETTE[bg];
                    }
                }
            }
        }
    } // End for

    if ( screen.cursor) {
        if (frames++ < 8) {
            // Draw the cursor only when is necesary
            if (screen.cur_start <= screen.cur_end) {
                unsigned char col = screen.cur_col;
                unsigned char row = screen.cur_row;
                DWord color = PALETTE[screen.cur_color]; // Color
                if (row < 30 && col < 40) {
                    // Paints the cursor
                    std::size_t addr = col + (WIDTH_CHARS * row);
                    for (unsigned y = screen.cur_start ; y <= screen.cur_end; y++) {
                        for (unsigned x = 0; x < 8; x++) {
                            addr = x + col*8 + ( 40*8 * (y + row*8) ); // Addres of the
                                                                       // pixel in the
                                                                       // buffer
                            texture[addr] = color;
                        }
                    }
                }
            }
        } else if (frames++ < 16) {
            // Do nothing
        } else {
            frames = 0; // Reset it
        }
    }

} // TDAtoRGBATexture

TDADev::TDADev () : buffer_ptr(0), font_ptr(0), vsync_msg(0), do_vsync(false),
                    cursor(false), blink(false) {
}

TDADev::~TDADev() {
}

void TDADev::Reset () {
    this->buffer_ptr = 0;
    this->font_ptr   = 0;
    this->vsync_msg  = 0;
    this->a          = 0;
    this->b          = 0;
    this->d          = 0;
    this->e          = 0;
    this->do_vsync   = false;
    this->cursor     = false;
    this->blink      = false;
}

void TDADev::SendCMD (Word cmd) {
    DWord tmp;

    switch (cmd) {
    case 0x0000: // Map Buffer
        tmp = ( (b << 16) | a );
        if ( tmp + TXT_BUFFER_SIZE < vcomp->RamSize() ) {
            buffer_ptr = tmp;
        }
        break;

    case 0x0001: // Map Font
        tmp = ( (b << 16) | a );
        if ( tmp + FONT_BUFFER_SIZE < vcomp->RamSize() ) {
            font_ptr = tmp;
        }
        break;

    case 0x0002: // Set Int
        vsync_msg = a;
        break;

    default:
        break;
    } // switch
}     // SendCMD

bool TDADev::DoesInterrupt(Word& msg) {
    if (do_vsync && vsync_msg != 0x0000) {
        msg = vsync_msg;
        return true;
    }
    return false;
}

void TDADev::IACK () {
    do_vsync = false; // Acepted, so we can forgot now of sending it again
}

bool TDADev::IsSyncDev() const {
    return false;
}

void TDADev::GetState (void* ptr, std::size_t& size) const {
    if ( ptr != nullptr && size >= sizeof(TDAState) ) {
        auto state = (TDAState*) ptr;
        state->buffer_ptr = this->buffer_ptr;
        state->font_ptr   = this->font_ptr;
        state->vsync_msg  = this->vsync_msg;
        state->a          = this->a;
        state->b          = this->b;
        state->d          = this->d;
        state->e          = this->e;

        state->do_vsync = this->do_vsync;
    }
} // GetState

bool TDADev::SetState (const void* ptr, std::size_t size) {
    if ( ptr != nullptr && size >= sizeof(TDAState) ) {
        // Sanity check
        auto state = (const TDAState*) ptr;
        this->buffer_ptr = state->buffer_ptr;
        this->font_ptr   = state->font_ptr;
        this->vsync_msg  = state->vsync_msg;
        this->a          = state->a;
        this->b          = state->b;
        this->d          = state->d;
        this->e          = state->e;

        this->do_vsync = state->do_vsync;

        return true;
    }

    return false;
} // SetState

} // End of namespace tda
} // End of namespace computer
} // End of namespace trillek
