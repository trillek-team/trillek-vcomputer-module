/**
 * \brief       TR3200 CPU online dis-assembler
 * \file        DisTR3200.hpp
 * \copyright   The MIT License (MIT)
 *
 */
#ifndef __DISTR3200_HPP_
#define __DISTR3200_HPP_ 1

#include "TR3200.hpp"

#include <string>

namespace vm {
namespace cpu {

/**
 * Disassembly one instruction of TR3200 code to a humman redable text
 * \param data Ptr. to TR3200 machine code
 */
std::string DisassemblyTR3200 (const VComputer& vc, dword_t pc);
} // End of namespace cpu
} // End of namespace vm

#endif // __DISTR3200_HPP_
