/**
 * \brief       Virtual Computer Random Number Generator
 * \file        rng.hpp
 * \copyright   The MIT License (MIT)
 *
 * Implementation of embedded RNG device
 */
#ifndef __RNG_HPP_
#define __RNG_HPP_ 1

#include "../types.hpp"
#include "../addr_listener.hpp"

#include <random>

namespace trillek {
namespace computer {

class RNG : public AddrListener {
public:

    RNG();
    virtual ~RNG();

    virtual Byte ReadB (DWord addr);
    virtual Word ReadW (DWord addr);
    virtual DWord ReadDW (DWord addr);

    virtual void WriteB (DWord addr, Byte val);
    virtual void WriteW (DWord addr, Word val);
    virtual void WriteDW (DWord addr, DWord val);

    void Reset ();

private:

    std::uniform_int_distribution<int> distribution;
    std::mt19937 engine;
    DWord seed;
    bool blockGenerate;
    DWord number;
};

} // End of namespace computer
} // End of namespace trillek

#endif // __RNG_HPP_
