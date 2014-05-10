/**
 * \brief       Class definitions for the DCPU-16N CPU
 * \file        dcpu16n.hpp
 * \copyright   The MIT License (MIT)
 *
 */

#ifndef __DCPU16N_HPP__
#define __DCPU16N_HPP__

#include "cpu.hpp"
#include "vcomputer.hpp"

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
    virtual void Reset ();

    /**
     * Executes a single CPU instruction
     * \return Number of CPU cycles used
     */
    unsigned Step ();

    /**
     * Executes one or more CPU clock cycles
     * \param n Number of cycles (default=1)
     */
    void Tick (unsigned n = 1);

    /**
     * Sends an interrupt to the CPU.
     * \param msg Interrupt message
     * \return True if the CPU accepts the interrupt
     */
    bool SendInterrupt (Word msg);

    /**
     * Writes a copy of CPU state in a chunk of memory pointer by ptr.
     * @param ptr Pointer where to write
     * @param size Size of the chunk of memory where we can write. If
     * successful, it will be set to the size of the written data.
     */
    virtual void GetState (void* ptr, std::size_t& size) const;

    /**
     * Sets the CPU state.
     * @param ptr Pointer were read the state information
     * @param size Size of the chunk of memory were will read.
     * @return True if can read the State data from the pointer.
     */
    virtual bool SetState (const void* ptr, std::size_t size);

protected:

    // I/O Interface for opcodes
    Word IORead(Word);
    void IOWrite(Word, Word);

    unsigned int cpu_clock; // CPU clock speed

    // CPU Core
    Word r[8];
    Word pc;
    Word sp;
    Word ex;
    Word ia;

    // EMU
    DWord emu[16];

    // Interrupt
    Word iqp;
    Word iqe;
    Word iqc;
    Word intq[256];

    // hardware status
    unsigned phase;
    unsigned phasenext;
    unsigned pwrdraw;
    unsigned wait_cycles;
    unsigned last_cycles;

    // Internal use registers
    Word acu;
    DWord aca;
    Word bcu;
    DWord bca;
    Word opcl;
    Word wrt;
    Word fetchh;

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
    Word r[8];
    Word pc;
    Word sp;
    Word ex;
    Word ia;
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
    DWord emu[16];
    // Interrupt
    Word iqp;
    Word iqe;
    Word iqc;
    Word intq[256];
    // Internal use registers
    Word acu;
    DWord aca;
    Word bcu;
    DWord bca;
    Word opcl;
    Word wrt;
    Word fetchh;
};

} // cpu
} // vm

#endif
