#pragma once
/**
 * Trillek Virtual Computer - TR3200_macros.hpp
 * Macros used by TR3200 implementation
 */

#ifndef __TR3200_MACROS_HPP__
#define __TR3200_MACROS_HPP__

// Alias to special registers
#define REG_Y			(11)
#define BP				(12)
#define SP				(13)
#define REG_IA		(14)
#define REG_FLAGS (15)

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
#define GRD(x)              ( (x)       & 0x0F) 
#define GRS(x)              (((x) >> 5) & 0x0F) 
#define GRN(x)              (((x) >> 10)& 0x0F) 

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

// Operation in Flags bits
#define GET_CF(x)          ((x) & 0x1)
#define SET_ON_CF(x)       (x |= 0x1)
#define SET_OFF_CF(x)      (x &= 0xFFFFFFFE)

#define GET_OF(x)          (((x) & 0x2) >> 1)
#define SET_ON_OF(x)       (x |= 0x2)
#define SET_OFF_OF(x)      (x &= 0xFFFFFFFD)

#define GET_DE(x)          (((x) & 0x4) >> 2)
#define SET_ON_DE(x)       (x |= 0x4)
#define SET_OFF_DE(x)      (x &= 0xFFFFFFFB)

#define GET_IF(x)          (((x) & 0x8) >> 3)
#define SET_ON_IF(x)       (x |= 0x8)
#define SET_OFF_IF(x)      (x &= 0xFFFFFFF7)

// Enable bits that change what does the CPU
#define GET_EI(x)          (((x) & 0x100) >> 8)
#define SET_ON_EI(x)       (x |= 0x100)
#define SET_OFF_EI(x)      (x &= 0xFFFFFEFF)

#define GET_ESS(x)         (((x) & 0x200) >> 9)
#define SET_ON_ESS(x)      (x |= 0x200)
#define SET_OFF_ESS(x)     (x &= 0xFFFFFDFF)

// Internal alias to Y Flags and IA registers
#define RY      r[REG_Y]
#define IA      r[REG_IA]
#define FLAGS   r[REG_FLAGS]


#endif // __TR3200_MACROS_HPP__

