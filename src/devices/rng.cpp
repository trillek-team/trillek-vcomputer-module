/**
 * \brief       Virtual Computer Random Number Generator
 * \file        rng.cpp
 * \copyright   LGPL v3
 *
 * Implementation of embedded RNG device
 */

#include "devices/rng.hpp"
#include "vs_fix.hpp"

namespace trillek {
namespace computer {

RNG::RNG() {
    distribution = std::uniform_int_distribution<int>(0);
    engine       = std::mt19937();

    seed          = engine.default_seed;
    blockGenerate = false;
}

RNG::~RNG() {
}

void RNG::Reset() {
    seed = engine.default_seed;
    engine.seed(engine.default_seed);
}

Byte RNG::ReadB(DWord addr) {

    if (!blockGenerate) {
        number = distribution(engine);
    }

    switch (addr)
    {
    case 0x11E040:
        return (Byte)(number);

    case 0x11E041:
        return (Byte)(number >> 8);

    case 0x11E042:
        return (Byte)(number >> 16);

    case 0x11E043:
        return (Byte)(number >> 24);

    default:
        return 0;
    } // switch
}     // ReadB

Word RNG::ReadW(DWord addr) {

    if (!blockGenerate) {
        number = distribution(engine);
    }

    switch (addr)
    {
    case 0x11E040:
        return (Word)(number);

    case 0x11E042:
        return (Word)(number >> 16);

    default:
        blockGenerate = true;
        Word value = this->ReadB(addr) | (this->ReadB(addr + 1) << 8);
        blockGenerate = false;
        return value;
    } // switch
}     // ReadW

DWord RNG::ReadDW(DWord addr) {

    number = distribution(engine);

    switch (addr) {
    case 0x11E040:
        return number;
        break;

    default:
        blockGenerate = true;
        DWord value = this->ReadW(addr) | (this->ReadW(addr + 2) << 16);
        blockGenerate = false;
        return value;
    } // switch
}     // ReadDW

void RNG::WriteB(DWord addr, Byte val) {

    switch (addr)
    {
    case 0x11E040:
        seed = (seed & 0xFFFFFF00) | val << 0;
        break;

    case 0x11E041:
        seed = (seed & 0xFFFF00FF) | val << 8;
        break;

    case 0x11E042:
        seed = (seed & 0xFF00FFFF) | val << 16;
        break;

    case 0x11E043:
        seed = (seed & 0x00FFFFFF) | val << 24;
        break;

    default:
        return;
    } // switch

    engine.seed(seed);
} // WriteB

void RNG::WriteW(DWord addr, Word val) {
    switch (addr) {
    case 0x11E040:
        seed = (seed & 0xFFFFFF00) | val << 0;
        break;

    case 0x11E042:
        seed = (seed & 0xFF00FFFF) | val << 16;
        break;

    default:
        this->WriteB(addr, val);
        this->WriteB(addr + 1, val >> 8);
    } // switch

    engine.seed(seed);
} // WriteW

void RNG::WriteDW(DWord addr, DWord val) {
    switch (addr) {
    case 0x11E040:
        seed = (seed & 0xFFFFFF00) | val << 0;
        break;

    default:
        this->WriteW(addr, val);
        this->WriteW(addr + 2, val >> 16);
    }

    engine.seed(seed);
} // WriteDW

} // End of namespace computer
} // End of namespace trillek
