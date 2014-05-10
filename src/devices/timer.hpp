/**
 * \brief       Virtual Computer Timer
 * \file        timer.hpp
 * \copyright   The MIT License (MIT)
 *
 * Implementation of Timer embed device
 */
#ifndef __TIMER_HPP_
#define __TIMER_HPP_ 1

#include "types.hpp"
#include "addr_listener.hpp"

namespace vm {

class Timer : public AddrListener {

public:

    Timer ();
    virtual ~Timer();

    virtual Byte ReadB (DWord addr);
    virtual Word ReadW (DWord addr);
    virtual DWord ReadDW (DWord addr);

    virtual void WriteB (DWord addr, Byte val);
    virtual void WriteW (DWord addr, Word val);
    virtual void WriteDW (DWord addr, DWord val);

    void Reset ();

    /**
     * Executes N Device clock cycles.
     * Here resides the code that is executed every Device Clock tick.
     * @param n Number of clock ticks executing
     * @param delta Number milliseconds since the last call
     */
    void Tick (unsigned n = 1, const double delta = 0);

    /**
     * Checks if the device is trying to generate an interrupt
     * @param msg The interrupt message will be writen here
     * @return True if is generating a new interrupt
     */
    bool DoesInterrupt (Word& msg);

    /**
     * Informs to the device that his generated interrupt was accepted by the
     **CPU
     */
    void IACK ();

    DWord tmr0; /// Timer 0
    DWord tmr1; /// Timer 1

    DWord re0; /// Reload value of Timer 0
    DWord re1; /// Reload value of Timer 1

    Byte cfg; /// Config byte of PIT

    bool do_int_tmr0; /// Try to thorow interrupt of TMR0 ?
    bool do_int_tmr1; /// Try to thorow interrupt of TMR1 ?
};
} // End of namespace vm

#endif // __TIMER_HPP_
