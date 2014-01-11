#pragma once
/**
 * VM - ICpu.hpp
 * CPU of the virtual machine
 */

#ifndef __ICPU_HPP__
#define __ICPU_HPP__

#include "Types.hpp"
#include "Ram.hpp"

#include <cassert>

namespace vm {
	namespace cpu {

		class ICpu {
			public:
				ICpu(size_t ram_size, unsigned clock) : ram(ram_size), clock(clock), tot_cycles(0), wait_cycles(0), skiping(false), sleeping(false), int_msg(0), interrupt(false) {
					assert(ram_size > 0);

					Reset();
				}

				virtual ~ICpu() { 
				}

				/**
				 * Return the actual CPU model clock speed
				 */
				unsigned Clock() const {
					return clock;
				}

				/**
				 * Number of CPU cycles executed
				 */
				std::size_t TotalCycles () const {
					return tot_cycles;
				}
				
				/**
				 * Resets the CPU state
				 */
				virtual void Reset() {
					wait_cycles = 0;
					skiping = false;
					sleeping = false;
					interrupt = false;
					int_msg = 0;
					
					ram.Reset();
				}

				/**
				 * Executes a singe instrucction of the CPU
				 * @return Number of CPU cycles used
				 */
				unsigned Step() {
					if (!sleeping) {
						auto cyc = RealStep();
						tot_cycles += cyc;
						return cyc;
					} else {
						tot_cycles++;
						ProcessInterrupt();
						return 1;
					}
				}
				
				/**
				 * Executes one or more CPU clock cycles
				 * @param n Number of cycles (default=1)
				 */
				void Tick(unsigned n=1) {
					assert (n > 0);

					unsigned i = 0;
					do {
						if (!sleeping) {
							if (wait_cycles <= 0 )  {
								RealStep();
							}

							wait_cycles--;
						} else {
							ProcessInterrupt();
						}

						tot_cycles++;
						i++;
					} while (i < n);
				}

				/**
				 * Throws a interrupt to the CPU
				 * @param msg Interrupt message
				 * @return True if the CPU accepts the interrupt
				 */
				virtual bool ThrowInterrupt (dword_t msg) = 0;

				/**
				 * Number of cycles that needs to end of executing the actual instruction
				 */
				unsigned WaitCycles() const { return wait_cycles;}

				/**
				 * Is skiping the actual instruction ?
				 */
				bool Skiping() const { return skiping;}

				/**
				 * Is sleeping the CPU ?
				 */
				bool Sleeping() const { return sleeping;}

				ram::Mem ram;											/// Handles the RAM mapings / access 

			protected:
				const unsigned clock;		/// CPU clock speed
				
				std::size_t tot_cycles;	/// Total number of cpu cycles executed
			
				unsigned wait_cycles;   /// Number of cycles need to wait do realice a instruction
				bool skiping;           /// Is skiping the next instruction
				bool sleeping;          /// Is sleeping?
			
				dword_t int_msg;        /// Interrupt message
				bool interrupt;         /// Is hapening an interrupt?

				/**
				 * Does the real work of executing a instrucction
				 * @param Numvber of cycles tha requires to execute an instrucction
				 */
				virtual unsigned RealStep() = 0;

				/**
				 * Process if an interrupt is waiting
				 */
				virtual void ProcessInterrupt() = 0;
		};


	} // End of namespace cpu
} // End of namespace vm

#endif // __ICPU_HPP__

