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

VirtualComputer (std::size_t ram_size = 128*1024) : cpu(ram_size), n_devices(0), enumerator(this), timers(this) {
  cpu.ram.AddBlock(&enumerator);  // Add Enumerator address handler
  cpu.ram.AddBlock(&timers);      // Add PIT address handler
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
 * @param delta Number of milliseconds since the last call
 * @return number of cycles executed
 */
unsigned Step( const double delta = 0) {
  auto cycles = cpu.Step();
  timers.Update(cycles);
  
  for (std::size_t i=0; i < MAX_N_DEVICES; i++) {
    if (devices[i] != nullptr) {
      devices[i]->Tick(cpu, cycles, delta);
    }
  }
  return cycles;
}

/**
 * Executes N clock ticks
 * @param n nubmer of clock ticks, by default 1
 * @param delta Number of milliseconds since the last call
 */
void Tick( unsigned n=1, const double delta = 0) {
  cpu.Tick(n);
  timers.Update(n);
  
  for (std::size_t i=0; i < MAX_N_DEVICES; i++) {
    if (devices[i] != nullptr) {
      devices[i]->Tick(cpu, n, delta);
    }
  }
}

private:

RC3200 cpu; /// Virtual CPU

IDevice* devices[MAX_N_DEVICES]; /// Devices atached to the virtual computer
unsigned n_devices;

  /**
   * Addres handler of Hardware enumerator
   * Also implementes the HWN itself as the HWN not does nothing outside responding commands
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

      if (ndev >= MAX_N_DEVICES || vm->devices[ndev] == nullptr) {
        read = 0;   // Invalid device. Reads 0
        return;          
      }

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

  /**
   * Addres handler that setups the PIT (timers)
   */
  class PIT : public ram::AHandler {
  public:


  PIT (VirtualComputer* vm) : tmr0(0), tmr1(0), re0(0), re1(0), cfg(0) {
    this->vm = vm;
    this->begin = 0xFF000040;
    this->size = 17;
  }

  virtual ~PIT () {
  }

  byte_t RB (dword_t addr) {
    addr &= 0x7F; // We only need to analize address 40 to 50
    switch (addr) {
      // Read TMR0_VAL
      case 0x40 :
        return (byte_t)tmr0;

      case 0x41 :
        return (byte_t)(tmr0>>8);

      case 0x42 :
        return (byte_t)(tmr0>>16);
      
      case 0x43 :
        return (byte_t)(tmr0>>24);

      // Read TMR0_RELOAD
      case 0x44 :
        return (byte_t)(re0);

      case 0x45 :
        return (byte_t)(re0>>8);

      case 0x46 :
        return (byte_t)(re0>>16);

      case 0x47 :
        return (byte_t)(re0>>24);

      // Read TMR1_VAL
      case 0x48 :
        return (byte_t)tmr1;

      case 0x49 :
        return (byte_t)(tmr1>>8);

      case 0x4A :
        return (byte_t)(tmr1>>16);
      
      case 0x4B :
        return (byte_t)(tmr1>>24);

      // Read TMR1_RELOAD
      case 0x4C :
        return (byte_t)(re1);

      case 0x4D :
        return (byte_t)(re1>>8);

      case 0x4E :
        return (byte_t)(re1>>16);

      case 0x4F :
        return (byte_t)(re1>>24);

      // Read TMR_CFG
      case 0x50 :
        return cfg;

      default:
        return 0;
    }
  }

  void WB (dword_t addr, byte_t val) {
    addr &= 0x7F; // We only need to analize address 40 to 50
    switch (addr) {
      // Write TMR0_RELOAD
      case 0x44 :
        re0 = (re0 & 0xFFFFFF00) | val;

      case 0x45 :
        re0 = (re0 & 0xFFFF00FF) | (val<<8);

      case 0x46 :
        re0 = (re0 & 0xFF00FFFF) | (val<<16);

      case 0x47 :
        re0 = (re0 & 0x00FFFFFF) | (val<<24);

      // Write TMR1_RELOAD
      case 0x4C :
        re1 = (re1 & 0xFFFFFF00) | val;

      case 0x4D :
        re1 = (re1 & 0xFFFF00FF) | (val<<8);

      case 0x4E :
        re1 = (re1 & 0xFF00FFFF) | (val<<16);

      case 0x4F :
        re1 = (re1 & 0x00FFFFFF) | (val<<24);

      // Write TMR_CFG
      case 0x50 :
        cfg = val;
      
      default:
        ;
    }
  }
 
  /**
   * Update the timers and generate the interrupt if underflow hapens
   * @param n Number of cycles executed
   */
  void Update(unsigned n) {
    dword_t tmp;
    if ((cfg & 1) != 0) {
      tmp = tmr0;
      tmr0 -= n;
      if (tmr0 > tmp) { // Underflow of TMR0
        tmr0 = re0 - (0xFFFFFFFF - tmr0);
        do_int_tmr0 = (cfg & 2) != 0;
      }
    }

    if ((cfg & 8) != 0) {
      tmp = tmr1;
      tmr1 -= n;
      if (tmr1 > tmp) { // Underflow of TMR1
        tmr1 = re1 - (0xFFFFFFFF - tmr1);
        do_int_tmr1 = (cfg & 16) != 0;
      }
    }

    if (((cfg & 2) != 0) && do_int_tmr0) { // Try to throw TMR0 interrupt
      do_int_tmr0 = ! vm->cpu.ThrowInterrupt(0x0001);
    } else if (((cfg & 16) != 0) && do_int_tmr1) { // Try to thorow TMR1 interrupt 
      do_int_tmr1 = ! vm->cpu.ThrowInterrupt(0x1001);
    }
  }

  private:
  VirtualComputer* vm;
    
  dword_t tmr0;     /// Timer 0
  dword_t tmr1;     /// Timer 1
  
  dword_t re0;      /// Reload value of Timer 0
  dword_t re1;      /// Reload value of Timer 1

  byte_t cfg;       /// Config byte of PIT

  bool do_int_tmr0; /// Try to thorow interrupt of TMR0 ?
  bool do_int_tmr1; /// Try to thorow interrupt of TMR1 ?

  };

PIT timers;

};

} // End of namespace vm

#endif // __VM_HPP_
