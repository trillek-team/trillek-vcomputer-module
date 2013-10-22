#pragma once

#ifndef __RC3200_OPCODES__
#define __RC3200_OPCODES__ 1


/// Instrucction types
#define IS_PAR3(x)  (((x) & 0xC0000000) == 0xC0000000 )
#define IS_PAR2(x)  (((x) & 0x80000000) == 0x80000000 )
#define IS_PAR1(x)  (((x) & 0x40000000) == 0x40000000 )
#define IS_NOPAR(x) (((x) & 0xC0000000) == 0)

/// Instrucction sub-types
#define IS_RAM(x)       (((x) & 0x30000000) == 0x10000000 )
#define IS_BRANCH(x)    (((x) & 0x30000000) == 0x20000000 )
#define IS_JUMP(x)      (((x) & 0x30000000) == 0x30000000 )

/// Uses a Literal value ?
#define HAVE_LITERAL(x)     (((x) & 0x8000) != 0)

/// Extract operands
#define OP1(x)              (((x) >> 10) & 0x1F) 
#define OP2(x)              (((x) >> 5) & 0x1F) 
#define OP3(x)              ((x) & 0x1F) 

/// Uses next dword as literal
#define IS_BIG_LITERAL_P3(x)  ((x) == 0x10)
#define IS_BIG_LITERAL_P2(x)  ((x) == 0x200)
#define IS_BIG_LITERAL_P1(x)  ((x) == 0x400000)

// Macros for ALU operations
#define CARRY_BIT(x)    ((((x) >> 32) & 0x1) == 1)
#define DW_SIGN_BIT(x)  (((x) >> 31)  & 0x1)
#define W_SIGN_BIT(x)   (((x) >> 15)  & 0x1)
#define B_SIGN_BIT(x)   (((x) >> 7)   & 0x1)

// Extract sign of Literal Operator
#define O5_SIGN_BIT(x)  (((x) >> 4)   & 0x1)
#define O10_SIGN_BIT(x) (((x) >> 9)   & 0x1)
#define O15_SIGN_BIT(x) (((x) >> 14)  & 0x1)

namespace vm {
namespace cpu {


/**
 * OpCodes that are to access RAM
 */
enum OPCODE_RAM {
    LOAD            = 0,
    LOADW           = 1,
    LOADB           = 2,

    STORE           = 3,
    STOREW          = 4,
    STOREB          = 5,

    PUSH            = 6,
    POP             = 7,
};

/**
 * OpCodes that does jump
 */
enum OPCODE_JUMP {
    INT             = 0,
    JMP             = 1,
    CALL            = 2,
    RFI             = 3,
    RET             = 4,
};

/**
 * OpCodes that does branching
 */
enum OPCODE_BRANCH {
    IFEQ             = 0,   
    IFNEQ            = 1,  

    IFG              = 2,
    IFUG             = 3,
    IFGE             = 4,
    IFUGE            = 5,
    
    IFL              = 6,
    IFUL             = 7,
    IFLE             = 8,
    IFULE            = 9,
    
    IFBITS           = 10,
    IFCLEAR          = 11,
    
    IFOF             = 12,
    IFCF             = 13,
};

// 3 Parameters OpCodes *******************************************************

/**
 * 3 parameters OpCodes that does ALU operations
 */
enum PAR3_OPCODE_ALU {
    ADD             = 0,
    SUB             = 1,
    ADDC            = 2,
    SUBB            = 3,

    AND             = 4,
    OR              = 5,
    XOR             = 6,
    NAND            = 7,

    SLL             = 8,
    SRL             = 9,
    SRA             = 10,
    ROTL            = 11,
    ROTR            = 12,

    UMUL            = 13,
    UDIV            = 14,
    MUL             = 15,
    DIV             = 16,
};

// 2 Parameters OpCodes *******************************************************

/**
 * 2 parameter OpCodes that manipulates registers
 */
enum PAR2_OPCODE {
    CPY_SET         = 0,
    SWP             = 1,
};

// 1 Parameter OpCodes ********************************************************

/**
 * 1 Parameter OpCodes that manipulates registers
 */
enum PAR1_OPCODE {
    NOT             = 0,
    NEG             = 1,

    XCHG            = 2,
    XCHGW           = 3,
    SXTBD           = 4,
    SXTWD           = 5,

    GETPC           = 8,

    SETFLAGS        = 9,
    GETFLAGS        = 10,

    SETY            = 11,
    GETY            = 12,

    SETIA           = 13, 
    GETIA           = 14,
};

// 0 Parameters OpCodes ********************************************************

/**
 * 0 Paramaters generic OpCodes
 */
enum NOPAR_OPCODE {
    NOP             = 1,
    SLEEP           = 0,
};


} // End of namespace cpu
} // End of namespace vm

#endif // __RC3200_OPCODES__
