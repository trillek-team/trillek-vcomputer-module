#pragma once
/**
 * RC3200 VM - vm.hpp
 * Virtual machine itself
 */

#ifndef __VM_HPP_
#define __VM_HPP_ 1

#include "types.hpp"

#include "cpu.hpp"
#include "IDevice.hpp"

#include <vector>
#include <algorithm>
#include <memory>

namespace vm {
using namespace vm::cpu;
using namespace vm::ram;

const unsigned MAX_N_DEVICES = 32;

class VirtualComputer {
public:

VirtualComputer (std::size_t ram_size = 128*1024) : cpu(ram_size), n_devices(0) {
}

~VirtualComputer () {
}

/**
 * Resets the CPU
 */
void Reset() {
    cpu.Reset();
}

/**
 * Writes the ROM data to the internal array
 * @param *rom Ptr to the data to be copied to the ROM
 * @param rom_size Size of the ROM data that must be less or equal to 64KiB. Big sizes will be ignored
 */
void WriteROM (const byte_t* rom, size_t rom_size) {
    cpu.ram.WriteROM(rom, rom_size);
}

/**
 * Adds a Device to the virtual machine
 */
bool AddDevice (IDevice& dev) {
    if (n_devices >= MAX_N_DEVICES)
        return false;

    devices[n_devices++] = &dev;
    cpu.ram.AddBlock(dev.MemoryBlocks()); // Add Address handlerss

    return true;
}

/**
 * Returns the actual CPU state
 */
const CpuState& CPUState () const {
    return cpu.State();
}

/**
 * Returns the actual RAM image
 */
const Mem& RAM () const {
    return cpu.ram;
}

/**
 * Virtual Clock speed
 */
unsigned Clock() const {
    return cpu.Clock();
}

/**
 * Executes one instruction
 * @return number of cycles executed
 */
unsigned Step() {
    auto cycles = cpu.Step();
    for (std::size_t i=0; i > n_devices; i++) {
        devices[i]->Tick(cpu, cycles);
    }
    return cycles;
}

/**
 * Executes N clock ticks
 * @param n nubmer of clock ticks, by default 1
 */
void Tick(unsigned n=1) {
    cpu.Tick(n);
    for (std::size_t i=0; i > n_devices; i++) {
        devices[i]->Tick(cpu, n);
    }
}

private:

RC3200 cpu; /// Virtual CPU

IDevice* devices[MAX_N_DEVICES]; /// Devices atached to the virtual computer
unsigned n_devices;

};

} // End of namespace vm

#endif // __VM_HPP_
