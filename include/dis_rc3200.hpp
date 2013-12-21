#pragma once
/**
 * RC3200 VM - dis_rc3200.hpp
 * On-Line dissambler of VM ram
 */
#ifndef __DIS_RC3200_HPP__
#define __DIS_RC3200_HPP__ 1

#include "Cpu.hpp"

#include <string>

namespace vm {
namespace cpu {

/**
* Disassembly one instruction of RC3200 code to a humman redable text
* @param data Ptr. to RC3200 machine code
*/
std::string Disassembly (const ram::Mem& ram, dword_t pc);

} // End of namespace cpu
} // End of namespace vm

#endif // __DIS_RC3200_HPP__
