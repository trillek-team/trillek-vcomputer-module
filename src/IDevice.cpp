/**
 * RC3200 VM - CDA.cpp
 * Base class of all devices
 *
 */

#include "IDevice.hpp"

namespace vm {

IDevice::IDevice (dword_t j1, dword_t j2) : jmp1(j1), jmp2(j2) {
}

IDevice::~IDevice () {
}

word_t IDevice::Jmp1() const {
  return jmp1;
}

word_t IDevice::Jmp2() const {
  return jmp2;
}

std::vector<ram::AHandler*> IDevice::MemoryBlocks() const {
  std::vector<ram::AHandler*> output;
  return output;
}

} // End of namespace vm
