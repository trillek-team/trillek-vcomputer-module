#pragma once
/**
* Trillek Virtual Computer - DisDCPU16N.hpp
* OnLine dis-assembly of DCPU-16N machine code
*/

#include "DCPU16N.hpp"

#include <string>

namespace vm {
  namespace cpu {

    /**
    * Disassembe one instruction of DCPU-16N code to a humman readable string
    * @param vc VComputer to read from
    * @param pc 24 bit address where to read instruction from
    */
    std::string DisassemblyDCPU16N(const VComputer& vc, dword_t pc);

  } // End of namespace cpu
} // End of namespace vm

