#pragma once
/**
 * RC3200 VM - CDA.hpp
 * Base class of all devices
 *
 */

#ifndef __IDEVICE_HPP__
#define __IDEVICE_HPP__ 1

#include "types.hpp"
#include "cpu.hpp"

#include <memory>
#include <vector>

#include <cassert>


namespace vm {

/** 
 * Base class of all devices
 */
class IDevice {
public:

IDevice() : jmp1(0), jmp2(0) {
}

virtual ~IDevice() {
}

/**
 * Device Class
 */
virtual byte_t DevClass() const = 0;

/**
 * Device Builder/Vendor
 */
virtual word_t Builder() const = 0;

/**
 * Device ID
 */
virtual word_t DevId() const = 0;

/**
 * Device Version
 */
virtual word_t DevVer() const = 0;

/**
 * Return Jumper 1 value
 */
word_t Jmp1() const {
    return jmp1;
}

/**
 * Sets Jumper 1
 */
void Jmp1(byte_t val) {
    jmp1 = val;
}

/**
 * Return Jumper 2 value
 */
word_t Jmp2() const {
    return jmp2;
}

/**
 * Sets Jumper 2
 */
void Jmp2(byte_t val) {
    jmp2 = val;
}

/**
 * Does Hardware stuff in sync with the CPU clock
 * @param cpu Ptr to the CPU, if needs to thorow a interrupt
 * @param n Number of clock ticks executing
 */
virtual void Tick (cpu::RC3200& cpu, unsigned n=1) = 0;

/**
 * Return an vector of ptrs AHandler that uses this device
 */
virtual std::vector<ram::AHandler*> MemoryBlocks() const {
    std::vector<ram::AHandler*> output;
    return output;
}

protected:
unsigned jmp1;
unsigned jmp2;

};

} // End of namespace vm

#endif
