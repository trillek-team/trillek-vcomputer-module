/*!
 * \brief       Virtual Computer embeded device Beeper
 * \file        beeper.hpp
 * \copyright   The MIT License (MIT)
 *
 */

#include "devices/beeper.hpp"
#include "vs_fix.hpp"

namespace trillek {
namespace computer {

Beeper::Beeper () : freq(0), f_changed(nullptr) {
}

Beeper::~Beeper () {
}

Byte Beeper::ReadB (DWord addr) {
    if (addr == 0x11E020) {
        // LSB
        return freq;
    }
    else {
        // MSB
        return freq >> 8;
    }
}

Word Beeper::ReadW (DWord addr) {
    if (addr == 0x11E020) {
        return freq;
    }
    else {
        // Only MSB
        return freq >> 8;
    }
}

DWord Beeper::ReadDW (DWord addr) {
    if (addr == 0x11E020) {
        return freq;
    }
    else {
        // Only MSB
        return freq >> 8;
    }
}

void Beeper::WriteB (DWord addr, Byte val) {
    if (addr == 0x11E020) {
        // LSB
        freq &= 0xFF00;
        freq |= val;
    }
    else {
        // MSB
        freq &= 0x00FF;
        freq |= val << 8;
    }

    if (f_changed) {
        f_changed(freq);
    }
} // WriteB

void Beeper::WriteW (DWord addr, Word val) {
    if (addr == 0x11E020) {
        freq = val;
    }
    else {
        // Only affects MSB
        freq &= 0x00FF;
        freq |= (val & 0xFF) << 8;
    }

    if (f_changed) {
        f_changed(freq);
    }
} // WriteW

void Beeper::WriteDW (DWord addr, DWord val) {
    if (addr == 0x11E020) {
        freq = val;
    }
    else {
        // Only affects MSB
        freq &= 0x00FF;
        freq |= (val & 0xFF) << 8;
    }

    if (f_changed) {
        f_changed(freq);
    }
} // WriteDW

void Beeper::Reset () {
    freq = 0;
    if (f_changed) {
        f_changed(0);
    }
}

void Beeper::SetFreqChangedCB (std::function<void(DWord freq)> f_changed) {
    this->f_changed = f_changed;
}

} // End of namespace computer
} // End of namespace trillek
