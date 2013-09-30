#ifndef __CPU_HPP__
#define __CPU_HPP__

#include "ram.hpp"

#include <cstdint>
#include <vector>

namespace CPU {

#define BP (14)
#define SP (15)

/**
 * Represent the state of the CPU in a moment of time
 */
struct CpuState {
    word_t r[16];   /// GPR registers

    word_t pc;      /// Program Counter 
    word_t cs;      /// Code Segment
    
    word_t ia;      /// Interrupt Address
    word_t is;      /// Interrupt Segment
    
    word_t ss;      /// Stack Segment
    word_t ds;      /// Data Segment

    word_t flags;   /// Flags register
    word_t y;       /// Y register

    unsigned wait_cycles;
    
    bool skiping;
    bool sleeping;

    bool interrupt;
    word_t int_msg;
    bool iacq;

    bool step_mode;
};

// Operation in Flags bits
#define GET_CF(x)          ((x) & 0x1)
#define SET_ON_CF(x)       (x |= 0x1)
#define SET_OFF_CF(x)      (x &= 0xFFFE)

#define GET_OF(x)          (((x) & 0x2) >> 1)
#define SET_ON_OF(x)       (x |= 0x2)
#define SET_OFF_OF(x)      (x &= 0xFFFD)

#define GET_DE(x)          (((x) & 0x4) >> 2)
#define SET_ON_DE(x)       (x |= 0x4)
#define SET_OFF_DE(x)      (x &= 0xFFFB)

#define GET_IF(x)          (((x) & 0x8) >> 3)
#define SET_ON_IF(x)       (x |= 0x8)
#define SET_OFF_IF(x)      (x &= 0xFFF7)

// Togle bits that hcange what does the CPU
#define GET_TSS(x)         (((x) & 0x100) >> 8)
#define SET_ON_TSS(x)      (x |= 0x100)
#define SET_OFF_TSS(x)     (x &= 0xFEFF)

#define GET_TOE(x)         (((x) & 0x200) >> 9)
#define SET_ON_TOE(x)      (x |= 0x200)
#define SET_OFF_TOE(x)     (x &= 0xFDFF)

#define GET_TDE(x)         (((x) & 0x400) >> 10)
#define SET_ON_TDE(x)      (x |= 0x400)
#define SET_OFF_TDE(x)     (x &= 0xFBFF)


class RC1600 {
public:
    
    RC1600();
    virtual ~RC1600();

    /**
     * Return the actual CPU model clock speed
     */
    virtual unsigned getClock() const {return 1000000;}

    void reset();

    unsigned step();

    void tick(unsigned n=1);

    const CpuState& getState() const
    {
        return state;
    }

    /**
     * Throws a interrupt to the CPU
     */
    void throwInterrupt (word_t msg)
    {
        if (!state.iacq) {
            // The CPU accpets a new interrupt
            state.interrupt = true;
            state.int_msg = msg;
        }
    }

    // TODO External class to have a real ROM mapped somewhere and check
    // bounds
    byte_t ram[0x100000]; // 1 MiB (0 to F:FFFF) 

protected:
    CpuState state;
    std::size_t tot_cycles;

    unsigned realStep();

    void processInterrupt();
    
};

} // End of namespace CPU


#endif // __CPU_HPP__

