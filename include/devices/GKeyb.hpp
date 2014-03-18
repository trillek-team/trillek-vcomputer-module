#pragma once
/**
 * Trillek Virtual Computer - GKeyb.hpp
 * Generic Western/Latin Kaeyboard
 * @see https://github.com/trillek-team/trillek-computer/blob/master/Keyboard.md
 */

#ifndef __GKEYBOARD_DEV_HPP__
#define __GKEYBOARD_DEV_HPP__ 1

#include "Types.hpp"
#include "VComputer.hpp"

#include <cstdio>

namespace vm {
  namespace dev {
    namespace gkeyboard {

      /**
       * Structure to store a snapshot of the device state
       */
      struct GKeyboardState {
        public:


          bool do_int;
      };

      /**
       * Keyboard mode
       */
      enum KeybMode {
        KEY,
        RAW,
        RAW_KEY
      };

      /**
       * Genertic Keyboard
       * Western / Latin generic keyboard
       */
      class GKeyboardDev : public IDevice {
        public:

          GKeyboardDev () : int_msg(0), do_int(false), mode(KeybMode::KEY) {
          }

          virtual ~GKeyboardDev() {
          }

          virtual void Reset () {
            int_msg = 0;
            a = 0;
            b = 0;
            do_int = false;
          
            mode = KeybMode::KEY;
          }

          /**
           * Sends (writes to CMD register) a command to the device
           * @param cmd Command value to send
           */
          virtual void SendCMD (word_t cmd) {
            switch (cmd) {

              case 0x0003: // SET_INT
                int_msg = a;
                break;

              case 0x0005: // SET_MODE
                mode = (KeybMode)(a & 3);
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
            return 0x03; // HID
          }

          /**
           * Device SubType
           */
          virtual byte_t DevSubType() const {
            return 0x01; // Western / Latin Keyboard
          }

          /**
           * Device ID
           */
          virtual byte_t DevID() const {
            return 0x01; //
          }

          /**
           * Device Vendor ID
           */
          virtual dword_t DevVendorID() const {
            return 0x00000000; // Generic
          }

          virtual bool DoesInterrupt(word_t& msg) {
            if (do_int && int_msg != 0x0000) {
              msg = int_msg;
              return true;
            }
            return false;
          }

          virtual void IACK () {
            do_int = false; // Acepted, so we can forgot now of sending it again
          }

          virtual void GetState (void* ptr, std::size_t& size) const {
            if (ptr != nullptr && size >= sizeof(GKeyboardState)) {
              auto state = (GKeyboardState*) ptr;
            }
          }

          virtual bool SetState (const void* ptr, std::size_t size) {
            if (ptr != nullptr && size >= sizeof(GKeyboardState)) { // Sanity check
              auto state = (const GKeyboardState*) ptr;

              return true;
            }

            return false;
          }

          /* API exterior to the Virtual Computer (affects or afected by stuff outside of the computer) */

        protected:
          word_t int_msg;
          word_t a, b;
          bool do_int;
          
          KeybMode mode;
      };


    } // End of namespace tda
  } // End of namespace dev
} // End of namespace vm

#endif

