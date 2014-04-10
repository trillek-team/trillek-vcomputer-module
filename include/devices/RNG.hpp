#pragma once
/**
 * Trillek Virtual Computer - RNG.hpp
 * Implementation of embedded PRNG device
 */

#include "Types.hpp"
#include "AddrListener.hpp"

#include <random>

namespace vm {

  class RNG : public AddrListener{
  public:
    RNG();
    virtual ~RNG();

    virtual byte_t ReadB(dword_t addr);
    virtual word_t ReadW(dword_t addr);
    virtual dword_t ReadDW(dword_t addr);

    virtual void WriteB(dword_t addr, byte_t val);
    virtual void WriteW(dword_t addr, word_t val);
    virtual void WriteDW(dword_t addr, dword_t val);

    void Reset();

  private:
    std::uniform_int_distribution<int> distribution;
    std::mt19937 engine;
    dword_t seed;
  };

} // End of namespace vm