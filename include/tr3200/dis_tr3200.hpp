/**
 * \brief       TR3200 CPU online dis-assembler
 * \file        dis_tr3200.hpp
 * \copyright   LGPL v3
 *
 */
#ifndef __DISTR3200_HPP_
#define __DISTR3200_HPP_ 1

#include "tr3200.hpp"

#include <string>

namespace trillek {
namespace computer {

/**
 * Disassembly one instruction of TR3200 code to a humman redable text
 * \param vc Virtual computer instance
 * \param pc Address to dissamble
 */
DECLDIR
std::string DisassemblyTR3200 (const VComputer& vc, DWord pc);

/**
 * Disassembly one instruction of TR3200 code to a humman redable text
 * \param data Ptr. to TR3200 machine code
 * \param size Size of data to read. Must be >= 8
 */
DECLDIR
std::string DisassemblyTR3200 (const Byte* data, std::size_t size);
} // End of namespace computer
} // End of namespace trillek

#endif // __DISTR3200_HPP_
