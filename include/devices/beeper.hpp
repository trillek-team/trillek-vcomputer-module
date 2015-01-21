/**
 * \brief       Virtual Computer embeded device Beeper
 * \file        beeper.hpp
 * \copyright   The MIT License (MIT)
 *
 */
#ifndef __BEEPER_HPP_
#define __BEEPER_HPP_ 1

#include "../types.hpp"
#include "../addr_listener.hpp"

#include <functional>

namespace trillek {
namespace computer {

/**
 * Implements a embed beeper on the Virtual Computer
 */
class Beeper : public AddrListener {
public:

	DECLDIR Beeper();
	DECLDIR virtual ~Beeper();

    virtual Byte ReadB (DWord addr);
    virtual Word ReadW (DWord addr);
    virtual DWord ReadDW (DWord addr);

    virtual void WriteB (DWord addr, Byte val);
    virtual void WriteW (DWord addr, Word val);
    virtual void WriteDW (DWord addr, DWord val);

    void Reset ();

    /**
     * /brief Assing a function to be called when Freq is changed
     * /param f_changed function to be called
     */
	DECLDIR void SetFreqChangedCB(std::function<void(DWord freq)> f_changed);

private:

    DWord freq;

    std::function<void(DWord freq)> f_changed;
};

} // End of namespace computer
} // End of namespace trillek

#endif // __BEEPER_HPP_
