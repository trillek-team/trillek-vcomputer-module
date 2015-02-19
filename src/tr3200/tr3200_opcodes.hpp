/**
 * \brief       OpCodes of TR3200 CPU v0.10
 * \file        TR3200_opcodes.hpp
 * \copyright   LGPL v3
 *
 * OpCodes of TR3200 CPU v0.4.2
 * @see https://github.com/trillek-team/trillek-computer/blob/master/TR3200.md
 */
#ifndef __TR3200_OPCODES_HPP_
#define __TR3200_OPCODES_HPP_ 1

namespace trillek {
namespace computer {

// 3 Parameters OpCodes *******************************************************

/**
 * 3 parameters OpCodes
 */
enum P3_OPCODE {
    AND  = 0x80,
    OR   = 0x81,
    XOR  = 0x82,
    BITC = 0x83,

    ADD  = 0x84,
    ADDC = 0x85,
    SUB  = 0x86,
    SUBB = 0x87,
    RSB  = 0x88,
    RSBB = 0x89,

    LLS  = 0x8A,
    RLS  = 0x8B,
    ARS  = 0x8C,
    ROTL = 0x8D,
    ROTR = 0x8E,

    MUL  = 0x8F,
    SMUL = 0x90,
    DIV  = 0x91,
    SDIV = 0x92,

    LOAD  = 0x93,
    LOADW = 0x94,
    LOADB = 0x95,

    STORE  = 0x96,
    STOREW = 0x97,
    STOREB = 0x98,
};

// 2 Parameters OpCodes *******************************************************

/**
 * 2 parameter OpCodes
 */
enum P2_OPCODE {
    MOV = 0x40,
    SWP = 0x41,

    NOT = 0x42,

    SIGXB = 0x43,
    SIGXW = 0x44,

    LOAD2  = 0x45,
    LOADW2 = 0x46,
    LOADB2 = 0x47,

    STORE2  = 0x48,
    STOREW2 = 0x49,
    STOREB2 = 0x4A,

    JMP2  = 0x4B,
    CALL2 = 0x4C,

    IFEQ  = 0x70,
    IFNEQ = 0x71,

    IFL   = 0x72,
    IFSL  = 0x73,
    IFLE  = 0x74,
    IFSLE = 0x75,

    IFG   = 0x76,
    IFSG  = 0x77,
    IFGE  = 0x78,
    IFSGE = 0x79,

    IFBITS  = 0x7A,
    IFCLEAR = 0x7B,

};

// 1 Parameter OpCodes ********************************************************

/**
 * 1 Parameter OpCodes
 */
enum P1_OPCODE {

    XCHGB = 0x20,
    XCHGW = 0x21,

    GETPC = 0x22,

    POP  = 0x23,
    PUSH = 0x24,

    JMP  = 0x25,
    CALL = 0x26,

    RJMP  = 0x27,
    RCALL = 0x28,

    INT = 0x29,
};

// 0 Parameters OpCodes ********************************************************

/**
 * 0 Paramaters OpCodes
 */
enum NP_OPCODE {
    SLEEP = 0x00,

    RET = 0x01,
    RFI = 0x02,
};

} // End of namespace computer
} // End of namespace trillek

#endif // __TR3200_OPCODES_HPP_
