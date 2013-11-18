#pragma once
/**
 * RC3200 VM - vm.hpp
 * Virtual machine itself
 */

#ifndef __VM_HPP_
#define __VM_HPP_ 1

#include "config.hpp"
#include "types.hpp"

#include "cpu.hpp"
#include "ram.hpp"
#include "IDevice.hpp"

#include <vector>
#include <algorithm>
#include <memory>

// MS Visual C++ stuff
#if defined(_MSC_VER)
        // VC++ C compiler support : C89 thanks microsoft !
        #define snprintf _snprintf 
        
        // Get bored of theses warnings
        #pragma warning(disable : 4996) // Ni puñetera idea
        #pragma warning(disable : 4333) // Shift warning execding output var size, data loss
        #pragma warning(disable : 4018) // Comparation of signed and unsigned with auto conversion
        #pragma warning(disable : 4244) // Conversion of variables with data loss
#endif

namespace vm {
using namespace vm::cpu;
using namespace vm::ram;

const unsigned MAX_N_DEVICES = 32;  /// Max number of devices attached

enum HWN_CMD {
        GET_NUMBER = 0,
        GET_CLASS  = 1,
        GET_BUILDER  = 2,
        GET_ID  = 3,
        GET_VERSION  = 4,
        GET_JMP1  = 0x10,
        GET_JMP2  = 0x20,
};

class VirtualComputer {
public:

VirtualComputer (std::size_t ram_size = 128*1024) : cpu(ram_size), n_devices(0), enumerator(this) {
  cpu.ram.AddBlock(&enumerator); // Add Enumerator device
  std::fill_n(devices, MAX_N_DEVICES, nullptr);
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
 * Adds a Device to the virtual machine in a slot
 * @param slot Were plug the device
 * @param dev The device to be pluged in the slot
 * @return False if the slot havea device or the slot is invalid.
 */
bool AddDevice (unsigned slot , IDevice& dev) {
  if (slot >= MAX_N_DEVICES)
    return false;

  if (devices[slot] != nullptr)
    return false;

  devices[slot] = &dev;
  n_devices++;
  cpu.ram.AddBlock(dev.MemoryBlocks()); // Add Address handlerss

  return true;
}

/**
 * Remove a device from a virtual machine slot
 * @param slot Slot were unplug the device
 */
void RemoveDevice (unsigned slot) {
  if (slot < MAX_N_DEVICES && devices[slot] != nullptr) {
    devices[slot] = nullptr;
    n_devices--;
    assert(n_devices >= 0);
  }
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
 * @param delta Number of microseconds since the last call
 * @return number of cycles executed
 */
unsigned Step(long delta = 0) {
  auto cycles = cpu.Step();
  for (std::size_t i=0; i > MAX_N_DEVICES; i++) {
    if (devices[i] != nullptr)
      devices[i]->Tick(cpu, cycles, delta);
  }
  return cycles;
}

/**
 * Executes N clock ticks
 * @param n nubmer of clock ticks, by default 1
 * @param delta Number of microseconds since the last call
 */
void Tick(unsigned n=1, long delta = 0) {
  cpu.Tick(n);
  for (std::size_t i=0; i > MAX_N_DEVICES; i++) {
    if (devices[i] != nullptr)
      devices[i]->Tick(cpu, n, delta);
  }
}

private:

RC3200 cpu; /// Virtual CPU

IDevice* devices[MAX_N_DEVICES]; /// Devices atached to the virtual computer
unsigned n_devices;

  /**
   * Hardware enumerator
   */
  class HWN : public ram::AHandler {
  public:
  HWN (VirtualComputer* vm) {
    this->vm = vm;
    this->begin = 0xFF000000;
    this->size = 2;
    ndev = 0;
    read = 0;
  }

  virtual ~HWN () {
  }

  byte_t RB (dword_t addr) {
    addr -= this->begin;
    if (addr == 0)
      return read & 0xFF;
    else
      return read >> 8;
  }

  /**
   * Gets the commad value for the enumarator
   */
  void WB (dword_t addr, byte_t val) {
    addr -= this->begin;
    if (addr == 0) {
      ndev = val;
    } else {
      if (val == HWN_CMD::GET_NUMBER) { // Get number of devices commad
        read = vm->n_devices;
        return;
      }

      if (ndev >= MAX_N_DEVICES || vm->devices[ndev] == nullptr)
        return; // Invalid device

      // Updates the read value
      switch (val) {
        case HWN_CMD::GET_CLASS : 
          read = vm->devices[ndev]->DevClass();
          break; 

        case HWN_CMD::GET_BUILDER : 
          read = vm->devices[ndev]->Builder();
          break; 
        
        case HWN_CMD::GET_ID : 
          read = vm->devices[ndev]->DevId();
          break; 

        case HWN_CMD::GET_VERSION : 
          read = vm->devices[ndev]->DevVer();
          break; 

        case HWN_CMD::GET_JMP1 : 
          read = vm->devices[ndev]->Jmp1();
          break; 

        case HWN_CMD::GET_JMP2 : 
          read = vm->devices[ndev]->Jmp2();
          break; 

        default:
          break;

      }
    }
  }
  
  private:
          VirtualComputer* vm;

          byte_t ndev;    /// Device Index of the HWN command
          word_t read;    /// Value to be read
          

  };

HWN enumerator;

};

} // End of namespace vm

#endif // __VM_HPP_
