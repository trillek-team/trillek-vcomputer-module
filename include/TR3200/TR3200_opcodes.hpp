#pragma once
/**
 * Trillek Virtual Computer - TR3200_opcodes.hpp
 * OpCodes of TR3200 CPU v0.10
 * @see https://github.com/trillek-team/trillek-computer/blob/master/TR3200.md
 */

namespace vm {
  namespace cpu {

    // 3 Parameters OpCodes *******************************************************

    /**
     * 3 parameters OpCodes
     */
    enum P3_OPCODE {
      AND             = 0x80,
      OR              = 0x81,
      XOR             = 0x82,
      BITC            = 0x83,

      ADD             = 0x84,
      ADDC            = 0x85,
      SUB             = 0x86,
      SUBB            = 0x87,
      RSB             = 0x88,
      RSBB            = 0x89,

      LLS             = 0x8A,
      RLS             = 0x8B,
      ARS             = 0x8C,
      ROTL            = 0x8D,
      ROTR            = 0x8E,

      MUL             = 0x8F,
      SMUL            = 0x90,
      DIV             = 0x91,
      SDIV            = 0x92,

      LOAD            = 0x93,
      LOADW           = 0x94,
      LOADB           = 0x95,

      STORE           = 0x96,
      STOREW          = 0x97,
      STOREB          = 0x98,
    };

    // 2 Parameters OpCodes *******************************************************

    /**
     * 2 parameter OpCodes
     */
    enum P2_OPCODE {
      MOV             = 0x40,
      SWP             = 0x41,

      SIGXB           = 0x42,
      SIGXW           = 0x43,

      NOT             = 0x44,

      LOAD2           = 0x45,
      LOADW2          = 0x46,
      LOADB2          = 0x47,

      STORE2          = 0x48,
      STOREW2         = 0x49,
      STOREB2         = 0x4A,

      IFEQ            = 0x4B,
      IFNEQ           = 0x4C,

      IFL             = 0x4D,
      IFSL            = 0x4E,
      IFLE            = 0x4F,
      IFSLE           = 0x50,

      IFBITS          = 0x51,
      IFCLEAR         = 0x52,

      JMP2            = 0x53,
      CALL2           = 0x54,
    };

    // 1 Parameter OpCodes ********************************************************

    /**
     * 1 Parameter OpCodes
     */
    enum P1_OPCODE {

      XCHGB           = 0x20,
      XCHGW           = 0x21,

      GETPC           = 0x22,

      POP             = 0x23,
      PUSH            = 0x24,

      JMP             = 0x25,
      CALL            = 0x26,

      RJMP            = 0x27,
      RCALL           = 0x28,

      INT             = 0x29,
    };

    // 0 Parameters OpCodes ********************************************************

    /**
     * 0 Paramaters OpCodes
     */
    enum NP_OPCODE {
      SLEEP           = 0x00,

      RET             = 0x01,
      RFI             = 0x02,
    };

  } // End of namespace cpu
} // End of namespace vm

