/**
 * Trillek Virtual Computer - Timer.cpp
 * Implementation of Timer embed device
 */

#include "Timer.hpp"

namespace vm {

  Timer::Timer () {
  }

  Timer::~Timer () {
  }

  void Timer::Reset() {
    tmr0 = 0;
    tmr1 = 0;

    re0 = 0;
    re1 = 0;

    cfg = 0;

    do_int_tmr0 = false;
    do_int_tmr1 = false;
  }

  void Timer::Tick (unsigned n, const double delta) {

  }

  bool Timer::DoesInterrupt(word_t& msg) {
    return false;
  }

  void Timer::IACK () {

  }

  byte_t Timer::ReadB (dword_t addr) {
    switch (addr) {
      case 0x11E000:
        return tmr0;
        break;

      case 0x11E001:
        return tmr0 >> 8;
        break;

      case 0x11E002:
        return tmr0 >> 16;
        break;

      case 0x11E003:
        return tmr0 >> 24;
        break;


      case 0x11E004:
        return re0;
        break;

      case 0x11E005:
        return re0 >> 8;
        break;

      case 0x11E006:
        return re0 >> 16;
        break;

      case 0x11E007:
        return re0 >> 24;
        break;

      
      case 0x11E008:
        return tmr1;
        break;

      case 0x11E009:
        return tmr1 >> 8;
        break;

      case 0x11E00A:
        return tmr1 >> 16;
        break;

      case 0x11E00B:
        return tmr1 >> 24;
        break;


      case 0x11E00C:
        return re1;
        break;

      case 0x11E00D:
        return re1 >> 8;
        break;

      case 0x11E00E:
        return re1 >> 16;
        break;

      case 0x11E00F:
        return re1 >> 24;
        break;
      
      case 0x11E010:
        return cfg;
        break;

      default:
        return 0;
    }
  }

  word_t Timer::ReadW (dword_t addr) {
    switch (addr) {
      case 0x11E000:
        return tmr0;
        break;

      case 0x11E002:
        return tmr0 >> 16;
        break;

      case 0x11E004:
        return re0;
        break;

      case 0x11E006:
        return re0 >> 16;
        break;

      case 0x11E008:
        return tmr1;
        break;

      case 0x11E00A:
        return tmr1 >> 16;
        break;

      case 0x11E00C:
        return re1;
        break;

      case 0x11E00E:
        return re1 >> 16;
        break;

      case 0x11E010:
        return cfg;
        break;

      default:
        return this->ReadB(addr) | (this->ReadB(addr+1) << 8);
    }
  }

  dword_t Timer::ReadDW (dword_t addr) {
    switch (addr) {
      case 0x11E000:
        return tmr0;
        break;

      case 0x11E004:
        return re0;
        break;

      case 0x11E008:
        return tmr1;
        break;

      case 0x11E00C:
        return re1;
        break;

      case 0x11E010:
        return cfg;
        break;

      default:
        return this->ReadW(addr) | (this->ReadW(addr+2) << 16);
    }
  }

  void Timer::WriteB (dword_t addr, byte_t val) {
  }

  void Timer::WriteW (dword_t addr, word_t val) {
  }

  void Timer::WriteDW (dword_t addr, dword_t val) {
  }


} // End of namespace vm

