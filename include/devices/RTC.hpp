#pragma once
/**
* Trillek Virtual Computer - RTC.hpp
* Implementation of embedded RTC device
*/

#include "Types.hpp"
#include "AddrListener.hpp"

#include <ctime>

namespace vm {

  class RTC : public AddrListener{
  public:
    virtual byte_t ReadB(dword_t addr);
    virtual word_t ReadW(dword_t addr);
    virtual dword_t ReadDW(dword_t addr);

    virtual void WriteB(dword_t addr, byte_t val);
    virtual void WriteW(dword_t addr, word_t val);
    virtual void WriteDW(dword_t addr, dword_t val);

  private:
    static const int EPOCH_YEAR_OFFSET = 1900 + 0;
  };

} // End of namespace vm
