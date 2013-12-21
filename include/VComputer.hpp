#pragma once
/**
 * RC3200 VM - vcomputer.hpp
 * Virtual machine itself
 */

#ifndef __VM_HPP_
#define __VM_HPP_ 1

#include "Types.hpp"

#include "Cpu.hpp"
#include "Ram.hpp"
#include "IDevice.hpp"

#include <vector>
#include <algorithm>
#include <memory>


namespace vm {
using namespace vm::cpu;
using namespace vm::ram;

const unsigned MAX_N_DEVICES = 32;  /// Max number of devices attached

/**
 * Hardware Enumerator Commands
 */
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

  /**
   * Builds a Virtual Computer
   * @param ram_size RAM size in BYTES
   */
  VirtualComputer (std::size_t ram_size = 128*1024);

  ~VirtualComputer ();

  /**
   * Resets the virtual machine (but not clears RAM!)
   */
  void Reset();

  /**
   * Writes the ROM data to the internal array
   * @param *rom Ptr to the data to be copied to the ROM
   * @param rom_size Size of the ROM data that must be less or equal to 64KiB. Big sizes will be ignored
   */
  void WriteROM (const byte_t* rom, size_t rom_size);

  /**
   * Adds a Device to the virtual machine in a slot
   * @param slot Were plug the device
   * @param dev The device to be pluged in the slot
   * @return False if the slot have a device or the slot is invalid.
   */
  bool AddDevice (unsigned slot , IDevice& dev);

  /**
   * Remove a device from a virtual machine slot
   * @param slot Slot were unplug the device
   */
  void RemoveDevice (unsigned slot);

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
  unsigned Step( const double delta = 0);

  /**
   * Executes N clock ticks
   * @param n nubmer of clock ticks, by default 1
   * @param delta Number of milliseconds since the last call
   */
  void Tick( unsigned n=1, const double delta = 0);

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
    HWN (VirtualComputer* vm);

    virtual ~HWN ();

    byte_t RB (dword_t addr);

    /**
     * Gets the commad value for the enumarator
     */
    void WB (dword_t addr, byte_t val);
  
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
    PIT (VirtualComputer* vm);

    virtual ~PIT ();

    byte_t RB (dword_t addr);

    void WB (dword_t addr, byte_t val);
   
    /**
     * Update the timers and generate the interrupt if underflow hapens
     * @param n Number of cycles executed
     */
    void Update(unsigned n);

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
