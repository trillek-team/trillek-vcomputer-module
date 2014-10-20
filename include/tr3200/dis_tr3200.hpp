/**
 * \brief       TR3200 CPU online dis-assembler
 * \file        dis_tr3200.hpp
 * \copyright   The MIT License (MIT)
 *
 */
#ifndef __DISTR3200_HPP_
#define __DISTR3200_HPP_ 1

#include "tr3200/tr3200.hpp"

#include <string>

namespace trillek {
namespace computer {

/**
 * Disassembly one instruction of TR3200 code to a humman redable text
 * \param data Ptr. to TR3200 machine code
 */
std::string DisassemblyTR3200 (const VComputer& vc, DWord pc);

} // End of namespace computer
} // End of namespace trillek

#endif // __DISTR3200_HPP_
