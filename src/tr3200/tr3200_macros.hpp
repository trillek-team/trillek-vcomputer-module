/**
 * \brief       TR3200 CPU Macros
 * \file        ITR3200_macros.hpp
 * \copyright   The MIT License (MIT)
 *
 * Macros used by TR3200 cpu implementation
 */
#ifndef __TR3200_MACROS_HPP_
#define __TR3200_MACROS_HPP_ 1

// Alias to special registers
#define RY                  (11)
#define BP                  (12)
#define SP                  (13)
#define IA                  (14)
#define FLAGS               (15)

// Instrucction types
#define IS_P3(x)            ( ( (x) & 0x80000000 ) == 0x80000000 )
#define IS_P2(x)            ( ( (x) & 0xC0000000 ) == 0x40000000 )
#define IS_P1(x)            ( ( (x) & 0xE0000000 ) == 0x20000000 )
#define IS_NP(x)            ( ( (x) & 0xE0000000 ) == 0x00000000 )

// Instruction OpCode
#define GET_OP_CODE(x)      ( ( (x) >> 24 ) & 0xFF )

// Instrucction sub-type
#define IS_BRANCH(x)        ( ( (x) >= 0x4B ) && ( (x) <= 0x52 ) )

// Uses immediate value (M bit) ?
#define HAVE_IMMEDIATE(x)   ( ( (x) & 0x00800000) != 0 )
// Uses next dword as literal
#define IS_BIG_LITERAL(x)   ( ( (x) & 0x00C00000) == 0x00C00000)

// Extract operands
// Register
#define GRD(x)              ( ( (x) >> 18 ) & 0x0F )
#define GRS(x)              ( ( (x) >> 14 ) & 0x0F )
#define GRN(x)              (   (x)         & 0x0F )
// Short immediate valie
#define LIT14(x)            (   (x) &   0x3FFF )
#define LIT18(x)            (   (x) &  0x3FFFF )
#define LIT22(x)            (   (x) & 0x3FFFFF )

// Macros for ALU operations
#define CARRY_BIT(x)        ( ( ( (x) >> 32 ) & 0x1 ) == 1 )
#define DW_SIGN_BIT(x)      ( ( (x) >> 31 ) & 0x1 )
#define W_SIGN_BIT(x)       ( ( (x) >> 15 ) & 0x1 )
#define B_SIGN_BIT(x)       ( ( (x) >> 7 )  & 0x1 )

// Extend sign of an short immediate value
#define NEG_LIT14(x)        ( (x) | 0xFFFFC000)
#define NEG_LIT18(x)        ( (x) | 0xFFFC0000)
#define NEG_LIT22(x)        ( (x) | 0xFFC00000)
#define SIGN_LIT14(x)       ( ( (x) >> 13 ) & 0x1 )
#define SIGN_LIT18(x)       ( ( (x) >> 17 ) & 0x1 )
#define SIGN_LIT22(x)       ( ( (x) >> 21 ) & 0x1 )

// Operation in Flags bits
#define GET_CF(x)           ( (x) & 0x1 )
#define SET_ON_CF(x)        (x |= 0x1)
#define SET_OFF_CF(x)       (x &= 0xFFFFFFFE)

#define GET_OF(x)           ( ( (x) & 0x2 ) >> 1 )
#define SET_ON_OF(x)        (x |= 0x2)
#define SET_OFF_OF(x)       (x &= 0xFFFFFFFD)

#define GET_DE(x)           ( ( (x) & 0x4 ) >> 2 )
#define SET_ON_DE(x)        (x |= 0x4)
#define SET_OFF_DE(x)       (x &= 0xFFFFFFFB)

#define GET_IF(x)           ( ( (x) & 0x8 ) >> 3 )
#define SET_ON_IF(x)        (x |= 0x8)
#define SET_OFF_IF(x)       (x &= 0xFFFFFFF7)

// Enable bits that change what does the CPU
#define GET_EI(x)           ( ( (x) & 0x100 ) >> 8 )
#define SET_ON_EI(x)        (x |= 0x100)
#define SET_OFF_EI(x)       (x &= 0xFFFFFEFF)

#define GET_ESS(x)          ( ( (x) & 0x200 ) >> 9 )
#define SET_ON_ESS(x)       (x |= 0x200)
#define SET_OFF_ESS(x)      (x &= 0xFFFFFDFF)

// Internal alias to Y Flags and IA registers
#define REG_Y               r[RY]
#define REG_IA              r[IA]
#define REG_FLAGS           r[FLAGS]

#endif // __TR3200_MACROS_HPP_
