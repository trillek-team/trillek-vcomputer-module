/**
 * \brief       Virtual Computer Text Display Adapter
 * \file        tda.hpp
 * \copyright   The MIT License (MIT)
 *
 * Nya Elektriska Text Display Adapter
 * @see https://github.com/trillek-team/trillek-computer/blob/master/TDA.md
 */
#ifndef __TDA_HPP_
#define __TDA_HPP_ 1

#include "types.hpp"
#include "vcomputer.hpp"

#include <algorithm>
#include <cstdio>

namespace trillek {
namespace computer {
namespace tda {

static const unsigned WIDTH_CHARS  = 40; /// Width of the screen in Characters
static const unsigned HEIGHT_CHARS = 30; /// Height of the screen in Characters

static const unsigned TXT_BUFFER_SIZE  = WIDTH_CHARS*HEIGHT_CHARS*2;
static const unsigned FONT_BUFFER_SIZE = 256*8;
/// Texture size in total pixels!
static const unsigned TEXTURE_SIZE     = WIDTH_CHARS*HEIGHT_CHARS*8*8;

static const DWord PALETTE[] = {
    /// Default color palette
        #include "devices/rom_palette.inc"
};

static const Byte ROM_FONT[256*8] = { /// Default font
        #include "devices/tda_font.inc"
};

/**
 * Structure to store a snapshot of the device state
 */
struct TDAState {
public:

    Word txt_buffer[WIDTH_CHARS*HEIGHT_CHARS];
    Byte font_buffer[FONT_BUFFER_SIZE];

    DWord buffer_ptr;
    DWord font_ptr;
    Word vsync_msg;
    Word a, b, d, e;

    bool do_vsync;
};

/**
 * Structure to store a snapshot TDA computer screen
 */
struct TDAScreen {
public:

    Word txt_buffer[WIDTH_CHARS*HEIGHT_CHARS];
    Byte font_buffer[FONT_BUFFER_SIZE];
    bool user_font;

    bool cursor;    /// Cursor enabled ?
    bool cursor_blink;  /// Cursor blinking ?
    Byte cur_col;   // Cursor position
    Byte cur_row;
    Byte cur_color; /// Cursor color
    Byte cur_start; /// Start scanline
    Byte cur_end;   /// End scnaline
};

/**
 * Generates/Updates a RGBA texture (4 byte per pixel) of the screen state
 * @param state Copy of the state of the TDA card
 * @param texture Ptr. to the texture. Must have a size enought to containt a
 **320x240 RGBA8 texture.
 * @param frames Frames counter. Used to handle blinking
 */
void TDAtoRGBATexture (const TDAScreen& screen, DWord* texture, unsigned& frames);
/**
 * Generates/Updates a RGBA texture (4 byte per pixel) of the screen state
 * @param state Copy of the state of the TDA card
 * @param texture Ptr. to the texture. Must have a size enought to containt a
 **320x240 RGBA8 texture.
 */
void TDAtoRGBATexture (const TDAScreen& screen, DWord* texture) {
    static unsigned frames = 0;
    TDAtoRGBATexture (screen, texture, frames);
}

/**
 * Text Generator Adapter
 * Text only video card
 */
class TDADev : public IDevice {
public:

    TDADev ();

    virtual ~TDADev();

    virtual void Reset ();

    /**
     * Sends (writes to CMD register) a command to the device
     * @param cmd Command value to send
     */
    virtual void SendCMD (Word cmd);

    virtual void A (Word val) {
        a = val;
    }

    virtual void B (Word val) {
        b = val;
    }

    virtual void D (Word val) {
        d = val;
        cursor = (d & 0x80) != 0; // bit 7
        blink  = (d & 0x40) != 0; // bit 6
    }

    virtual void E (Word val) {
        e = val;
    }

    virtual Word A () {
        return a;
    }

    virtual Word B () {
        return b;
    }

    virtual Word D () {
        return d;
    }

    virtual Word E () {
        return e;
    }

    /**
     * Device Type
     */
    virtual Byte DevType() const {
        return 0x0E; // Graphics device
    }

    /**
     * Device SubType
     */
    virtual Byte DevSubType() const {
        return 0x01; // TDA compatible
    }

    /**
     * Device ID
     */
    virtual Byte DevID() const {
        return 0x01; // Nya Elesktriska TDA
    }

    /**
     * Device Vendor ID
     */
    virtual DWord DevVendorID() const {
        return 0x1C6C8B36; // Nya Elekstrika
    }

    virtual bool DoesInterrupt (Word& msg);

    virtual void IACK ();

    virtual void GetState (void* ptr, std::size_t& size) const;

    virtual bool SetState (const void* ptr, std::size_t size);

    virtual bool IsSyncDev() const;

    // API exterior to the Virtual Computer (affects or afected by stuff outside
    // of the computer)

    /**
     * Does a dump of the TDA screen ram
     * @param screen Structure TDAScreen were store the dump
     */
    void DumpScreen (TDAScreen& screen) const {
        // Copy TEXT_BUFFER
        if ( this->buffer_ptr != 0 &&
             this->buffer_ptr + TXT_BUFFER_SIZE < vcomp->RamSize() ) {
            auto orig = &(vcomp->Ram()[this->buffer_ptr]);
            std::copy_n(orig, TXT_BUFFER_SIZE, (Byte*)screen.txt_buffer);
        }

        screen.user_font = false;
        // Copy FONT_BUFFER
        if ( this->font_ptr != 0 &&
             this->font_ptr + FONT_BUFFER_SIZE < vcomp->RamSize() ) {
            auto orig = &(vcomp->Ram()[this->font_ptr]);
            std::copy_n(orig, FONT_BUFFER_SIZE, (Byte*)screen.font_buffer);
            screen.user_font = true;
        }

        screen.cursor    = this->cursor;
        screen.cursor_blink = this->blink;
        screen.cur_row   = (Byte)(this->e >> 8);
        screen.cur_col   = (Byte) this->e;
        screen.cur_start = (Byte) this->d & 0x7;
        screen.cur_end   = (Byte)(this->d & 0x38) >> 3;
        screen.cur_color = (Byte)((this->d & 0xF000) >> 12);

    } // DumpScreen

    /**
     * Generate a VSync interrupt if is enabled
     * And handles the cursor blink state
     */
    void DoVSync() {
        do_vsync = (vsync_msg != 0x0000);
    }

protected:

    DWord buffer_ptr;
    DWord font_ptr;
    Word vsync_msg;
    Word a, b, d, e;

    bool do_vsync;

    bool cursor;        /// Cursor enabled ?
    bool blink;         /// Blink enabled ?
};


} // End of namespace tda
} // End of namespace computer
} // End of namespace trillek

#endif // __TDA_HPP_
