#include "cpu.hpp"
#include "rc1600_opcodes.hpp"

#include <iostream>
#include <cstdio>
#include <algorithm>

namespace CPU {

RC1600::RC1600() : tot_cycles(0)
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
    if (!state.sleeping) {
        auto cyc = realStep();
        tot_cycles += cyc;
        return cyc;
    } else {
        tot_cycles++;
        processInterrupt();
        return 1;
    }
}

void RC1600::tick (unsigned n)
{
    for (unsigned i=0; i < n; i++) {
        if (state.wait_cycles <=0 && !state.sleeping) {
            realStep();
        }
        
        if (!state.sleeping)
            state.wait_cycles--;
        else
            processInterrupt();

        tot_cycles++;
    }
}

#define GRAB_NEXT_WORD_LITERAL(R)                                             \
{                                                                             \
    if (R == 0xF) { /* Literal comes from the next word */                    \
        uint32_t epc = ((state.cs&0x0F) << 16) | state.pc;                    \
        if (state.pc >= 0xFFFE) { /* Auto increment of CS */                  \
            state.cs++;                                                       \
            state.pc -= 0xFFFE;                                               \
        }                                                                     \
        state.pc +=2;                                                         \
        R = (ram[epc+1] << 8) | ram[epc]; /* Big Endian */                    \
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
    
    processInterrupt(); // Here we check if a interrupt happens

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
    // Check if we are skiping a instrucction
    if (state.skiping) {
        state.wait_cycles = 1;
        // See what kind of instrucction is to know how many should
        // increment PC, and remove skiping flag if is not a branch
        if (IS_PAR3(inst)) {
            // 3 parameter instrucction
            opcode = (inst >> 12) & 7;
            switch (opcode) {
                case PAR3_OPCODE::LOAD_LIT:
                case PAR3_OPCODE::LOADB_LIT:
                case PAR3_OPCODE::STORE_LIT:
                case PAR3_OPCODE::STOREB_LIT:
                    GRAB_NEXT_WORD_LITERAL(reg2);
                    break;

                default:
                    break;
            }
            state.skiping = false;
        } else if (IS_PAR2(inst)) {
            // 2 parameter instrucction
            opcode = (inst >> 8) & 0x3F;
            switch (opcode) {
                case PAR2_OPCODE::ADD_LIT :
                case PAR2_OPCODE::SUB_LIT :
                case PAR2_OPCODE::SUBC_LIT :
                case PAR2_OPCODE::SLL_LIT :
                case PAR2_OPCODE::SRL_LIT :
                case PAR2_OPCODE::SRA_LIT :
                case PAR2_OPCODE::SET :
                case PAR2_OPCODE::INP_LIT :
                case PAR2_OPCODE::OUT_LIT :
                case PAR2_OPCODE::JMP :
                case PAR2_OPCODE::CALL :
                    GRAB_NEXT_WORD_LITERAL(reg2);
                    state.skiping = false;
                    break;
               
                case PAR2_OPCODE::BEQ_LIT :
                case PAR2_OPCODE::BNEQ_LIT :
                case PAR2_OPCODE::BG_LIT :
                case PAR2_OPCODE::BGE_LIT :
                case PAR2_OPCODE::BUG_LIT :
                case PAR2_OPCODE::BUGE_LIT :
                    GRAB_NEXT_WORD_LITERAL(reg2);
                
                case PAR2_OPCODE::BEQ:
                case PAR2_OPCODE::BNEQ:
                case PAR2_OPCODE::BG:
                case PAR2_OPCODE::BGE:
                case PAR2_OPCODE::BUG:
                case PAR2_OPCODE::BUGE:
                case PAR2_OPCODE::BBITS:
                case PAR2_OPCODE::BCLEAR:
                    state.skiping = true;
                    break;

                default:
                    state.skiping = false;
                    break;
            }
        } else if (IS_PAR1(inst)) {
            // 1 parameter instrucction
            opcode = (inst >> 4) & 0x1FF;
            switch (opcode) {
                case PAR1_OPCODE::SETY_LIT :
                case PAR1_OPCODE::INT_LIT  :
                case PAR1_OPCODE::SETIA_LIT  :
                case PAR1_OPCODE::SETCS_LIT  :
                case PAR1_OPCODE::SETDS_LIT  :
                case PAR1_OPCODE::SETSS_LIT  :
                case PAR1_OPCODE::SETIS_LIT  :
                    GRAB_NEXT_WORD_LITERAL(reg2);
                    break;

                default:
                    break;
            }
            state.skiping = false;
        } else if (IS_NOPAR(inst) ) {
            // Instrucctions without parameters
            opcode = inst & 0xFFF;
            switch (opcode) {
                case NOPAR_OPCODE::BOVF :
                case NOPAR_OPCODE::BOCF :
                    state.skiping = true;
                    break;

                default:
                    state.skiping = false;
                    break;
            }
        }
        return state.wait_cycles;
    }

    // Check the type of instrucction
    if (IS_PAR3(inst)) {
        // 3 parameter instrucction
        opcode = (inst >> 12) & 7;
        state.wait_cycles += 4;
        switch (opcode) {
            // READ -----------------------------------------------------------
            case PAR3_OPCODE::LOAD_LIT :
                GRAB_NEXT_WORD_LITERAL(reg2);
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
                GRAB_NEXT_WORD_LITERAL(reg2);
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
                GRAB_NEXT_WORD_LITERAL(reg2);
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
                GRAB_NEXT_WORD_LITERAL(reg2);
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
                            throwInterrupt(4);
                        }
                    } else {
                        SET_OFF_OF(state.flags);
                    }
                }

                state.r[reg3] = (word_t)(tmp3 & 0xFFFF);
                break;

            case PAR2_OPCODE::ADD_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg2);
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
                            throwInterrupt(4);
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
                            throwInterrupt(4);
                        }
                    } else {
                        SET_OFF_OF(state.flags);
                    }
                }

                state.r[reg3] = (word_t)(tmp3 & 0xFFFF);
                break;

            case PAR2_OPCODE::SUB_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg2);
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
                            throwInterrupt(4);
                        }
                    } else {
                        SET_OFF_OF(state.flags);
                    }
                }

                state.r[reg3] = (word_t)(tmp3 & 0xFFFF);
                break;

            case PAR2_OPCODE::ADDC :
                state.wait_cycles +=3;
                tmp3 = (uint_fast32_t)(state.r[reg3]) + state.r[reg2];
                if (GET_CF(state.flags)) // Add carry bit
                     tmp3++;

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
                            throwInterrupt(4);
                        }
                    } else {
                        SET_OFF_OF(state.flags);
                    }
                }

                state.r[reg3] = (word_t)(tmp3 & 0xFFFF);
                break;

            case PAR2_OPCODE::SUBC :
                state.wait_cycles +=3;
                tmp3 = (uint_fast32_t)(state.r[reg3]) - state.r[reg2];
                if (GET_CF(state.flags)) // Borrow bit
                    tmp3--;
                
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
                        if (GET_TOE(state.flags)) { // Overflow interrupt
                            throwInterrupt(4);
                        }
                    } else {
                        SET_OFF_OF(state.flags);
                    }
                }

                state.r[reg3] = (word_t)(tmp3 & 0xFFFF);
                break;

            case PAR2_OPCODE::SUBC_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg2);
                tmp3 = (uint_fast32_t)(state.r[reg3]) - reg2;
                if (GET_CF(state.flags)) // Borrow bit
                    tmp3--;
                
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
                            throwInterrupt(4);
                        }
                    } else {
                        SET_OFF_OF(state.flags);
                    }
                }

                state.r[reg3] = (word_t)(tmp3 & 0xFFFF);
                break;

            case PAR2_OPCODE::AND :
                state.wait_cycles +=3;
                state.r[reg3] &= state.r[reg2];
                SET_OFF_OF(state.flags);
                SET_OFF_CF(state.flags);
                break;

            case PAR2_OPCODE::OR :
                state.wait_cycles +=3;
                state.r[reg3] |= state.r[reg2];
                SET_OFF_OF(state.flags);
                SET_OFF_CF(state.flags);
                break;

            case PAR2_OPCODE::XOR :
                state.wait_cycles +=3;
                state.r[reg3] ^= state.r[reg2];
                SET_OFF_OF(state.flags);
                SET_OFF_CF(state.flags);
                break;

            case PAR2_OPCODE::SLL :
                state.wait_cycles +=3;
                state.r[reg3] = state.r[reg3] << state.r[reg2]; 
                SET_OFF_OF(state.flags);
                SET_OFF_CF(state.flags);
                break;

            case PAR2_OPCODE::SLL_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg2);
                state.r[reg3] = state.r[reg3] << reg2; 
                SET_OFF_OF(state.flags);
                SET_OFF_CF(state.flags);
                break;

            case PAR2_OPCODE::SRL :
                state.wait_cycles +=3;
                state.r[reg3] = state.r[reg3] >> state.r[reg2]; 
                SET_OFF_OF(state.flags);
                SET_OFF_CF(state.flags);
                break;

            case PAR2_OPCODE::SRL_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg2);
                state.r[reg3] = state.r[reg3] >> reg2; 
                SET_OFF_OF(state.flags);
                SET_OFF_CF(state.flags);
                break;

            case PAR2_OPCODE::SRA :
            {
                state.wait_cycles +=3;
                int16_t r3 = state.r[reg3];
                int16_t r2 = state.r[reg2];
                int16_t r = r3 << r2;
                state.r[reg3] = (word_t)(r);
                SET_OFF_OF(state.flags);
                SET_OFF_CF(state.flags);
                break;
            }

            case PAR2_OPCODE::SRA_LIT :
            {
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg2);
                int16_t r3 = state.r[reg3];
                int16_t r2 = reg2;
                int16_t r = r3 << r2;
                state.r[reg3] = (word_t)(r);
                SET_OFF_OF(state.flags);
                SET_OFF_CF(state.flags);
                break;
            }
            // TODO MUL/DIV/MOD family

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
                GRAB_NEXT_WORD_LITERAL(reg2);
                state.r[reg3] = reg2;
                break;

            // Branch --------------------------------------------------------- 
            case PAR2_OPCODE::BEQ :
                state.wait_cycles +=5;
                if (state.r[reg3] == state.r[reg2]) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;

            case PAR2_OPCODE::BEQ_LIT :
                state.wait_cycles +=5;
                GRAB_NEXT_WORD_LITERAL(reg2);
                if (state.r[reg3] == reg2) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;
            
            case PAR2_OPCODE::BNEQ :
                state.wait_cycles +=5;
                if (state.r[reg3] != state.r[reg2]) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;

            case PAR2_OPCODE::BNEQ_LIT :
                state.wait_cycles +=5;
                GRAB_NEXT_WORD_LITERAL(reg2);
                if (state.r[reg3] != reg2) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;
            
            case PAR2_OPCODE::BG :
            {
                state.wait_cycles +=5;
                int16_t r3 = state.r[reg3];
                int16_t r2 = state.r[reg2];
                if (r3 > r2) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;
            }

            case PAR2_OPCODE::BG_LIT :
            {
                state.wait_cycles +=5;
                GRAB_NEXT_WORD_LITERAL(reg2);
                int16_t r3 = state.r[reg3];
                int16_t r2 = reg2;
                if (r3 > r2) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;
            }
            
            case PAR2_OPCODE::BGE :
            {
                state.wait_cycles +=5;
                int16_t r3 = state.r[reg3];
                int16_t r2 = state.r[reg2];
                if (r3 >= r2) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;
            }

            case PAR2_OPCODE::BGE_LIT :
            {
                state.wait_cycles +=5;
                GRAB_NEXT_WORD_LITERAL(reg2);
                int16_t r3 = state.r[reg3];
                int16_t r2 = reg2;
                if (r3 >= r2) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;
            }
            
            case PAR2_OPCODE::BUG :
                state.wait_cycles +=5;
                if (state.r[reg3] > state.r[reg2]) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;

            case PAR2_OPCODE::BUG_LIT :
                state.wait_cycles +=5;
                GRAB_NEXT_WORD_LITERAL(reg2);
                if (state.r[reg3] > reg2) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;
            
            case PAR2_OPCODE::BUGE :
                state.wait_cycles +=5;
                if (state.r[reg3] >= state.r[reg2]) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;

            case PAR2_OPCODE::BUGE_LIT :
                state.wait_cycles +=5;
                GRAB_NEXT_WORD_LITERAL(reg2);
                if (state.r[reg3] >= reg2) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;
           
            case PAR2_OPCODE::BBITS :
                state.wait_cycles +=6;
                if ((state.r[reg3] & state.r[reg2]) != 0) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;
            
            case PAR2_OPCODE::BCLEAR :
                state.wait_cycles +=6;
                if ((state.r[reg3] & state.r[reg2]) == 0) {
                    state.skiping = true;
                    state.wait_cycles++;
                }
                break;

                
            // TODO INP and OUT
            
            // Absolute JuMPs and CALLs ---------------------------------------
            case PAR2_OPCODE::LJMP :
                state.wait_cycles += 3;
                state.cs = state.r[reg3];
                state.pc = state.r[reg2];
                break;
            
            case PAR2_OPCODE::LCALL :
                state.wait_cycles += 3;

                // Push CS MSB
                ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
                ram[ptr] = state.cs >> 8; 
                // Push CS LSB
                ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
                ram[ptr] = state.cs & 0xFF;
                // Push PC MSB
                ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
                ram[ptr] = state.pc >> 8; 
                // Push PC LSB
                ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
                ram[ptr] = state.pc & 0xFF;

                state.cs = state.r[reg3];
                state.pc = state.r[reg2];
                break;
            
            case PAR2_OPCODE::JMP :
                state.wait_cycles += 3;
                GRAB_NEXT_WORD_LITERAL(reg2);
                state.pc = state.r[reg3] + reg2;
                break;
            
            case PAR2_OPCODE::CALL :
                state.wait_cycles += 3;
                GRAB_NEXT_WORD_LITERAL(reg2);

                // Push PC MSB
                ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
                ram[ptr] = state.pc >> 8; 
                // Push PC LSB
                ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
                ram[ptr] = state.pc & 0xFF;

                state.pc = state.r[reg3] + reg2;
                break;

            default: // Acts like a NOP
                state.wait_cycles++;
                break;

        }

    } else if (IS_PAR1(inst)) {
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

            case PAR1_OPCODE::PUSH :
                state.wait_cycles +=3;
                // Push MSB
                std::printf("### before SP 0x%04x ", state.r[SP]);
                ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
                std::printf("   after SP 0x%04x\n", state.r[SP]);
                ram[ptr] = state.r[reg3] >> 8; 
                // Push LSB
                ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
                ram[ptr] = state.r[reg3] & 0xFF;
                break;

            case PAR1_OPCODE::POP :
                state.wait_cycles +=3;
                // Pop LSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.r[reg3] = ram[ptr];
                // Pop MSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.r[reg3] |= ram[ptr] << 8; 
                break;
            
            case PAR1_OPCODE::GETPC :
                state.wait_cycles +=2;
                state.r[reg3] = state.pc;
                break;
   
            case PAR1_OPCODE::SETFLAGS :
                state.wait_cycles +=3;
                state.flags = state.r[reg3];
                break;

            case PAR1_OPCODE::GETFLAGS :
                state.wait_cycles +=2;
                state.r[reg3] = state.flags;
                break;
    
            case PAR1_OPCODE::SETY :
                state.wait_cycles +=3;
                state.y = state.r[reg3];
                break;

            case PAR1_OPCODE::SETY_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg3);
                state.y = reg3;
                break;

            case PAR1_OPCODE::GETY :
                state.wait_cycles +=2;
                state.r[reg3] = state.y;
                break;

            // Interrupt ------------------------------------------------------
            case PAR1_OPCODE::INT :
                state.wait_cycles +=8;
                throwInterrupt(state.r[reg3]);
                break;

            case PAR1_OPCODE::INT_LIT :
                state.wait_cycles +=8;
                GRAB_NEXT_WORD_LITERAL(reg3);
                throwInterrupt(reg3);
                break;
            
                // ----------------------------------------------------------------
            case PAR1_OPCODE::SETIA :
                state.wait_cycles +=3;
                state.ia = state.r[reg3];
                break;

            case PAR1_OPCODE::SETIA_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg3);
                state.ia = reg3;
                break;

            case PAR1_OPCODE::GETIA :
                state.wait_cycles +=2;
                state.r[reg3] = state.ia;
                break;

            case PAR1_OPCODE::SETCS :
                state.wait_cycles +=3;
                state.cs = state.r[reg3];
                break;

            case PAR1_OPCODE::SETCS_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg3);
                state.cs = reg3;
                break;

            case PAR1_OPCODE::GETCS :
                state.wait_cycles +=2;
                state.r[reg3] = state.cs;
                break;

            case PAR1_OPCODE::SETDS :
                state.wait_cycles +=3;
                state.ds = state.r[reg3];
                break;

            case PAR1_OPCODE::SETDS_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg3);
                state.ds = reg3;
                break;

            case PAR1_OPCODE::GETDS :
                state.wait_cycles +=2;
                state.r[reg3] = state.ds;
                break;

            case PAR1_OPCODE::SETSS :
                state.wait_cycles +=3;
                state.ss = state.r[reg3];
                break;

            case PAR1_OPCODE::SETSS_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg3);
                state.ss = reg3;
                break;

            case PAR1_OPCODE::GETSS :
                state.wait_cycles +=2;
                state.r[reg3] = state.ss;
                break;

            case PAR1_OPCODE::SETIS :
                state.wait_cycles +=3;
                state.is = state.r[reg3];
                break;

            case PAR1_OPCODE::SETIS_LIT :
                state.wait_cycles +=3;
                GRAB_NEXT_WORD_LITERAL(reg3);
                state.is = reg3;
                break;

            case PAR1_OPCODE::GETIS :
                state.wait_cycles +=2;
                state.r[reg3] = state.is;
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
                
                // Push PC MSB
                ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
                ram[ptr] = state.pc >> 8; 
                // Push PC LSB
                ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
                ram[ptr] = state.pc & 0xFF;

                relpos = state.pc + long_lit;
                if (relpos > 0xFFFF) {
                    state.pc = 0xFFFF;
                } else if (tmp3 < 0) {
                    state.pc = 0;
                } else {
                    state.pc = (word_t)(relpos &0xFFFF);
                }

                break;

            default: // Acts like a NOP
                state.wait_cycles++;
                break;

        }

    } else if (IS_NOPAR(inst)) {
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
                // Pop PC LSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.pc = ram[ptr];
                // Pop PC MSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.pc |= ram[ptr] << 8; 
                break;
            
            case NOPAR_OPCODE::LRET :
                state.wait_cycles +=5;
                // Pop PC LSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.pc = ram[ptr];
                // Pop PC MSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.pc |= ram[ptr] << 8; 
                // Pop CS LSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.cs = ram[ptr];
                // Pop CS MSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.cs |= ram[ptr] << 8; 
                break;

            case NOPAR_OPCODE::RFI :
                state.wait_cycles +=8;
                // Pop PC LSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.pc = ram[ptr];
                // Pop PC MSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.pc |= ram[ptr] << 8; 
                // Pop CS LSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.cs = ram[ptr];
                // Pop CS MSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.cs |= ram[ptr] << 8; 
                // Pop R1 LSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.r[reg3] = ram[ptr];
                // Pop R1 MSB
                ptr = ((state.ss&0x0F) << 16) | state.r[SP]++;
                state.r[1] |= ram[ptr] << 8; 
                
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

/**
 * Check if is a interrupt to be procesed
 */
void RC1600::processInterrupt()
{
    if (state.ia != 0 && state.interrupt) {
        uint32_t ptr;
        state.iacq = true;
        // Push R1 MSB
        ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
        ram[ptr] = state.r[1] >> 8; 
        // Push R1 LSB
        ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
        ram[ptr] = state.r[1] & 0xFF;

        // Push CS MSB
        ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
        ram[ptr] = state.cs >> 8; 
        // Push CS LSB
        ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
        ram[ptr] = state.cs & 0xFF;
        // Push PC MSB
        ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
        ram[ptr] = state.pc >> 8; 
        // Push PC LSB
        ptr = ((state.ss&0x0F) << 16) | --state.r[SP];
        ram[ptr] = state.pc & 0xFF;

        state.r[1] = state.int_msg;
        state.pc = state.ia;
        state.cs = state.is;
        SET_ON_IF(state.flags);
        state.sleeping = false; // WakeUp! 
    }
}

} // End of namespace CPU
