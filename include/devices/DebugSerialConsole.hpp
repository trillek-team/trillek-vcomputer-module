#pragma once
/**
 * Trillek Virtual Computer - DebugSerialConsole.hpp
 * Debug Serial Console for debuing the Virtual Computer
 */

#include "Types.hpp"
#include "VComputer.hpp"

namespace vm {
  namespace dev {

    /**
     * Serial Console for debuing
     */
    class DebugSerialConsole : public IDevice {
      public:

        DebugSerialConsole () {
        }

        virtual ~DebugSerialConsole () {
        }

        virtual void Reset () {
          a = 0;
          int_msg = 0;
          do_int = false;
        }

        bool DoesInterrupt (word_t& msg) {
          if (do_int && int_msg != 0x0000) {
            msg = int_msg;
            return true;
          }
          return false;
        }

        void IACK () {
          do_int = false; // Acepted, so we can forgot now of sending it again
        }

        /**
         * Sends (writes to CMD register) a command to the device
         * @param cmd Command value to send
         */
        virtual void SendCMD (word_t cmd) {
          switch (cmd) {
            case 0x0000: // READ_WORD
              if (onRead != nullptr) {
                a = onRead();
              } else {
                a = 0;
              }
              break;

            case 0x0001: // SEND_WORD
              if (onWrite != nullptr) {
                onWrite(a);
              }
              break;

            case 0x0002: // SET_RXINT
              int_msg = a;
              break;

            default:
              break;

          }
        }

        virtual void A (word_t val) {
          a = val;
        }

        virtual word_t A () {
          return a;
        }

        /**
         * Device Type
         */
        virtual byte_t DevType() const {
          return 0x02;
        }

        /**
         * Device SubType
         */
        virtual byte_t DevSubType() const {
          return 0xFF;
        }

        /**
         * Device ID
         */
        virtual byte_t DevID() const {
          return 0x01;
        }

        /**
         * Device Vendor ID
         */
        virtual dword_t DevVendorID() const {
          return 0x00000000;
        }

        virtual void GetState (void* ptr, std::size_t& size) const {
        }

        virtual bool SetState (const void* ptr, std::size_t size) {
          return true;
        }

      protected:
        word_t a;

        word_t int_msg;
        bool do_int;

        std::function<word_t()> onRead;     /// Callback when the computer try to read a byte from the serial console
        std::function<void(word_t)> onWrite;/// Callback when the computer try to write a byte to the serial console
    };

  } // End of namespace dev
} // End of namespace vm

