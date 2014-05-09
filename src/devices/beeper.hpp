/**
 * \brief       Virtual Computer embeded device Beeper
 * \file        beeper.hpp
 * \copyright   The MIT License (MIT)
 *
 */
#ifndef __BEEPER_HPP_
#define __BEEPER_HPP_ 1

#include "types.hpp"
#include "addr_listener.hpp"

#include <functional>

namespace vm {

/**
 * Implements a embed beeper on the Virtual Computer
 */
class Beeper : public AddrListener {
public:

    Beeper();
    virtual ~Beeper();

    virtual byte_t ReadB (dword_t addr);
    virtual word_t ReadW (dword_t addr);
    virtual dword_t ReadDW (dword_t addr);

    virtual void WriteB (dword_t addr, byte_t val);
    virtual void WriteW (dword_t addr, word_t val);
    virtual void WriteDW (dword_t addr, dword_t val);

    void Reset ();

    /**
     * /brief Assing a function to be called when Freq is changed
     * /param f_changed function to be called
     */
    void SetFreqChangedCB (std::function<void(dword_t freq)> f_changed);

private:

    dword_t freq;

    std::function<void(dword_t freq)> f_changed;
};
} // End of namespace vm

#endif // __BEEPER_HPP_
