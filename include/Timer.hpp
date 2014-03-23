#pragma once
/**
 * Trillek Virtual Computer - Timer.hpp
 * Implementation of Timer embed device
 */

#include "Types.hpp"
#include "VComputer.hpp"

namespace vm {

  class Timer : public AddrListener {
    friend class VComputer; // Necesary to do a copy of the internal data

    public:
      Timer ();
      virtual ~Timer();
      
      virtual byte_t ReadB (dword_t addr);
      virtual word_t ReadW (dword_t addr);
      virtual dword_t ReadDW (dword_t addr);

      virtual void WriteB (dword_t addr, byte_t val);
      virtual void WriteW (dword_t addr, word_t val);
      virtual void WriteDW (dword_t addr, dword_t val);

      void Reset ();

      /**
       * Executes N Device clock cycles.
       * Here resides the code that is executed every Device Clock tick.
       * @param n Number of clock ticks executing
       * @param delta Number milliseconds since the last call
       */
      void Tick (unsigned n=1, const double delta = 0);

      /**
       * Checks if the device is trying to generate an interrupt
       * @param msg The interrupt message will be writen here
       * @return True if is generating a new interrupt
       */
      bool DoesInterrupt(word_t& msg);

      /**
       * Informs to the device that his generated interrupt was accepted by the CPU
       */
      void IACK ();

    private:
      dword_t tmr0;     /// Timer 0
      dword_t tmr1;     /// Timer 1

      dword_t re0;      /// Reload value of Timer 0
      dword_t re1;      /// Reload value of Timer 1

      byte_t cfg;       /// Config byte of PIT

      bool do_int_tmr0; /// Try to thorow interrupt of TMR0 ?
      bool do_int_tmr1; /// Try to thorow interrupt of TMR1 ?
  };

} // End of namespace vm

