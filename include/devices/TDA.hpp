#pragma once
/**
 * Trillek Virtual Computer - TDA.hpp
 * Nya Elektriska Text Display Adapter
 * @see https://github.com/trillek-team/trillek-computer/blob/master/TDA.md
 */

#ifndef __TDA_DEV_HPP__
#define __TDA_DEV_HPP__ 1

#include "Types.hpp"
#include "VComputer.hpp"

namespace vm {
  namespace dev {
    namespace tda {

      static const unsigned WIDTH_CHARS     = 40; /// Width of the screen in Characters
      static const unsigned HEIGHT_CHARS    = 30; /// Height of the screen in Characters

      static const unsigned TXT_BUFFER_SIZE   = WIDTH_CHARS*HEIGHT_CHARS*2;
      static const unsigned FONT_BUFFER_SIZE  = 256*8;
      static const unsigned TEXTURE_SIZE      = WIDTH_CHARS*HEIGHT_CHARS*8*8; /// Texture size in total pixels!

      static const dword_t PALETTE[] = {  /// Default color palette
        #include "devices/ROMPalette.inc"
      };

      static const byte_t ROM_FONT[256*8] = {  /// Default font
        #include "devices/TDAfont.inc"
      };

      /**
       * Structure to store a snapshot of the device state
       */
      struct TDAState {
        public:

          word_t txt_buffer[WIDTH_CHARS*HEIGHT_CHARS];
          byte_t font_buffer[FONT_BUFFER_SIZE];

          dword_t buffer_ptr;
          dword_t font_ptr;
          word_t vsync_msg;
          word_t a, b;

          bool do_vsync;
      };

      /**
       * Generates/Updates a RGBA texture (4 byte per pixel) of the screen state
       * @param state Copy of the state of the TDA card
       * @param texture Ptr. to the texture. Must have a size enought to containt a 320x240 RGBA8 texture.
       */
      void TDAtoRGBATexture (const TDAState& state, dword_t* texture);

      /**
       * Text Generator Adapter
       * Text only video card
       */
      class TDADev : public IDevice {
        public:

          TDADev () : buffer_ptr(0), font_ptr(0), vsync_msg(0), do_vsync(false) {
          }

          virtual ~TDADev() {
          }

          /**
           * Sends (writes to CMD register) a command to the device
           * @param cmd Command value to send
           */
          virtual void SendCMD (word_t cmd) {
            dword_t tmp;
            switch (cmd) {
              case 0x0000: // Map Buffer
                tmp = ((b << 16) | a);
                if (tmp + TXT_BUFFER_SIZE < vcomp->RamSize()) {
                  buffer_ptr = tmp;
                }
                break;

              case 0x0001: // Map Font
                tmp = ((b << 16) | a);
                if (tmp + FONT_BUFFER_SIZE < vcomp->RamSize()) {
                  font_ptr = tmp;
                }
                break;

              case 0x0002: // Set Int
                vsync_msg = a;
                break;

              default:
                break;
            }
          }

          virtual void A (word_t val) { a = val; }
          virtual void B (word_t val) { b = val; }

          virtual word_t A () { return a; }
          virtual word_t B () { return b; }

          /**
           * Device Type
           */
          virtual byte_t DevType() const {
            return 0x0E; // Graphics device
          }

          /**
           * Device SubType
           */
          virtual byte_t DevSubType() const {
            return 0x01; // TDA compatible
          }

          /**
           * Device ID
           */
          virtual byte_t DevID() const {
            return 0x01; // Nya Elesktriska TDA
          }

          /**
           * Device Vendor ID
           */
          virtual dword_t DevVendorID() const {
            return 0x1C6C8B36; // Nya Elekstrika
          }

          virtual bool DoesInterrupt(word_t& msg) {
            if (do_vsync && vsync_msg != 0x0000) {
              msg = vsync_msg;
              return true;
            }
            return false;
          }

          virtual void IACK () {
            do_vsync = false; // Acepted, so we can forgot now of sending it again
          }

          virtual void GetState (void* ptr, std::size_t& size) const {
            if (ptr != nullptr && size >= sizeof(TDAState)) {
              auto state = (TDAState*) ptr;
              state->buffer_ptr = this->buffer_ptr;
              state->font_ptr   = this->font_ptr;
              state->vsync_msg  = this->vsync_msg;
              state->a          = this->a;
              state->b          = this->b;

              state->do_vsync   = this->do_vsync;

              if (this->buffer_ptr != 0 ) {
                // Copy TXT_BUFFER
                // TODO Improve this
                for (unsigned i=0; i < TXT_BUFFER_SIZE ; i += 2) {
                  state->txt_buffer[i>>2] = vcomp->ReadW(this->buffer_ptr + i);
                }
              }

              if (this->font_ptr != 0 ) {
                // Copy FONT_BUFFER
                // TODO Improve this
                for (unsigned i=0; i < FONT_BUFFER_SIZE ; i += 1) {
                  state->font_buffer[i] = vcomp->ReadB(this->font_ptr + i);
                }
              }
            }
          }

          virtual bool SetState (const void* ptr, std::size_t size) {
            if (ptr != nullptr && size >= sizeof(TDAState)) { // Sanity check
              auto state = (const TDAState*) ptr;
              this->buffer_ptr  = state->buffer_ptr;
              this->font_ptr    = state->font_ptr;
              this->vsync_msg   = state->vsync_msg;
              this->a           = state->a;
              this->b           = state->b;

              this->do_vsync    = state->do_vsync;

              if (this->buffer_ptr != 0 &&
                  this->buffer_ptr + TXT_BUFFER_SIZE < vcomp->RamSize() ) {
                // Copy TXT_BUFFER
                // TODO Improve this
                for (unsigned i=0; i < TXT_BUFFER_SIZE ; i += 2) {
                  vcomp->WriteW(this->buffer_ptr + i, state->txt_buffer[i]);
                }
              }

              if (this->font_ptr != 0 &&
                  this->font_ptr + FONT_BUFFER_SIZE < vcomp->RamSize() ) {
                // Copy FONT_BUFFER
                // TODO Improve this
                for (unsigned i=0; i < FONT_BUFFER_SIZE ; i += 1) {
                  vcomp->WriteB(this->font_ptr + i, state->font_buffer[i]);
                }
              }

              return true;
            }

            return false;
          }


          /* API exterior to the Virtual Computer */

          /**
           * Generate a VSync interrupt if is enabled
           */
          void DoVSync() {
            do_vsync = (vsync_msg != 0x0000);
          }

        protected:
          dword_t buffer_ptr;
          dword_t font_ptr;
          word_t vsync_msg;
          word_t a, b;

          bool do_vsync;
      };


    } // End of namespace tda
  } // End of namespace dev
} // End of namespace vm

#endif

