#pragma once
/**
 * Trillek Virtual Computer - ICPU.hpp
 * Defines a base interface for all Devices in the Virtual Computer
 */

#ifndef __IDEVICE_HPP__
#define __IDEVICE_HPP__ 1

#include "Types.hpp"
#include "VComputer.hpp"

namespace vm {

	/** 
	 * Interface that must be implemente by any Device that will be used by the
	 * Virtual Computer.
	 * Derived class constructors must set vcomp == nullptr.
	 */
	class IDevice {
		public:

			virtual ~IDevice() {
			}
			
			/**
			 * Sets the VComputer pointer
			 * This method must be only called by VComputer itself
			 * @param vcomp VComputer pointer or nullptr
			 */
			virtual void SetVComputer (vm::VComputer* vcomp) {
				this->vcomp = vcomp;
			}

			/**
			 * Sends (writes to CMD register) a command to the device
			 * @param cmd Command value to send
			 */
			virtual void SendCMD (word_t cmd) = 0;

			virtual void A (word_t cmd) = 0;
			virtual void B (word_t cmd) = 0;
			virtual void C (word_t cmd) = 0;
			virtual void D (word_t cmd) = 0;
			virtual void E (word_t cmd) = 0;

			virtual word_t A () = 0;
			virtual word_t B () = 0;
			virtual word_t C () = 0;
			virtual word_t D () = 0;
			virtual word_t E () = 0;

			/**
			 * Device Type
			 */
			virtual byte_t DevType() const = 0;

			/**
			 * Device Builder ID
			 */
			virtual dword_t DevBuildID() const = 0;

			/**
			 * Device ID
			 */
			virtual byte_t DevId() const = 0;

			/**
			 * Device Revision
			 */
			virtual byte_t DevRev() const = 0;

			/**
			 * Return if the device does something each Device Clock tick.
			 * Few devices really need to do this, so IDevice implementation 
			 * returns false.
			 */
			virtual bool IsSyncDev const {
				return false;
			}

			/**
			 * Executes N Device clock cycles. 
			 * Here resides the code that is executed every Device Clock tick. 
			 * IDevice implementation does nothing.
			 * @param n Number of clock ticks executing
			 * @param delta Number milliseconds since the last call
			 */
			virtual void Tick (unsigned n=1, const double delta = 0) { 
			}

			/**
			 * Checks if the device is trying to generate an interrupt
			 * @param msg The interrupt message will be writen here
			 * @return True if is generating a new interrupt
			 */
			bool DoesInterrupt(word_t& msg);


		protected:
	};

} // End of namespace vm

#endif
