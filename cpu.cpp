#include "cpu.hpp"
#include "rc1600_opcodes.hpp"

#include <iostream>
#include <cstdio>
#include <algorithm>

namespace CPU {

RC1600::RC1600()
{
    reset();    
}

RC1600::~RC1600()
{ }

void RC1600::reset()
{
    std::fill_n(state.r, 15, 0);

    state.pc = state.cs = state.ia = state.is = 0;
    state.ss = state.ds = state.flags = state.y = 0;

    state.wait_cycles = 0;

    state.skiping = false;
    state.sleeping = false;

    state.interrupt = false;
    state.int_msg = 0;
    state.iacq = false;
}

unsigned RC1600::step()
{
   return realStep();
}

void RC1600::tick (unsigned n)
{
    for (unsigned i=0; i < n; i++) {
        if (state.wait_cycles <=0) {
            realStep();
        }

        state.wait_cycles--;
    }
}

#define GRAB_NEXT_WORD_LITERAL                                                \
{                                                                             \
    if (reg2 == 0xF) { /* Literal comes from the next word */                 \
        epc = ((state.cs&0x0F) << 16) | state.pc;                             \
        if (state.pc >= 0xFFFE) { /* Auto increment of CS */                  \
            state.cs++;                                                       \
            state.pc -= 0xFFFE;                                               \
        }                                                                     \
        state.pc +=2;                                                         \
        reg2 = (ram[epc+1] << 8) | ram[epc]; /* Big Endian */                 \
        state.wait_cycles++;                                                  \
    }                                                                         \
}

#define CARRY_BIT(x)  ((((x) >> 16) &0x1) == 1)

/**
 * Executes a RC1600 instrucction
 * @return Number of cycles that takes to do it
 */
unsigned RC1600::realStep()
{
    state.wait_cycles = 0;

    // TODO Check if is sleeping. If is sleeping, cheks that a interrupt is not
    // hapening. If is not happening, then skips all. If happens, removes the
    // sleeping flag and does the interrupt

    dword_t esp;
    dword_t epc = ((state.cs&0x0F) << 16) | state.pc;
    if (state.pc >= 0xFFFE) { // Auto increment of CS
        state.cs++;
        state.pc -= 0xFFFE;
    }
    state.pc +=2;
    word_t inst = (ram[epc+1] << 8) | ram[epc]; // Big Endian
    //std::printf("### 0x%04x \n", inst);
    word_t opcode, reg1, reg2, reg3;

    word_t tmp1, tmp2;
    dword_t ptr;
    uint_fast32_t tmp3;


    reg3 = inst & 0xF;
    reg2 = (inst >> 4) & 0xF;
    reg1 = (inst >> 8) & 0xF;
    std::printf("### reg3 %x reg2 %x reg1 %x\n", reg3, reg2, reg1);
    // Check the type of instrucction
    if (IS_PAR3(inst)) {
        std::cout << "## Type Par3" << std::endl;
        // 3 parameter instrucction
        opcode = (inst >> 12) & 7;
        state.wait_cycles += 4;
        switch (opcode) {
            // READ -----------------------------------------------------------
            case PAR3_OPCODE::LOAD_LIT :
                GRAB_NEXT_WORD_LITERAL;
                if (reg3 == 0) // We cant write on r0
                    break;
                tmp1 = (state.r[reg1] + reg2)&0xFFFF;
                if (reg1 == BP || reg1 == SP ) {
                    ptr = ((state.ss&0xF) << 16) | tmp1;
                } else {
                    ptr = ((state.ds&0xF) << 16) | tmp1;
                }
                state.r[reg3] = (ram[ptr+1] << 8) | ram[ptr];
                break;

            case PAR3_OPCODE::LOAD :
                if (reg3 == 0) // We cant write on r0
                    break;
                tmp1 = (state.r[reg1] + state.r[reg2])&0xFFFF;
                if (reg1 == BP || reg1 == SP ) {
                    ptr = ((state.ss&0xF) << 16) | tmp1;
                } else {
                    ptr = ((state.ds&0xF) << 16) | tmp1;
                }
                state.r[reg3] = (ram[ptr+1] << 8) | ram[ptr];
                break;

            case PAR3_OPCODE::LOADB_LIT :
                GRAB_NEXT_WORD_LITERAL;
                if (reg3 == 0) // We cant write on r0
                    break;
                tmp1 = (state.r[reg1] + reg2)&0xFFFF;
                if (reg1 == BP || reg1 == SP ) {
                    ptr = ((state.ss&0xF) << 16) | tmp1;
                } else {
                    ptr = ((state.ds&0xF) << 16) | tmp1;
                }
                state.r[reg3] &= 0xFF00; // Not overwrite MSB
                state.r[reg3] |= ram[ptr];
                break;

            case PAR3_OPCODE::LOADB :
                if (reg3 == 0) // We cant write on r0
                    break;
                tmp1 = (state.r[reg1] + state.r[reg2])&0xFFFF;
                if (reg1 == BP || reg1 == SP ) {
                    ptr = ((state.ss&0xF) << 16) | tmp1;
                } else {
                    ptr = ((state.ds&0xF) << 16) | tmp1;
                }
                state.r[reg3] &= 0xFF00; // Not overwrite MSB
                state.r[reg3] |= ram[ptr];
                break;

            // Write ----------------------------------------------------------
            case PAR3_OPCODE::STORE_LIT :
                GRAB_NEXT_WORD_LITERAL;
                tmp1 = (state.r[reg1] + reg2)&0xFFFF;
                if (reg1 == BP || reg1 == SP ) {
                    ptr = ((state.ss&0xF) << 16) | tmp1;
                } else {
                    ptr = ((state.ds&0xF) << 16) | tmp1;
                }
                ram[ptr] = state.r[reg3] & 0x00FF;
                ram[ptr+1] = state.r[reg3] >> 8;
                break;

            case PAR3_OPCODE::STORE :
                tmp1 = (state.r[reg1] + state.r[reg2])&0xFFFF;
                if (reg1 == BP || reg1 == SP ) {
                    ptr = ((state.ss&0xF) << 16) | tmp1;
                } else {
                    ptr = ((state.ds&0xF) << 16) | tmp1;
                }
                ram[ptr] = state.r[reg3] & 0x00FF;
                ram[ptr+1] = state.r[reg3] >> 8;
                break;

            case PAR3_OPCODE::STOREB_LIT :
                GRAB_NEXT_WORD_LITERAL;
                tmp1 = (state.r[reg1] + reg2)&0xFFFF;
                if (reg1 == BP || reg1 == SP ) {
                    ptr = ((state.ss&0xF) << 16) | tmp1;
                } else {
                    ptr = ((state.ds&0xF) << 16) | tmp1;
                }
                ram[ptr] = state.r[reg3];
                break;

            case PAR3_OPCODE::STOREB :
                tmp1 = (state.r[reg1] + state.r[reg2])&0xFFFF;
                if (reg1 == BP || reg1 == SP ) {
                    ptr = ((state.ss&0xF) << 16) | tmp1;
                } else {
                    ptr = ((state.ds&0xF) << 16) | tmp1;
                }
                ram[ptr] = state.r[reg3];
                break;
       
            default:
                break; // Uhh this not should hapen
        }

    } else if (IS_PAR2(inst)) {
        std::cout << "## Type Par2" << std::endl;
        // 2 parameter instrucction
        opcode = (inst >> 8) & 0x3F;

        switch (opcode) {
            case PAR2_OPCODE::ADD :
                state.wait_cycles +=3;
                tmp3 = (uint_fast32_t)(state.r[reg3]) + state.r[reg2];
                if (CARRY_BIT(tmp3))
                    SET_ON_CF(state.flags);
                else
                    SET_OFF_CF(state.flags);

                tmp1 = (state.r[reg3] >> 15) & 0x1; // gets Sign bits
                tmp2 = (state.r[reg2] >> 15) & 0x1;
                if (tmp1 == tmp2) { // Check sign of result
                    tmp2 = (tmp3 >> 15) & 0x1;
                    if (tmp1 != tmp2) { // Overflow
                        SET_ON_OF(state.flags);
                        if (GET_TOE(state.flags)) {
                            state.interrupt = true;
                            state.int_msg = 4;
                        }
                    } else {
                        SET_OFF_OF(state.flags);
                    }
                }

                state.r[reg3] = (word_t)(tmp3 & 0xFFFF);
                break;

            case PAR2_OPCODE::ADD_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL;
                tmp3 = (uint_fast32_t)(state.r[reg3]) + reg2;
                if (CARRY_BIT(tmp3) != 0)
                    SET_ON_CF(state.flags);
                else
                    SET_OFF_CF(state.flags);

                tmp1 = (state.r[reg3] >> 15) & 0x1; // gets Sign bits
                tmp2 = (reg2 >> 15) & 0x1;
                if (tmp1 == tmp2) { // Check sign of result
                    tmp2 = (tmp3 >> 15) & 0x1;
                    if (tmp1 != tmp2) { // Overflow
                        SET_ON_OF(state.flags);
                        if (GET_TOE(state.flags)) {
                            state.interrupt = true;
                            state.int_msg = 4;
                        }
                    } else {
                        SET_OFF_OF(state.flags);
                    }
                }

                state.r[reg3] = (word_t)(tmp3 & 0xFFFF);
                break;

            case PAR2_OPCODE::SUB :
                state.wait_cycles +=3;
                tmp3 = (uint_fast32_t)(state.r[reg3]) - state.r[reg2];
                if (state.r[reg3] < state.r[reg2])
                    SET_ON_CF(state.flags);
                else
                    SET_OFF_CF(state.flags);

                tmp1 = (state.r[reg3] >> 15) & 0x1; // gets Sign bits
                tmp2 = (state.r[reg2] >> 15) & 0x1;
                if (tmp1 == tmp2) { // Check sign of result
                    tmp2 = (tmp3 >> 15) & 0x1;
                    if (tmp1 != tmp2) { // Overflow
                        SET_ON_OF(state.flags);
                        if (GET_TOE(state.flags)) {
                            state.interrupt = true;
                            state.int_msg = 4;
                        }
                    } else {
                        SET_OFF_OF(state.flags);
                    }
                }

                state.r[reg3] = (word_t)(tmp3 & 0xFFFF);
                break;

            case PAR2_OPCODE::SUB_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL;
                tmp3 = (uint_fast32_t)(state.r[reg3]) - reg2;
                if (state.r[reg3] < reg2)
                    SET_ON_CF(state.flags);
                else
                    SET_OFF_CF(state.flags);

                tmp1 = (state.r[reg3] >> 15) & 0x1; // gets Sign bits
                tmp2 = (reg2 >> 15) & 0x1;
                if (tmp1 == tmp2) { // Check sign of result
                    tmp2 = (tmp3 >> 15) & 0x1;
                    if (tmp1 != tmp2) { // Overflow
                        SET_ON_OF(state.flags);
                        if (GET_TOE(state.flags)) {
                            state.interrupt = true;
                            state.int_msg = 4;
                        }
                    } else {
                        SET_OFF_OF(state.flags);
                    }
                }

                state.r[reg3] = (word_t)(tmp3 & 0xFFFF);
                break;

            // TODO

            case PAR2_OPCODE::SWP :
                state.wait_cycles += 3;
                tmp1 = state.r[reg2];
                state.r[reg2] = state.r[reg3];
                state.r[reg3] = tmp1;
                break;

            case PAR2_OPCODE::CPY :
                state.wait_cycles += 2;
                state.r[reg3] = state.r[reg2];
                break;

            case PAR2_OPCODE::SET :
                state.wait_cycles += 2;
                GRAB_NEXT_WORD_LITERAL;
                state.r[reg3] = reg2;
                break;

            // TODO

            default: // Acts like a NOP
                state.wait_cycles++;
                break;

        }

    } else if (IS_PAR1(inst)) {
        std::cout << "## Type Par1" << std::endl;
        // 1 parameter instrucction
        opcode = (inst >> 4) & 0x1FF;

        switch (opcode) {
            case PAR1_OPCODE::NOT :
                state.wait_cycles += 3;
                state.r[reg3] = ~state.r[reg3];
                break;
            
            case PAR1_OPCODE::NEG :
                state.wait_cycles += 3;
                state.r[reg3] = ~state.r[reg3] +1;
                break;

            case PAR1_OPCODE::XCHG :
                state.wait_cycles += 3;
                tmp1 = state.r[reg3] & 0x00FF;
                state.r[reg3] = (state.r[reg3] >> 8) | (tmp1 << 8);
                break;

            case PAR1_OPCODE::SXTBD :
                state.wait_cycles += 3;
                if ((state.r[reg3] & 0x0080) != 0) {
                    state.r[reg3] |= 0xFF00; // Negative
                } else {
                    state.r[reg3] &= 0x00FF; // Positive
                }
                break;

            default: // Acts like a NOP
                state.wait_cycles++;
                break;
        }

    } else if (IS_PAR1L(inst)) {
        std::cout << "## Type Par1L" << std::endl;
        // 1 parameter that is a long literal
        opcode = (inst >> 9) & 0x7;
        word_t long_lit = inst & 0x1FF;
        int_fast32_t relpos;
        switch (opcode) {
            case PAR1L_OPCODE::JMP_REL :
                state.wait_cycles +=3;
                relpos = state.pc + long_lit;
                if (relpos > 0xFFFF) {
                    state.cs++;
                    state.pc = (word_t)(relpos - 0xFFFF);
                } else if (relpos < 0) {
                    state.cs--;
                    state.pc = (word_t)(0xFFFF + relpos);
                } else {
                    state.pc = (word_t)(relpos &0xFFFF);
                }
                break;

            case PAR1L_OPCODE::CALL_REL : 
                state.wait_cycles +=4;
                // Push PC
                esp = ((state.ss&0x0F) << 16) | --state.r[SP];
                ram[esp] = state.pc; 

                relpos = state.pc + long_lit;
                if (relpos > 0xFFFF) {
                    state.cs++;
                    state.pc = (word_t)(relpos - 0xFFFF);
                } else if (tmp3 < 0) {
                    state.cs--;
                    state.pc = (word_t)(0xFFFF + relpos);
                } else {
                    state.pc = (word_t)(relpos &0xFFFF);
                }

                break;

            default: // Acts like a NOP
                state.wait_cycles++;
                break;

        }

    } else if (IS_NOPAR(inst)) {
        std::cout << "## Type NoPar" << std::endl;
        // Instrucctions without parameters
        opcode = inst & 0xFFF;

        switch (opcode) {
            case NOPAR_OPCODE::NOP :
                state.wait_cycles++;
                break;

            case NOPAR_OPCODE::SLEEP :
                state.wait_cycles++;
                state.sleeping = true;
                break;


            case NOPAR_OPCODE::RET :
                state.wait_cycles +=4;
                // Pop PC
                esp = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.pc = ram[esp];
                break;
            
            case NOPAR_OPCODE::LRET :
                state.wait_cycles +=5;
                // Pop PC
                esp = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.pc = ram[esp];
                // Pop CS
                esp = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.cs = ram[esp];
                break;

            case NOPAR_OPCODE::RFI :
                state.wait_cycles +=8;
                // Pop R1
                esp = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.r[1] = ram[esp];
                // Pop PC
                esp = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.pc = ram[esp];
                // Pop CS
                esp = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.cs = ram[esp];
                SET_OFF_IF(state.flags);
                state.iacq = false;
                break;

            // TODO
            default: // Acts like a NOP
                state.wait_cycles++;
                break;

        }
    }

    state.r[0] = 0; // r0 always is 0
    return state.wait_cycles;
    
}

}
