#pragma once
/**
 * RC3200 VM - RC3200 opcodes for RC3200 v7
 * CPU of the virtual machine
 */

#ifndef __RC3200_OPCODES__
#define __RC3200_OPCODES__ 1

// Alias to BP and SP registers
#define BP (30)
#define SP (31)

// Alias to Ym Flags and IA registers
#define Y       state.r[27]
#define IA      state.r[28]
#define FLAGS   state.r[29]


/// Instrucction types
#define IS_PAR3(x)  (((x) & 0xC0000000) == 0x40000000 )
#define IS_PAR2(x)  (((x) & 0x80000000) == 0x80000000 )
#define IS_PAR1(x)  (((x) & 0xE0000000) == 0x20000000 )
#define IS_NP(x)    (((x) & 0xE0000000) == 0x00000000 )

/// Instrucction sub-type
#define IS_BRANCH(x)    (((x) & 0xE0000000) == 0xA0000000 )

/// Uses a Literal value ?
#define HAVE_LITERAL(x)     (((x) & 0x00800000) != 0)

/// Extract operands
#define GRD(x)              ( (x)       & 0x1F) 
#define GRS(x)              (((x) >> 5) & 0x1F) 
#define GRN(x)              (((x) >> 10)& 0x1F) 

#define LIT13(x)            (((x) >> 10)& 0x1FFF) 
#define LIT18(x)            (((x) >> 5) & 0x3FFFF) 
#define LIT22(x)            ( (x)       & 0x7FFFFF) 

/// Uses next dword as literal
#define IS_BIG_LITERAL_L13(x)   ((x) == 0x1000)
#define IS_BIG_LITERAL_L18(x)   ((x) == 0x20000)
#define IS_BIG_LITERAL_L22(x)   ((x) == 0x400000)

// Macros for ALU operations
#define CARRY_BIT(x)        ((((x) >> 32) & 0x1) == 1)
#define DW_SIGN_BIT(x)      ( ((x) >> 31) & 0x1)
#define W_SIGN_BIT(x)       ( ((x) >> 15) & 0x1)
#define B_SIGN_BIT(x)       ( ((x) >> 7)  & 0x1)

// Extract sign of Literal Operator
#define O13_SIGN_BIT(x)     (((x) >> 12)  & 0x1)
#define O18_SIGN_BIT(x)     (((x) >> 17)  & 0x1)
#define O22_SIGN_BIT(x)     (((x) >> 21)  & 0x1)

namespace vm {
namespace cpu {


// 3 Parameters OpCodes *******************************************************

/**
 * 3 parameters OpCodes 
 */
enum P3_OPCODE {
    AND             = 0x00,
    OR              = 0x01,
    XOR             = 0x02,
    BITC            = 0x03,

    ADD             = 0x04,
    ADDC            = 0x05,
    SUB             = 0x06,
    SUBB            = 0x07,
    RSB             = 0x08,
    RSBB            = 0x09,

    LLS             = 0x0A,
    RLS             = 0x0B,
    ARS             = 0x0C,
    ROTL            = 0x0D,
    ROTR            = 0x0E,

    MUL             = 0x10,
    SMUL            = 0x11,
    DIV             = 0x12,
    SDIV            = 0x13,

    LOAD            = 0x20,
    LOADW           = 0x21,
    LOADB           = 0x22,

    STORE           = 0x24,
    STOREW          = 0x25,
    STOREB          = 0x26,
};

// 2 Parameters OpCodes *******************************************************

/**
 * 2 parameter OpCodes
 */
enum P2_OPCODE {
    MOV             = 0x00,
    SWP             = 0x01,

    SIGXB           = 0x02,
    SIGXW           = 0x03,

    NOT             = 0x04,

    LOAD2           = 0x40,
    LOADW2          = 0x41,
    LOADB2          = 0x42,

    STORE2          = 0x44,
    STOREW2         = 0x45,
    STOREB2         = 0x46,

    IFEQ            = 0x20,
    IFNEQ           = 0x21,

    IFL             = 0x22,
    IFSL            = 0x23,
    IFLE            = 0x24,
    IFSLE           = 0x25,

    IFBITS          = 0x26,
    IFCLEAR         = 0x27,

    JMP2            = 0x60,
    CALL2           = 0x61,
};

// 1 Parameter OpCodes ********************************************************

/**
 * 1 Parameter OpCodes
 */
enum P1_OPCODE {

    XCHGB           = 0x00,
    XCHGW           = 0x01,

    GETPC           = 0x02,

    POP             = 0x09,
    PUSH            = 0x0A,

    JMP             = 0x18,
    CALL            = 0x19,

    RJMP            = 0x1A,
    RCALL           = 0x1B,

    INT             = 0x1C,
};

// 0 Parameters OpCodes ********************************************************

/**
 * 0 Paramaters OpCodes
 */
enum NP_OPCODE {
    SLEEP           = 0x00000000,
    
    RET             = 0x00000004,
    RFI             = 0x00000005,
};


} // End of namespace cpu
} // End of namespace vm

#endif // __RC3200_OPCODES__
