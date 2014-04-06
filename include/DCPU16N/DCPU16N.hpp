#pragma once
/**
* Trillek Virtual Computer - DCPU16N.hpp
* Class definitions for the DCPU-16N CPU
*/

#ifndef __DCPU16N_HPP__
#define __DCPU16N_HPP__

#include "ICPU.hpp"
#include "VComputer.hpp"

namespace vm {
  namespace cpu {

    class DCPU16N : public ICPU {
    public:
      DCPU16N(unsigned clock = 100000);
      virtual ~DCPU16N();

      /**
       * Returns CPU clock speed in Hz
       */
      virtual unsigned Clock() {
        return this->cpu_clock;
      }

      /**
       * Resets the CPU state
       */
      virtual void Reset();

      /**
       * Executes a single CPU instruction
       * @return Number of CPU cycles used
       */
      unsigned Step();

      /**
       * Executes one or more CPU clock cycles
       * @param n Number of cycles (default=1)
       */
      void Tick(unsigned n = 1);

      /**
       * Sends an interrupt to the CPU.
       * @param msg Interrupt message
       * @return True if the CPU accepts the interrupt
       */
      bool SendInterrupt(word_t msg);

      /**
       * Writes a copy of CPU state in a chunk of memory pointer by ptr.
       * @param ptr Pointer where to write
       * @param size Size of the chunk of memory where we can write. If
       * successful, it will be set to the size of the written data.
       */
      virtual void GetState(void* ptr, std::size_t& size) const;

      /**
       * Sets the CPU state.
       * @param ptr Pointer were read the state information
       * @param size Size of the chunk of memory were will read.
       * @return True if can read the State data from the pointer.
       */
      virtual bool SetState(const void* ptr, std::size_t size);
    protected:

      // I/O Interface for opcodes
      word_t IORead(word_t);
      void IOWrite(word_t, word_t);

      unsigned int cpu_clock;   // CPU clock speed

      // CPU Core
      word_t r[8];
      word_t pc;
      word_t sp;
      word_t ex;
      word_t ia;

      // EMU
      dword_t emu[16];

      // Interrupt
      word_t iqp;
      word_t iqe;
      word_t iqc;
      word_t intq[256];

      // hardware status
      unsigned phase;
      unsigned phasenext;
      unsigned pwrdraw;
      unsigned wait_cycles;
      unsigned last_cycles;

      // Internal use registers
      word_t acu;
      dword_t aca;
      word_t bcu;
      dword_t bca;
      word_t opcl;
      word_t wrt;
      word_t fetchh;

      // Status flags
      bool addradd;
      bool addrdec;
      bool bytemode;
      bool bytehigh;
      bool skip;
      bool fire;
      bool qint;

    };

    struct DCPU16NState {
      // CPU Core
      word_t r[8];
      word_t pc;
      word_t sp;
      word_t ex;
      word_t ia;
      // status flags
      bool addradd;
      bool addrdec;
      bool bytemode;
      bool bytehigh;
      bool skip;
      bool fire;
      bool qint;
      // hardware status
      unsigned phase;
      unsigned phasenext;
      unsigned pwrdraw;
      unsigned wait_cycles;
      unsigned last_cycles;
      // EMU
      dword_t emu[16];
      // Interrupt
      word_t iqp;
      word_t iqe;
      word_t iqc;
      word_t intq[256];
      // Internal use registers
      word_t acu;
      dword_t aca;
      word_t bcu;
      dword_t bca;
      word_t opcl;
      word_t wrt;
      word_t fetchh;
    };
  }
}

#endif
