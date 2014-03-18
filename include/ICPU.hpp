#pragma once
/**
 * Trillek Virtual Computer - ICPU.hpp
 * Defines a base interface for CPUs to the Virtual Computer
 */

#ifndef __ICPU_HPP__
#define __ICPU_HPP__ 1

#include "Types.hpp"

namespace vm {

	class VComputer;

	namespace cpu {

		/**
		 * Interface that must be implemented by any CPU that will be used by the 
		 * Virtual Computer.
		 * Derived class constructors must set vcomp == nullptr.
		 */
		class ICPU {
			public:

				ICPU() : vcomp(nullptr) {
				}

				virtual ~ICPU() { 
				}

				/**
				 * Sets the VComputer pointer
				 * This method must be only called by VComputer itself
				 * @param vcomp VComputer pointer or nullptr
				 */
				void SetVComputer (vm::VComputer* vcomp) {
					this->vcomp = vcomp;
				}

				/**
				 * Returns CPU clock speed in Hz
				 */
				virtual unsigned Clock() = 0;

				/**
				 * Resets CPU insternal state
				 */
				virtual void Reset() = 0;

				/**
				 * Executes a singe instrucction of the CPU
				 * @return Number of CPU cycles used
				 */
				virtual unsigned Step() = 0;
				
				/**
				 * Executes one or more CPU clock cycles
				 * @param n Number of cycles (default=1)
				 */
				virtual void Tick(unsigned n=1) = 0;

				/**
				 * Sends an interrupt to the CPU.
				 * @param msg Interrupt message
				 * @return True if the CPU accepts the interrupt
				 */
				virtual bool SendInterrupt (word_t msg) = 0;

				/**
				 * Writes a copy of CPU state in a chunk of memory pointer by ptr.
				 * @param ptr Pointer were to write
				 * @param size Size of the chunk of memory were can write. If is 
				 * sucesfull, it will be set to the size of the write data.
				 */
				virtual void GetState (void* ptr, size_t& size) const = 0;

				/**
				 * Sets the CPU state.
				 * @param ptr Pointer were read the state information
				 * @param size Size of the chunk of memory were will read.
				 * @return True if can read the State data from the pointer.
				 */
				virtual bool SetState (const void* ptr, size_t size) = 0;

			protected:
				vm::VComputer* vcomp;		/// Ptr to the Virtual Computer
		};


	} // End of namespace cpu
} // End of namespace vm

#endif // __ICPU_HPP__

