#pragma once
/**
 * Trillek Virtual Computer - dis_dcpu16n.hpp
 * OnLine dis-assembly of DCPU-16N machine code
 */

#include "dcpu16n/dcpu16n.hpp"

#include <string>

namespace trillek {
namespace computer {

/**
 * Disassemble one instruction of DCPU-16N code to a humman readable string
 * @param vc VComputer to read from
 * @param pc 24 bit address to read instruction from
 */
		DECLDIR
std::string DisassemblyDCPU16N(const VComputer& vc, DWord pc);

} // computer
} // trillek
