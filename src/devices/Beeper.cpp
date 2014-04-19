/*!
 * \brief       Virtual Computer embeded device Beeper
 * \file        Beeper.hpp
 * \copyright   The MIT License (MIT)
 *
 */

#include "Beeper.hpp"

namespace vm {

    Beeper::Beeper () : freq(0), f_changed(nullptr) {
    }

    Beeper::~Beeper () {
    }

    byte_t Beeper::ReadB (dword_t addr) {
        if (addr == 0x11E020) { // LSB
            return freq;
        } else { // MSB
            return freq >> 8;
        }
    }

    word_t Beeper::ReadW (dword_t addr) {
        if (addr == 0x11E020) {
            return freq;
        } else { // Only MSB
            return freq >> 8;
        }
    }

    dword_t Beeper::ReadDW (dword_t addr) {
        if (addr == 0x11E020) {
            return freq;
        } else { // Only MSB
            return freq >> 8;
        }
    }

    void Beeper::WriteB (dword_t addr, byte_t val) {
        if (addr == 0x11E020) { // LSB
            freq &= 0xFF00;
            freq |= val;
        } else { // MSB
            freq &= 0x00FF;
            freq |= val << 8;
        }

        if (f_changed) {
            f_changed(freq);
        }
    }

    void Beeper::WriteW (dword_t addr, word_t val) {
        if (addr == 0x11E020) {
            freq = val;
        } else { // Only affects MSB
            freq &= 0x00FF;
            freq |= (val & 0xFF) << 8;
        }

        if (f_changed) {
            f_changed(freq);
        }
    }

    void Beeper::WriteDW (dword_t addr, dword_t val) {
        if (addr == 0x11E020) {
            freq = val;
        } else { // Only affects MSB
            freq &= 0x00FF;
            freq |= (val & 0xFF) << 8;
        }

        if (f_changed) {
            f_changed(freq);
        }
    }

    void Beeper::Reset () {
        freq = 0;
        if (f_changed) {
            f_changed(0);
        }
    }

    void Beeper::SetFreqChangedCB (std::function<void(dword_t freq)> f_changed) {
        this->f_changed = f_changed;
    }

} // End of namespace vm

