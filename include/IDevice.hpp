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

	class VComputer;

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
			virtual void SetVComputer (VComputer* vcomp) {
				this->vcomp = vcomp;
			}

			/**
			 * Sends (writes to CMD register) a command to the device
			 * @param cmd Command value to send
			 */
			virtual void SendCMD (word_t cmd) = 0;

			virtual void A (word_t cmd) { }
			virtual void B (word_t cmd) { }
			virtual void C (word_t cmd) { }
			virtual void D (word_t cmd) { }
			virtual void E (word_t cmd) { }

			virtual word_t A () { return 0; }
			virtual word_t B () { return 0; }
			virtual word_t C () { return 0; }
			virtual word_t D () { return 0; }
			virtual word_t E () { return 0; }

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
			virtual bool IsSyncDev() const {
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
			virtual bool DoesInterrupt(word_t& msg) {
				return false;
			}

			/**
			 * Informs to the device that his generated interrupt was accepted by the CPU
			 */
			void IACK () {
			}

		protected:
			VComputer* vcomp;		/// Ptr to the Virtual Computer
	};

} // End of namespace vm

#endif
