/**
* Trillek Virtual Computer - RNG.cpp
* Implementation of embedded PRNG device
*/

#include "RNG.hpp"

namespace vm {

  RNG::RNG() {
    distribution = std::uniform_int_distribution<int>(0);
    engine = std::mt19937();

    seed = engine.default_seed;
    blockGenerate = false;
  }

  RNG::~RNG() {
  }

  void RNG::Reset() {
    seed = engine.default_seed;
    engine.seed(engine.default_seed);
  }

  byte_t RNG::ReadB(dword_t addr) {

    if (!blockGenerate)
      number = distribution(engine);

    switch (addr)
    {
    case 0x11E040:
      return (byte_t)(number);
    case 0x11E041:
      return (byte_t)(number >> 8);
    case 0x11E042:
      return (byte_t)(number >> 16);
    case 0x11E043:
      return (byte_t)(number >> 24);

    default:
      return 0;
    }
  }

  word_t RNG::ReadW(dword_t addr) {

    if (!blockGenerate)
      number = distribution(engine);

    switch (addr)
    {
    case 0x11E040:
      return (word_t)(number);
    case 0x11E042:
      return (word_t)(number >> 16);

    default:
      blockGenerate = true;
      word_t value = this->ReadB(addr) | (this->ReadB(addr + 1) << 8);
      blockGenerate = false;
      return value;
    }
  }

  dword_t RNG::ReadDW(dword_t addr) {

    number = distribution(engine);

    switch (addr) {
    case 0x11E040:
      return number;
      break;

    default:
      blockGenerate = true;
      dword_t value = this->ReadW(addr) | (this->ReadW(addr + 2) << 16);
      blockGenerate = false;
      return value;
    }
  }

  void RNG::WriteB(dword_t addr, byte_t val) {

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
    }

    engine.seed(seed);
  }

  void RNG::WriteW(dword_t addr, word_t val) {
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
    }

    engine.seed(seed);
  }

  void RNG::WriteDW(dword_t addr, dword_t val) {
    switch (addr) {
    case 0x11E040:
      seed = (seed & 0xFFFFFF00) | val << 0;
      break;

    default:
      this->WriteW(addr, val);
      this->WriteW(addr + 2, val >> 16);
    }

    engine.seed(seed);
  }

} // End of namespace vm
