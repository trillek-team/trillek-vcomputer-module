#ifndef __DIS_RC1600_HPP__
#define __DIS_RC1600_HPP__ 1

#include "cpu.hpp"

#include <string>

namespace CPU {

/**
* Disassembly one instruction of RC1600 code to a humman redable text
* @param data Ptr. to RC1600 machine code
*/
std::string disassembly(const Mem& ram, dword_t epc);

} // End of namespace CPU

#endif // __DIS_RC1600_HPP__
