/**
 * \brief       Virtual Computer NVRAM
 * \file        rng.cpp
 * \copyright   LGPL v3
 *
 * Implementation of No Volatile RAM (NVRAM)
 */

#include "devices/nvram.hpp"
#include "vs_fix.hpp"

#include <exception>
#include <cassert>

namespace trillek {
namespace computer {

NVRAM::NVRAM() : dirty(false) {
}

NVRAM::~NVRAM() {
}

void NVRAM::Reset() {
}

Byte NVRAM::ReadB(DWord addr) {

    if (addr < BaseAddress || addr >= BaseAddress + 256) {
        return 0;
    }

    addr -= BaseAddress;
    assert (addr < 256);

    return eprom[addr];

}     // ReadB

Word NVRAM::ReadW(DWord addr) {

    if (addr < BaseAddress || addr >= BaseAddress + 256) {
        return 0;
    }

    addr -= BaseAddress;
    assert (addr < 256);

    if (addr == 255) {
        return eprom[addr];
    } else {
        return eprom[addr] | (eprom[addr+1] << 8);
    }

}     // ReadW

DWord NVRAM::ReadDW(DWord addr) {

    if (addr < BaseAddress || addr >= BaseAddress + 256) {
        return 0;
    }

    addr -= BaseAddress;
    assert (addr < 256);

    if (addr == 253) {
        return eprom[addr] | (eprom[addr+1] << 8) | (eprom[addr+2] << 16);
    } else if (addr == 254) {
        return eprom[addr] | (eprom[addr+1] << 8) ;
    } else if (addr == 255) {
        return eprom[addr];
    } else {
        return eprom[addr] | (eprom[addr+1] << 8) | (eprom[addr+2] << 16) | (eprom[addr+2] << 24);
    }

}     // ReadDW

void NVRAM::WriteB(DWord addr, Byte val) {

    if (addr < BaseAddress || addr >= BaseAddress + 256) {
        return ;
    }

    addr -= BaseAddress;
    assert (addr < 256);

    eprom[addr] = val;
    dirty = true;
} // WriteB

void NVRAM::WriteW(DWord addr, Word val) {
    if (addr < BaseAddress || addr >= BaseAddress + 256) {
        return ;
    }

    addr -= BaseAddress;
    assert (addr < 256);

    if (addr == 255) {
        eprom[addr] = val;
    } else {
        eprom[addr] = val;
        eprom[addr+1] = val >> 8;
    }
    dirty = true;
} // WriteW

void NVRAM::WriteDW(DWord addr, DWord val) {
    if (addr < BaseAddress || addr >= BaseAddress + 256) {
        return ;
    }

    addr -= BaseAddress;
    assert (addr < 256);

    if (addr == 253) {
        eprom[addr] = val;
        eprom[addr+1] = val >> 8;
        eprom[addr+2] = val >> 16;
    } else if (addr == 254) {
        eprom[addr] = val;
        eprom[addr+1] = val >> 8;
    } else if (addr == 255) {
        eprom[addr] = val;
    } else {
        eprom[addr] = val;
        eprom[addr+1] = val >> 8;
        eprom[addr+2] = val >> 16;
        eprom[addr+3] = val >> 24;
    }
    dirty = true;
} // WriteDW

bool NVRAM::isDirty() {
    return dirty;
}

bool NVRAM::Load (std::istream& stream) {
    if (stream.good() && ! stream.eof()) {
        try {
            stream.read(reinterpret_cast<char*>(eprom), 256);
        } catch (std::exception&) {
            return false;
        }
        dirty = false;
        return stream.gcount() == 256;
    }
    return false;
} // Load

bool NVRAM::Save (std::ostream& stream) {
    if (stream.good() && ! stream.eof()) {
        try {
            stream.write(reinterpret_cast<char*>(eprom), 256);
        } catch (std::exception&) {
            return false;
        }
        dirty = false;
        return true;
    }
    return false;
}

} // End of namespace computer
} // End of namespace trillek

