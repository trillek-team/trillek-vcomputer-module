#pragma once
/**
 * Trillek Virtual Computer - DisTR3200.hpp
 * OnLine dis-assembler of TR3200 machine code
 */

#ifndef __DIS_TR3200_HPP__
#define __DIS_TR3200_HPP__ 1

#include "TR3200.hpp"

#include <string>

namespace vm {
	namespace cpu {

		/**
		 * Disassembly one instruction of TR3200 code to a humman redable text
		 * @param data Ptr. to TR3200 machine code
		 */
		std::string Disassembly (const VComputer& vc, dword_t pc);

	} // End of namespace cpu
} // End of namespace vm

#endif // __DIS_TR3200_HPP__

