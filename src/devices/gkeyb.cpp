/**
 * \brief       Virtual Computer generic keyboard
 * \file        gkeyb.cpp
 * \copyright   LGPL v3
 *
 * Generic Western/Latin Keyboard
 * @see https://github.com/trillek-team/trillek-computer/blob/master/Keyboard.md
 */

#include "devices/gkeyb.hpp"
#include "vs_fix.hpp"

namespace trillek {
namespace computer {
namespace gkeyboard {

GKeyboardDev::GKeyboardDev () : int_msg(0), do_int(false) {
    keybuffer.clear();
}

GKeyboardDev::~GKeyboardDev() {
}

void GKeyboardDev::Reset () {
    a = 0;
    b = 0;
    c = 0;

    keybuffer.clear();

    int_msg = 0;
    do_int  = false;
}

bool GKeyboardDev::DoesInterrupt(Word& msg) {
    if (do_int && int_msg != 0x0000) {
        msg = int_msg;
        return true;
    }
    return false;
}

/**
 * Sends (writes to CMD register) a command to the device
 * @param cmd Command value to send
 */
void GKeyboardDev::SendCMD (Word cmd) {
    switch (cmd) {
    case 0x0000: // CLR_BUFFER
        keybuffer.clear();
        break;

    case 0x0001: // PULL_KEY
        if ( !keybuffer.empty() ) {
            // keyevent = ((status & 7) << 24) | (keycode << 16) | scancode;
            auto tmp = keybuffer.front();
            keybuffer.pop_front();
            c = tmp >> 24;
            b = tmp & 0xFFFF;
            a = (tmp >> 16) & 0xFF;
        }
        else {
            a = b = c = 0;
        }
        break;

    case 0x0002: // PUSH_KEY
        if (keybuffer.size() < BSIZE) {
            DWord keyevent = ( (c & 7) << 24 ) | ( (a & 0xFF) << 16 ) | b;
            keybuffer.push_front(keyevent);
        }
        break;

    case 0x0003: // SET_INT
        int_msg = a;
        break;

    default:
        break;
    } // switch
}     // SendCMD

void GKeyboardDev::IACK () {
    do_int = false; // Acepted, so we can forgot now of sending it again
}

void GKeyboardDev::GetState (void* ptr, std::size_t& size) const {
    if ( ptr != nullptr && size >= sizeof(GKeyboardState) ) {
        auto state = (GKeyboardState*) ptr;

        state->a = this->a;
        state->b = this->b;
        state->c = this->c;

        const std::deque<DWord>& tmp = this->keybuffer;
        state->keybuffer = tmp; // Copy

        state->int_msg = this->int_msg;
        state->do_int  = this->do_int;
    }
} // GetState

bool GKeyboardDev::SetState (const void* ptr, std::size_t size) {
    if ( ptr != nullptr && size >= sizeof(GKeyboardState) ) {
        // Sanity check
        auto state = (const GKeyboardState*) ptr;

        this->a = state->a;
        this->b = state->b;
        this->c = state->c;

        const std::deque<DWord>& tmp = state->keybuffer;
        this->keybuffer = tmp; // Copy

        this->int_msg = state->int_msg;
        this->do_int  = state->do_int;

        return true;
    }

    return false;
} // SetState


} // End of namespace gkeyboard
} // End of namespace computer
} // End of namespace trillek
