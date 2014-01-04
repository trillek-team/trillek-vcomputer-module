#pragma once
/**
 * TR3200 VM - TR3200.hpp
 * CPU of the virtual machine
 */

#ifndef __TR3200_HPP__
#define __TR3200_HPP__

#include "VSFix.hpp"

#include "Types.hpp"
#include "Ram.hpp"
#include "TR3200_opcodes.hpp"

#include <vector>

namespace vm {
	namespace cpu {

		static unsigned const TR3200_NGPRS = 32;

		/**
		 * Represent the state of the CPU in a moment of time
		 */
		struct CpuState {
			dword_t r[TR3200_NGPRS];      /// Registers
			dword_t pc;             /// Program Counter 

			dword_t int_msg;        /// Interrupt message

			unsigned wait_cycles;   /// Number of cycles need to wait do realice a instruction

			bool skiping;           /// Is skiping the next instruction
			bool sleeping;          /// Is sleeping?

			bool interrupt;         /// Is hapening an interrupt?
			bool iacq;              /// IACQ signal

			bool step_mode;         /// Is in step mode execution ?
		};

		// Alias to BP and SP registers
#define BP (30)
#define SP (31)

		// Alias to Ym Flags and IA registers
#define RY      state.r[27]
#define IA      state.r[28]
#define FLAGS   state.r[29]

		/// Instrucction types
#define IS_PAR3(x)  (((x) & 0xC0000000) == 0x40000000 )
#define IS_PAR2(x)  (((x) & 0x80000000) == 0x80000000 )
#define IS_PAR1(x)  (((x) & 0xE0000000) == 0x20000000 )
#define IS_NP(x)    (((x) & 0xE0000000) == 0x00000000 )

		/// Instrucction sub-type
#define IS_BRANCH(x)    (((x) & 0xE0000000) == 0xA0000000 )

		/// Uses a Literal value ?
#define HAVE_LITERAL(x)     (((x) & 0x00800000) != 0)

		/// Extract operands
#define GRD(x)              ( (x)       & 0x1F) 
#define GRS(x)              (((x) >> 5) & 0x1F) 
#define GRN(x)              (((x) >> 10)& 0x1F) 

#define LIT13(x)            (((x) >> 10)& 0x1FFF) 
#define LIT18(x)            (((x) >> 5) & 0x3FFFF) 
#define LIT22(x)            ( (x)       & 0x7FFFFF) 

		/// Uses next dword as literal
#define IS_BIG_LITERAL_L13(x)   ((x) == 0x1000)
#define IS_BIG_LITERAL_L18(x)   ((x) == 0x20000)
#define IS_BIG_LITERAL_L22(x)   ((x) == 0x400000)

		// Macros for ALU operations
#define CARRY_BIT(x)        ((((x) >> 32) & 0x1) == 1)
#define DW_SIGN_BIT(x)      ( ((x) >> 31) & 0x1)
#define W_SIGN_BIT(x)       ( ((x) >> 15) & 0x1)
#define B_SIGN_BIT(x)       ( ((x) >> 7)  & 0x1)

		// Extract sign of Literal Operator
#define O13_SIGN_BIT(x)     (((x) >> 12)  & 0x1)
#define O18_SIGN_BIT(x)     (((x) >> 17)  & 0x1)
#define O22_SIGN_BIT(x)     (((x) >> 21)  & 0x1)



		// Operation in Flags bits
#define GET_CF(x)          ((x) & 0x1)
#define SET_ON_CF(x)       (x |= 0x1)
#define SET_OFF_CF(x)      (x &= 0xFFFFFFFE)

#define GET_OF(x)          (((x) & 0x2) >> 1)
#define SET_ON_OF(x)       (x |= 0x2)
#define SET_OFF_OF(x)      (x &= 0xFFFFFFFD)

#define GET_DE(x)          (((x) & 0x4) >> 2)
#define SET_ON_DE(x)       (x |= 0x4)
#define SET_OFF_DE(x)      (x &= 0xFFFFFFFB)

#define GET_IF(x)          (((x) & 0x8) >> 3)
#define SET_ON_IF(x)       (x |= 0x8)
#define SET_OFF_IF(x)      (x &= 0xFFFFFFF7)

		// Enable bits that change what does the CPU
#define GET_EI(x)          (((x) & 0x100) >> 8)
#define SET_ON_EI(x)       (x |= 0x100)
#define SET_OFF_EI(x)      (x &= 0xFFFFFEFF)

#define GET_ESS(x)         (((x) & 0x200) >> 9)
#define SET_ON_ESS(x)      (x |= 0x200)
#define SET_OFF_ESS(x)     (x &= 0xFFFFFDFF)

		class RC3200 {
			public:

				/**
				 * Builds a RC3200 CPU
				 * @param ram_size Size of the Ram in BYTES
				 * @param clock CPU clock speed
				 */
				RC3200(size_t ram_size = 128*1024, unsigned clock = 100000 );

				virtual ~RC3200();

				/**
				 * Return the actual CPU model clock speed
				 */
				virtual unsigned Clock() const;

				/**
				 * Number of CPU cycles executed
				 */
				std::size_t TotalCycles () const;

				/**
				 * Resets the CPU state
				 */
				void Reset();

				/**
				 * Executes a singe instrucction of the CPU
				 * @return Number of CPU cycles used
				 */
				unsigned Step();

				/**
				 * Executes one or more CPU clock cycles
				 * @param n Number of cycles (default=1)
				 */
				void Tick(unsigned n=1);

				/**
				 * Return the actual CPU state
				 */
				const CpuState& State() const {
					return state;
				}

				/**
				 * Throws a interrupt to the CPU
				 * @param msg Interrupt message
				 * @return True if the CPU accepts the interrupt
				 */
				bool ThrowInterrupt (dword_t msg);

				ram::Mem ram;               /// Handles the RAM mapings / access 

			protected:

				CpuState state;             /// CPU actual state
				std::size_t tot_cycles;     /// Total number of cpu cycles executed

				unsigned clock;             /// CPU clock speed

				/**
				 * Does the real work of executing a instrucction
				 * @param Numvber of cycles tha requires to execute an instrucction
				 */
				unsigned RealStep();

				/**
				 * Process if an interrupt is waiting
				 */
				void ProcessInterrupt();

		};

	} // End of namespace cpu
} // End of namespace vm


#endif // __TR3200_HPP__

