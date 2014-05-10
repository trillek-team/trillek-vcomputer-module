#pragma once
/**
 * Trillek Virtual Computer - dis_dcpu16n.hpp
 * OnLine dis-assembly of DCPU-16N machine code
 */

#include "dcpu16n/dcpu16n.hpp"

#include <string>

namespace vm {
namespace cpu {

/**
 * Disassemble one instruction of DCPU-16N code to a humman readable string
 * @param vc VComputer to read from
 * @param pc 24 bit address to read instruction from
 */
std::string DisassemblyDCPU16N(const VComputer& vc, DWord pc);

} // cpu
} // vm

