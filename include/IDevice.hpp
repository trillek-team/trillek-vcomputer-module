#pragma once
/**
 * RC3200 VM - CDA.hpp
 * Base class of all devices
 *
 */

#ifndef __IDEVICE_HPP__
#define __IDEVICE_HPP__ 1

#include "Types.hpp"
#include "TR3200.hpp"

#include <memory>
#include <vector>

namespace vm {

/** 
 * Base class of all devices
 */
class IDevice {
public:

  /**
   * Build a device setting his jumper values
   * @param j1 Jumper 1 value
   * @param j2 Jumper 2 value
   */
  IDevice(dword_t j1 = 0, dword_t j2 = 0);

  virtual ~IDevice();

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
   * Return Jumper 2 value
   */
  word_t Jmp2() const {
    return jmp2;
  }


  /**
   * Does Hardware stuff in sync with the CPU clock
   * @param cpu Ptr to the CPU, if needs to thorow a interrupt
   * @param n Number of clock ticks executing
   * @param delta Number milliseconds since the last call
   */
  virtual void Tick (cpu::RC3200& cpu, unsigned n=1, const double delta = 0) = 0;

  /**
   * Return an vector of ptrs AHandler that uses this device
   */
  virtual std::vector<ram::AHandler*> MemoryBlocks() const;

protected:
  unsigned jmp1;
  unsigned jmp2;

};

} // End of namespace vm

#endif
