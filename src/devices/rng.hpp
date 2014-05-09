/**
 * \brief       Virtual Computer Random Number Generator
 * \file        rng.hpp
 * \copyright   The MIT License (MIT)
 *
 * Implementation of embedded RNG device
 */
#ifndef __RNG_HPP_
#define __RNG_HPP_ 1

#include "types.hpp"
#include "addr_listener.hpp"

#include <random>

namespace vm {

class RNG : public AddrListener {
public:

    RNG();
    virtual ~RNG();

    virtual byte_t ReadB (dword_t addr);
    virtual word_t ReadW (dword_t addr);
    virtual dword_t ReadDW (dword_t addr);

    virtual void WriteB (dword_t addr, byte_t val);
    virtual void WriteW (dword_t addr, word_t val);
    virtual void WriteDW (dword_t addr, dword_t val);

    void Reset ();

private:

    std::uniform_int_distribution<int> distribution;
    std::mt19937 engine;
    dword_t seed;
    bool blockGenerate;
    dword_t number;
};
} // End of namespace vm

#endif // __RNG_HPP_
