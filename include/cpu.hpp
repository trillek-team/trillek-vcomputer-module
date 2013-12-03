#pragma once
/**
 * RC3200 VM - cpu.hpp
 * CPU of the virtual machine
 */

#ifndef __CPU_HPP__
#define __CPU_HPP__

#include "types.hpp"
#include "ram.hpp"
#include "rc3200_opcodes.hpp"

#include <vector>

namespace vm {
namespace cpu {

static unsigned const N_GPRS = 32;

/**
 * Represent the state of the CPU in a moment of time
 */
struct CpuState {
    dword_t r[N_GPRS];      /// Registers
    dword_t pc;             /// Program Counter 
    
    dword_t int_msg;        /// Interrupt message
    
    unsigned wait_cycles;   /// Number of cycles need to wait do realice a instruction
    
    bool skiping;           /// Is skiping the next instruction
    bool sleeping;          /// Is sleeping?

    bool interrupt;         /// Is hapening an interrupt?
    bool iacq;              /// IACQ signal

    bool step_mode;         /// Is in step mode execution ?
};

// Operation in Flags bits
#define GET_CF(x)          ((x) & 0x1)
#define SET_ON_CF(x)       (x |= 0x1)
#define SET_OFF_CF(x)      (x &= 0xFFFFFFFE)

#define GET_OF(x)          (((x) & 0x2) >> 1)
#define SET_ON_OF(x)       (x |= 0x2)
#define SET_OFF_OF(x)      (x &= 0xFFFFFFFD)

#define GET_DE(x)          (((x) & 0x4) >> 2)
#define SET_ON_DE(x)       (x |= 0x4)
#define SET_OFF_DE(x)      (x &= 0xFFFFFFFB)

#define GET_IF(x)          (((x) & 0x8) >> 3)
#define SET_ON_IF(x)       (x |= 0x8)
#define SET_OFF_IF(x)      (x &= 0xFFFFFFF7)

// Enable bits that change what does the CPU
#define GET_EI(x)          (((x) & 0x100) >> 8)
#define SET_ON_EI(x)       (x |= 0x100)
#define SET_OFF_EI(x)      (x &= 0xFFFFFEFF)

#define GET_ESS(x)         (((x) & 0x200) >> 9)
#define SET_ON_ESS(x)      (x |= 0x200)
#define SET_OFF_ESS(x)     (x &= 0xFFFFFDFF)

/*
#define GET_EOE(x)         (((x) & 0x400) >> 10)
#define SET_ON_EOE(x)      (x |= 0x400)
#define SET_OFF_EOE(x)     (x &= 0xFFFFFBFF)

#define GET_EDE(x)         (((x) & 0x800) >> 11)
#define SET_ON_EDE(x)      (x |= 0x800)
#define SET_OFF_EDE(x)     (x &= 0xFFFFF7FF)
*/

class RC3200 {
public:
    
RC3200(size_t ram_size = 128*1024, unsigned clock = 100000 );

virtual ~RC3200();

/**
 * Return the actual CPU model clock speed
 */
virtual unsigned Clock() const {
    return this->clock;;
}

/**
 * Number of CPU cycles executed
 */
std::size_t TotalCycles () const {
    return tot_cycles;
}

/**
 * Resets the CPU state
 */
void Reset();

/**
 * Executes a singe instrucction of the CPU
 * @return Number of CPU cycles used
 */
unsigned Step();

/**
 * Executes one or more CPU clock cycles
 * @param n Number of cycles (default=1)
 */
void Tick(unsigned n=1);

/**
 * Return the actual CPU state
 */
const CpuState& State() const {
    return state;
}

/**
 * Throws a interrupt to the CPU
 * @param msg Interrupt message
 * @return True if the CPU accepts the interrupt
 */
bool ThrowInterrupt (dword_t msg) {
    if (!state.iacq && GET_EI(FLAGS)) {
        // The CPU accepts a new interrupt
        state.interrupt = true;
        state.int_msg = msg;
        return true;
    }
    return false;
}

ram::Mem ram;               /// Handles the RAM mapings / access 

protected:

CpuState state;             /// CPU actual state
std::size_t tot_cycles;     /// Total number of cpu cycles executed

unsigned clock;             /// CPU clock speed

unsigned RealStep();

void ProcessInterrupt();

};

} // End of namespace cpu
} // End of namespace vm


#endif // __CPU_HPP__

