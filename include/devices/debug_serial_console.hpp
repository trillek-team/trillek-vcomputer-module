/**
 * \brief       Debug Serial Console
 * \file        debug_serial_console.hpp
 * \copyright   The MIT License (MIT)
 *
 * Debug Serial Console for debuing the Virtual Computer
 */
#ifndef __DEBUGSERIALCONSOLE_HPP_
#define __DEBUGSERIALCONSOLE_HPP_ 1

#include "types.hpp"
#include "vcomputer.hpp"

#include <functional>

namespace trillek {
namespace computer {

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
        a       = 0;
        int_msg = 0;
        do_int  = false;
    }

    bool DoesInterrupt (Word& msg) {
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
    virtual void SendCMD (Word cmd) {
        switch (cmd) {
        case 0x0000: // READ_WORD
            if (onRead != nullptr) {
                a = onRead();
            }
            else {
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
        } // switch
    }     // SendCMD

    virtual void A (Word val) {
        a = val;
    }

    virtual Word A () {
        return a;
    }

    /**
     * Device Type
     */
    virtual Byte DevType() const {
        return 0x02;
    }

    /**
     * Device SubType
     */
    virtual Byte DevSubType() const {
        return 0xFF;
    }

    /**
     * Device ID
     */
    virtual Byte DevID() const {
        return 0x01;
    }

    /**
     * Device Vendor ID
     */
    virtual DWord DevVendorID() const {
        return 0x00000000;
    }

    virtual void GetState (void* ptr, std::size_t& size) const {
    }

    virtual bool SetState (const void* ptr, std::size_t size) {
        return true;
    }

    // Extenal API

    /**
     * Asigns a Callback callable element executed when software ask to the
     * device for a new wordt to read
     * @param cb callable element that returns a Word
     */
    void OnRead (std::function<Word()> cb) {
        this->onRead = cb;
    }

    /**
     * Asigns a Callback callable element executed when software sends to the
     * device a new wordt
     * @param cb callable element that gets a Word
     */
    void OnWrite (std::function<void(Word)> cb) {
        this->onWrite = cb;
    }

    /**
     * Sends a interrupt when the external code needs to indicate to the
     * software that there is a word ready to be read
     */
    void RX_Ready() {
        do_int = int_msg != 0x0000;
    }

protected:

    Word a;

    Word int_msg;
    bool do_int;

    std::function<Word()> onRead;      /// Callback when the computer try to
                                         // read a byte from the serial console
    std::function<void(Word)> onWrite; /// Callback when the computer try to
                                         // write a byte to the serial console
};

} // End of namespace computer
} // End of namespace trillek

#endif // __DEBUGSERIALCONSOLE_HPP_
