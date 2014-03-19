/**
* Trillek Virtual Computer - DCPU16N.cpp
* Implementation of the DCPU-16N CPU
*/

#include "DCPU16N.hpp"

namespace vm {
  namespace cpu {

    DCPU16N::DCPU16N(unsigned clock) : ICPU(), cpu_clock(clock) {
      this->Reset();
    }

    DCPU16N::~DCPU16N() {}

    void DCPU16N::Reset()
    {
      int i;

      // Generic initialization
      std::fill_n(r, 8, 0);
      pc = 0;
      sp = 0;
      ex = 0;
      ia = 0;
      for(i = 0; i < 16; i++) emu[i] = i << 12;
      iqp = 0;
      iqc = 0;
      madraw = 0;
      phase = 0;
      acu = 0;
      bytemode = false;
      bytehigh = false;
      skip = false;
      fire = false;

      // point EMU at ROM (page 0x100)
      emu[0] = 0x00100000;

    }
    unsigned DCPU16N::Step()
    {
      unsigned x = 0;
      do {
        Tick(1);
        x++;
      } while((!fire) && (phase != 0));
      return x;
    }
    void DCPU16N::Tick(unsigned n)
    {

    }
    bool DCPU16N::SendInterrupt(word_t msg)
    {
      return true;
    }

    word_t DCPU16N::IORead(word_t addr)
    {
      return vcomp->ReadW(0x00110000 | addr);
    }
    void DCPU16N::IOWrite(word_t addr, word_t v)
    {
      vcomp->WriteW(0x00110000 | addr, v);
    }

    void DCPU16N::GetState(void* ptr, std::size_t& size) const
    {
      size = 0;
    }

    bool DCPU16N::SetState(const void* ptr, std::size_t size)
    {
      return false;
    }
  }
}
