/**
 * RC3200 VM - CDA.cpp
 * Base class of all devices
 *
 */

#include "IDevice.hpp"

namespace vm {

  IDevice::IDevice (dword_t j1, dword_t j2) : jmp1(j1), jmp2(j2), do_interrupt(false) {
  }

  IDevice::~IDevice () {
  }

	bool IDevice::DoesInterrupt(dword_t& msg) {
		if (do_interrupt) {
			msg = int_msg;
			do_interrupt = false;
			return true;
		}

		return false;
	}

  std::vector<ram::AHandler*> IDevice::MemoryBlocks() const {
    std::vector<ram::AHandler*> output;
    return output;
  }

} // End of namespace vm
