/**
 * RC3200 VM - cpu.cpp
 * CPU of the virtual machine
 */

#include "cpu.hpp"
#include "rc3200_opcodes.hpp"

#include <iostream>
#include <cstdio>
#include <algorithm>

#include <cassert>

namespace vm {
namespace cpu {

RC3200::RC3200(const byte_t* rom, size_t rom_size, size_t ram_size) : 
    ram(rom, rom_size, ram_size), tot_cycles(0)
{
    reset();    
}

RC3200::~RC3200()
{ }

void RC3200::reset()
{
    std::fill_n(state.r, N_GPRS-1, 0);

    state.pc = state.ia = 0;
    state.flags = state.y = 0;

    state.wait_cycles = 0;

    state.skiping = false;
    state.sleeping = false;

    state.interrupt = false;
    state.int_msg = 0;
    state.iacq = false;

    state.step_mode = false;
}

unsigned RC3200::step()
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

void RC3200::tick (unsigned n)
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


/**
 * Executes a RC3200 instruction
 * @return Number of cycles that takes to do it
 */
unsigned RC3200::realStep()
{
    if (state.pc % 4 != 0) // Not multiply of 4 addresses have penalty
        state.wait_cycles = 1;
    else
        state.wait_cycles = 0;

    dword_t inst = ram.rb(state.pc++);
    inst |= (ram.rb(state.pc++) << 8);
    inst |= (ram.rb(state.pc++) << 16);
    inst |= (ram.rb(state.pc++) << 24); // Little Endian

    dword_t opcode, r1, r2, r3;
    bool literal = HAVE_LITERAL(inst); 

    dword_t tmp1, tmp2;
    qword_t ltmp;

    r3 = OP3(inst);
    r2 = OP2(inst);
    r1 = OP1(inst);
    opcode = (inst >> 16) & 0x0FFF;

    // Check if we are skiping a instruction
    
    if (state.skiping) {
        state.wait_cycles += 1;

        // See what kind of instruction is to know how many should
        // increment PC, and remove skiping flag if is not a branch
        if (IS_PAR3(inst)) {
            // 3 parameter instruction
            if (literal && IS_BIG_LITERAL_P3(r2) )
                state.pc +=4;
                
        } else if (IS_PAR2(inst)) {
            // 2 parameter instruction
            r2 = (inst >> 5) & 0x3FF;
            if (literal && IS_BIG_LITERAL_P2(r2) )
                state.pc +=4;
        } else if (IS_PAR1(inst)) { 
            // 1 parameter instruction
            r3 = inst & 0x7FFF;
            if (literal && IS_BIG_LITERAL_P1(r3) )
                state.pc +=4;
        }

        state.skiping = IS_BRANCH(inst);
        
        return state.wait_cycles;
    }

    // Check the type of instruction
    if (IS_PAR3(inst)) {
        // 3 parameter instruction ********************************************
        
        if (literal) {
            if (IS_BIG_LITERAL_P3(r2)) { // Next dword is literal value 
                r2 = ram.rb(state.pc++);
                r2 |= (ram.rb(state.pc++) << 8);
                r2 |= (ram.rb(state.pc++) << 16);
                r2 |= (ram.rb(state.pc++) << 24); // Little Endian
                state.wait_cycles++;
            } else if (O5_SIGN_BIT(r2)) { // Negative Literal -> Extend sign
                r2 |= 0xFFFFFFE0;
            }
        } else {
            r2 = state.r[r2];
        }
        
        if (IS_RAM(inst)) { // LOAD/STORE instruction
            state.wait_cycles += 4;
            tmp1 = (state.r[r1] + r2)&0xFFFFFFFF;

            if (tmp1 % 4 != 0)  // Not multiply of 4 addresses have penalty
                state.wait_cycles++;
            
            switch (opcode) {
                case OPCODE_RAM::LOAD :
                    state.r[r3] = ram.rb(tmp1);
                    state.r[r3] |= ram.rb(tmp1+1) << 8;
                    state.r[r3] |= ram.rb(tmp1+2) << 16;
                    state.r[r3] |= ram.rb(tmp1+3) << 24; // Little Endian
                    break;
                
                case OPCODE_RAM::LOADW :
                    state.r[r3] = ram.rb(tmp1);
                    state.r[r3] |= ram.rb(tmp1+1) << 8;
                    break;
                
                case OPCODE_RAM::LOADB :
                    state.r[r3] = ram.rb(tmp1);
                    break;

                case OPCODE_RAM::STORE :
                    ram.wb(tmp1++, state.r[r3]);
                    ram.wb(tmp1++, state.r[r3] >> 8);
                    ram.wb(tmp1++, state.r[r3] >> 16);
                    ram.wb(tmp1++, state.r[r3] >> 24);
                    break;
                
                case OPCODE_RAM::STOREW :
                    ram.wb(tmp1++, state.r[r3]);
                    ram.wb(tmp1++, state.r[r3] >> 8);
                    break;

                case OPCODE_RAM::STOREB :
                    ram.wb(tmp1++, state.r[r3]);
                    break;

                default:
                    // Unknow OpCode -> Acts like a NOP (this could change)
                    state.wait_cycles -= 3;
            }
        } else { // ALU operations of type r3 = r1 op r2 
            state.wait_cycles += 3;
            switch (opcode) {
                case PAR3_OPCODE_ALU::ADD :
                    ltmp = (qword_t)(state.r[r1] + r2);
                    if (CARRY_BIT(ltmp)) // We grab carry bit
                        SET_ON_CF(state.flags);
                    else
                        SET_OFF_CF(state.flags);

                    // If operands have same sign, check overflow
                    if (DW_SIGN_BIT(state.r[r1]) == DW_SIGN_BIT(r2)) {
                        if (DW_SIGN_BIT(r2) != DW_SIGN_BIT(state.r[ltmp]) ) { 
                            // Overflow happens
                            SET_ON_OF(state.flags);
                            if (GET_EOE(state.flags)) {
                                throwInterrupt(4);
                            }
                        } else {
                            SET_OFF_OF(state.flags);
                        }
                    }
                    state.r[r3] = (dword_t)ltmp;
                    break;
                
                case PAR3_OPCODE_ALU::SUB :
                    ltmp = (qword_t)(state.r[r1] - r2);
                    if (state.r[r1] < r2) // We borrow a bit
                        SET_ON_CF(state.flags);
                    else
                        SET_OFF_CF(state.flags);

                    // If operands have same sign, check overflow
                    if (DW_SIGN_BIT(state.r[r1]) == DW_SIGN_BIT(r2)) {
                        if (DW_SIGN_BIT(r2) != DW_SIGN_BIT(state.r[ltmp]) ) { 
                            // Overflow happens
                            SET_ON_OF(state.flags);
                            if (GET_EOE(state.flags)) {
                                throwInterrupt(4);
                            }
                        } else {
                            SET_OFF_OF(state.flags);
                        }
                    }
                    state.r[r3] = (dword_t)ltmp;
                    break;

                case PAR3_OPCODE_ALU::ADDC :
                    ltmp = (qword_t)(state.r[r1] + r2);
                    if (GET_CF(state.flags)) // Apply Carry bit
                        ltmp++;

                    if (CARRY_BIT(ltmp)) // We grab carry bit
                        SET_ON_CF(state.flags);
                    else
                        SET_OFF_CF(state.flags);

                    // If operands have same sign, check overflow
                    if (DW_SIGN_BIT(state.r[r1]) == DW_SIGN_BIT(r2)) {
                        if (DW_SIGN_BIT(r2) != DW_SIGN_BIT(state.r[ltmp]) ) { 
                            // Overflow happens
                            SET_ON_OF(state.flags);
                            if (GET_EOE(state.flags)) {
                                throwInterrupt(4);
                            }
                        } else {
                            SET_OFF_OF(state.flags);
                        }
                    }
                    state.r[r3] = (dword_t)ltmp;
                    break;
                
                case PAR3_OPCODE_ALU::SUBB :
                    ltmp = (qword_t)(state.r[r1] - r2);
                    if (GET_CF(state.flags)) // Apply borrow bit
                        ltmp--;

                    if (state.r[r1] < r2) // We borrow a bit
                        SET_ON_CF(state.flags);
                    else
                        SET_OFF_CF(state.flags);

                    // If operands have same sign, check overflow
                    if (DW_SIGN_BIT(state.r[r1]) == DW_SIGN_BIT(r2)) {
                        if (DW_SIGN_BIT(r2) != DW_SIGN_BIT(state.r[ltmp]) ) { 
                            // Overflow happens
                            SET_ON_OF(state.flags);
                            if (GET_EOE(state.flags)) {
                                throwInterrupt(4);
                            }
                        } else {
                            SET_OFF_OF(state.flags);
                        }
                    }
                    state.r[r3] = (dword_t)ltmp;
                    break;

                case PAR3_OPCODE_ALU::AND :
                    state.r[r3] = state.r[r1] & r2;
                    SET_OFF_OF(state.flags);
                    SET_OFF_CF(state.flags);
                
                case PAR3_OPCODE_ALU::OR :
                    state.r[r3] = state.r[r1] | r2;
                    SET_OFF_OF(state.flags);
                    SET_OFF_CF(state.flags);
                
                case PAR3_OPCODE_ALU::XOR :
                    state.r[r3] = state.r[r1] ^ r2;
                    SET_OFF_OF(state.flags);
                    SET_OFF_CF(state.flags);

                case PAR3_OPCODE_ALU::NAND :
                    state.r[r3] = ~(state.r[r1] & r2);
                    SET_OFF_OF(state.flags);
                    SET_OFF_CF(state.flags);

                case PAR3_OPCODE_ALU::SLL :
                    state.r[r3] = state.r[r1] << r2; 
                    SET_OFF_OF(state.flags);
                    SET_OFF_CF(state.flags);
                    break;
                
                case PAR3_OPCODE_ALU::SRL :
                    state.r[r3] = state.r[r1] >> r2; 
                    SET_OFF_OF(state.flags);
                    SET_OFF_CF(state.flags);
                    break;

                case PAR3_OPCODE_ALU::SRA : {
                      sword_t sr1 = state.r[r1];
                      sword_t sr2 = r2;
                      sword_t result = sr1 >> sr2; // Enforce to do arithmetic shift
                      state.r[r3] = (dword_t)result;
                      SET_OFF_OF(state.flags);
                      SET_OFF_CF(state.flags);
                      break;
                    }

                case PAR3_OPCODE_ALU::ROTL :
                    state.r[r3] = state.r[r1] << (r2%32);
                    state.r[r3] |= state.r[r1] >> (32 - (r2)%32);
                    SET_OFF_OF(state.flags);
                    SET_OFF_CF(state.flags);
                    break;

                case PAR3_OPCODE_ALU::ROTR :
                    state.r[r3] = state.r[r1] >> (r2%32);
                    state.r[r3] |= state.r[r1] << (32 - (r2)%32);
                    SET_OFF_OF(state.flags);
                    SET_OFF_CF(state.flags);
                    break;

                case PAR3_OPCODE_ALU::UMUL :
                    state.wait_cycles += 17;
                    ltmp = (qword_t)(state.r[r1]) * r2;
                    state.y = (word_t)(ltmp >> 32); // 32bit MSB of the 64 result
                    state.r[r3] = (word_t)ltmp;     // 32bit LSB of the 64 bit result
                    SET_OFF_OF(state.flags);
                    SET_OFF_CF(state.flags);
                    break;

                case PAR3_OPCODE_ALU::UDIV :
                    state.wait_cycles += 27;
                    if (r2 != 0) {
                        state.r[r3] = state.r[r1] / r2;
                        state.y = state.r[r1] % r2; // Compiler should optimize this and use a single isntrucction
                    } else { // Division by 0
                        SET_ON_DE(state.flags);
                        if ( GET_EDE(state.flags)) {
                            throwInterrupt(0);
                        }
                    }
                    SET_OFF_OF(state.flags);
                    SET_OFF_CF(state.flags);
                    break;

                case PAR3_OPCODE_ALU::MUL : {
                        state.wait_cycles += 27;
                        sqword_t lword = (sqword_t)(state.r[r1]);
                        lword *= r2;
                        state.y = (word_t)(lword >> 32); // 32bit MSB of the 64 result
                        state.r[r3] = (word_t)lword;     // 32bit LSB of the 64 bit result
                        SET_OFF_OF(state.flags);
                        SET_OFF_CF(state.flags);
                        break;
                    }

                case PAR3_OPCODE_ALU::DIV : {
                        state.wait_cycles += 37;
                        if (r2 != 0) {
                            sdword_t sr1 = state.r[r1];
                            sdword_t sr2 = r2;
                            sdword_t result = sr1 / sr2;
                            state.r[r3] = result;
                            result = sr1% sr2;
                            state.y = result;
                        } else { // Division by 0
                            SET_ON_DE(state.flags);
                            if ( GET_EDE(state.flags)) {
                                throwInterrupt(0);
                            }
                        }
                        
                        SET_OFF_OF(state.flags);
                        SET_OFF_CF(state.flags);
                        
                        break;
                    }

                default:
                    // Unknow OpCode -> Acts like a NOP (this could change)
                    state.wait_cycles -= 2;
            }
        }

    } else if (IS_PAR2(inst)) {
        // 2 parameter instrucction *******************************************
       
        if (literal) {
            r2 = (inst >> 5) & 0x3FF;
            if (IS_BIG_LITERAL_P2(r2)) { // Next dword is literal value 
                r2 = ram.rb(state.pc++);
                r2 |= (ram.rb(state.pc++) << 8);
                r2 |= (ram.rb(state.pc++) << 16);
                r2 |= (ram.rb(state.pc++) << 24); // Little Endian
                state.wait_cycles++;
            } else if (O10_SIGN_BIT(r2)) { // Negative Literal -> Extend sign
                r2 |= 0xFFFFFC00;
            }
        } else {
            r2 = state.r[r2];
        }

        
        if (IS_RAM(inst) && literal) { // LOAD/STORE instruction
            state.wait_cycles += 4;
            
            if (r2 % 4 != 0)  // Not multiply of 4 addresses have penalty
                state.wait_cycles++;
            
            switch (opcode) {
                case OPCODE_RAM::LOAD :
                    state.r[r3] = ram.rb(r2);
                    state.r[r3] |= ram.rb(r2+1) << 8;
                    state.r[r3] |= ram.rb(r2+2) << 16;
                    state.r[r3] |= ram.rb(r2+3) << 24; // Little Endian
                    break;
                
                case OPCODE_RAM::LOADW :
                    state.r[r3] = ram.rb(r2);
                    state.r[r3] |= ram.rb(r2+1) << 8;
                    break;
                
                case OPCODE_RAM::LOADB :
                    state.r[r3] = ram.rb(r2);
                    break;

                case OPCODE_RAM::STORE :
                    ram.wb(r2++, state.r[r3]);
                    ram.wb(r2++, state.r[r3] >> 8);
                    ram.wb(r2++, state.r[r3] >> 16);
                    ram.wb(r2++, state.r[r3] >> 24);
                    break;
                
                case OPCODE_RAM::STOREW :
                    ram.wb(r2++, state.r[r3]);
                    ram.wb(r2++, state.r[r3] >> 8);
                    break;

                case OPCODE_RAM::STOREB :
                    ram.wb(r2++, state.r[r3]);
                    break;

                default:
                    // Unknow OpCode -> Acts like a NOP (this could change)
                    state.wait_cycles -= 3;
            }

        } else if (IS_BRANCH(inst)) { // BRANCHing instruction
            state.wait_cycles += 3;
            switch (opcode) {
                case OPCODE_BRANCH::IFEQ :
                    if (!(r2 == state.r[r3])) {
                        state.skiping = true;
                        state.wait_cycles++;
                    }
                    break;

                case OPCODE_BRANCH::IFNEQ :
                    if (!(r2 != state.r[r3])) {
                        state.skiping = true;
                        state.wait_cycles++;
                    }
                    break;

                case OPCODE_BRANCH::IFUG :
                    if (!(r2 > state.r[r3])) {
                        state.skiping = true;
                        state.wait_cycles++;
                    }
                    break;

                case OPCODE_BRANCH::IFG : {
                        sdword_t sr3 = state.r[r3];
                        sdword_t sr2 = r2;
                        if (!(sr2 > sr3)) {
                            state.skiping = true;
                            state.wait_cycles++;
                        }
                        break;
                    }

                case OPCODE_BRANCH::IFUGE :
                    if (!(r2 >= state.r[r3])) {
                        state.skiping = true;
                        state.wait_cycles++;
                    }
                    break;
                
                case OPCODE_BRANCH::IFGE : {
                        sdword_t sr3 = state.r[r3];
                        sdword_t sr2 = r2;
                        if (!(sr2 >= sr3)) {
                            state.skiping = true;
                            state.wait_cycles++;
                        }
                        break;
                    }

                case OPCODE_BRANCH::IFUL :
                    if (!(r2 < state.r[r3])) {
                        state.skiping = true;
                        state.wait_cycles++;
                    }
                    break;

                case OPCODE_BRANCH::IFL : {
                        sdword_t sr3 = state.r[r3];
                        sdword_t sr2 = r2;
                        if (!(sr2 < sr3)) {
                            state.skiping = true;
                            state.wait_cycles++;
                        }
                        break;
                    }

                case OPCODE_BRANCH::IFULE :
                    if (!(r2 <= state.r[r3])) {
                        state.skiping = true;
                        state.wait_cycles++;
                    }
                    break;
                
                case OPCODE_BRANCH::IFLE : {
                        sdword_t sr3 = state.r[r3];
                        sdword_t sr2 = r2;
                        if (!(sr2 <= sr3)) {
                            state.skiping = true;
                            state.wait_cycles++;
                        }
                        break;
                    }

                case OPCODE_BRANCH::IFBITS :
                    if (! ((r2 & state.r[r3]) != 0)) {
                        state.skiping = true;
                        state.wait_cycles++;
                    }
                    break;
                
                case OPCODE_BRANCH::IFCLEAR :
                    if (! ((r2 & state.r[r3]) == 0)) {
                        state.skiping = true;
                        state.wait_cycles++;
                    }
                    break;

                default:
                    // Unknow OpCode -> Acts like a NOP (this could change)
                    state.wait_cycles -= 2;
            }

        } else if (IS_JUMP(inst)) { // JUMP/CALL instruction
            state.wait_cycles += 3;
            switch (opcode) {
                case OPCODE_JUMP::JMP : // Absolute jump
                    state.pc = r2 + state.r[r3];
                    break;
                
                case OPCODE_JUMP::CALL : // Absolute call
                    state.wait_cycles++;
                    // push to the stack register pc value
                    ram.wb(--state.r[SP], state.pc >> 24);
                    ram.wb(--state.r[SP], state.pc >> 16);
                    ram.wb(--state.r[SP], state.pc >> 8);
                    ram.wb(--state.r[SP], state.pc); // Little Endian
                    state.pc = r2 + state.r[r3];
                    break;
                
                default:
                    // Unknow OpCode -> Acts like a NOP (this could change)
                    state.wait_cycles -= 2;
            }

        } else { // Register manipulation instruction
            state.wait_cycles += 3;
            switch (opcode) {
                case PAR2_OPCODE::CPY_SET :
                        state.r[r3] = r2;
                        break;

                case PAR2_OPCODE::SWP :
                    if (! literal) { // SWP with literal acts like a NOP
                        tmp1 = state.r[r3];
                        state.r[r3]= state.r[r2];
                        state.r[r2] = tmp1;
                    }
                    break;

                default:
                    // Unknow OpCode -> Acts like a NOP (this could change)
                    state.wait_cycles -= 2;
            }
        }
    
    } else if (IS_PAR1(inst)) {
        // 1 parameter instrucction *******************************************
        
        if (literal) {
            r3 = inst & 0x7FFF;
            if (IS_BIG_LITERAL_P1(r3)) { // Next dword is literal value 
                r3 = ram.rb(state.pc++);
                r3 |= (ram.rb(state.pc++) << 8);
                r3 |= (ram.rb(state.pc++) << 16);
                r3 |= (ram.rb(state.pc++) << 24); // Little Endian
                state.wait_cycles++;
            } else if (O15_SIGN_BIT(r3)) { // Negative Literal -> Extend sign
                r3 |= 0xFFFF8000;
            }
        }

        if (IS_RAM(inst)) { // Stack instructions
            if (opcode == OPCODE_RAM::PUSH) {
                state.wait_cycles +=4;
                r3= (literal) ? r3 : state.r[r3]; // Operand value
               
                if (state.r[SP] % 4 != 0)  // Not multiply of 4 addresses have penalty
                    state.wait_cycles++;

                ram.wb(--state.r[SP], r3 >> 24);
                ram.wb(--state.r[SP], r3 >> 16);
                ram.wb(--state.r[SP], r3 >> 8);
                ram.wb(--state.r[SP], r3); // Little Endian
            } else if (opcode == OPCODE_RAM::POP) {
                state.wait_cycles +=4;

                if (state.r[SP] % 4 != 0)  // Not multiply of 4 addresses have penalty
                    state.wait_cycles++;

                state.r[r3] = ram.rb(state.r[SP]++);
                state.r[r3] |= ram.rb(state.r[SP]++) << 8;
                state.r[r3] |= ram.rb(state.r[SP]++) << 16;
                state.r[r3] |= ram.rb(state.r[SP]++) << 24;
                // Little Endian
            } 

        } else if (IS_JUMP(inst)) { // Jump/Call instructions
            state.wait_cycles += 3;
            r3 = (literal) ? r3 : state.r[r3]; // Operand 2 value
            switch (opcode) {
                case OPCODE_JUMP::INT : // Software interrupt
                    state.wait_cycles += 3;
                    throwInterrupt(r3);
                    break;
                
                case OPCODE_JUMP::JMP : // Absolute jump
                    state.pc += r3;
                    break;
                
                case OPCODE_JUMP::CALL : // Absolute call
                    state.wait_cycles++;
                    // push to the stack register pc value
                    ram.wb(--state.r[SP], state.pc >> 24);
                    ram.wb(--state.r[SP], state.pc >> 16);
                    ram.wb(--state.r[SP], state.pc >> 8);
                    ram.wb(--state.r[SP], state.pc); // Little Endian
                    state.pc += r3;
                    break;
                
                default:
                    // Unknow OpCode -> Acts like a NOP (this could change)
                    state.wait_cycles -= 2;
            }

        } else { // ALU / REGISTER instructions
            state.wait_cycles += 3;
            switch (opcode) {
                case PAR1_OPCODE::NOT :
                    state.r[r3] = ~ state.r[r3];
                    break;

                case PAR1_OPCODE::NEG :
                    state.r[r3] = (~ state.r[r3]) +1;
                    break;

                case PAR1_OPCODE::XCHG :
                    tmp1 = (state.r[r3] >> 8) & 0xFF;
                    tmp2 = (state.r[r3] & 0xFF) << 8 ;
                    state.r[r3] &= 0xFFFF0000;
                    state.r[r3] |= tmp1 | tmp2;
                    break;

                case PAR1_OPCODE::XCHGW :
                    tmp1 = state.r[r3] >> 16;
                    tmp2 = state.r[r3] << 16;
                    state.r[r3] = tmp1 | tmp2;
                    break;

                case PAR1_OPCODE::SXTBD : 
                    if ((state.r[r3] & 0x00000080) != 0) {
                        state.r[r3] |= 0xFFFFFF00;   // Negative
                    } else {
                        state.r[r3] &= 0x000000FF; // Positive
                    }
                    break;

                case PAR1_OPCODE::SXTWD : 
                    if ((state.r[r3] & 0x00008000) != 0) {
                        state.r[r3] |= 0xFFFF0000;   // Negative
                    } else {
                        state.r[r3] &= 0x0000FFFF; // Positive
                    }
                    break;


                case PAR1_OPCODE::GETPC :
                    state.r[r3] = state.pc;
                    break;

                case PAR1_OPCODE::SETFLAGS :
                    r3 = (literal) ? r3 : state.r[r3]; // Operand 2 value
                    state.flags = r3;
                    break;

                case PAR1_OPCODE::GETFLAGS :
                    state.r[r3] = state.flags;
                    break;

                case PAR1_OPCODE::SETY :
                    r3 = (literal) ? r3 : state.r[r3]; // Operand 2 value
                    state.y = r3;
                    break;

                case PAR1_OPCODE::GETY :
                    state.r[r3] = state.y;
                    break;

                case PAR1_OPCODE::SETIA :
                    r3 = (literal) ? r3 : state.r[r3]; // Operand 2 value
                    state.ia = r3;
                    break;

                case PAR1_OPCODE::GETIA :
                    state.r[r3] = state.ia;
                    break;

                default:
                    // Unknow OpCode -> Acts like a NOP (this could change)
                    state.wait_cycles -= 2;
            }

        }

    } else if (IS_NOPAR(inst)) {
        // Instructions without parameters ************************************
        opcode = inst & 0x0FFFFFFF; // OpCode uses the 16 LSB
        
        if (IS_JUMP(inst)) { // Return instruction
            switch (opcode) {
                case OPCODE_JUMP::RFI :
                    state.wait_cycles = 6;
                    
                    // Pop PC
                    state.pc = ram.rb(state.r[SP]++);
                    state.pc |= ram.rb(state.r[SP]++) << 8;
                    state.pc |= ram.rb(state.r[SP]++) << 16;
                    state.pc |= ram.rb(state.r[SP]++) << 24;
                    
                    // Pop %r0
                    state.r[0] = ram.rb(state.r[SP]++);
                    state.r[0] |= ram.rb(state.r[SP]++) << 8;
                    state.r[0] |= ram.rb(state.r[SP]++) << 16;
                    state.r[0] |= ram.rb(state.r[SP]++) << 24;

                    SET_OFF_IF(state.flags);
                    state.iacq = false;
                    state.interrupt = false; // We now not have a interrupt
                    break;

                case OPCODE_JUMP::RET:
                    state.wait_cycles = 4;
                    // Pop PC
                    state.pc = ram.rb(state.r[SP]++);
                    state.pc |= ram.rb(state.r[SP]++) << 8;
                    state.pc |= ram.rb(state.r[SP]++) << 16;
                    state.pc |= ram.rb(state.r[SP]++) << 24;
                    break;

                default:
                    // Unknow OpCode -> Acts like a NOP (this could change)
                    state.wait_cycles = 1;
            }

        } else if (IS_BRANCH(inst)) { // Branch if Flag X
            state.wait_cycles = 3;
            switch (opcode) {
                case OPCODE_BRANCH::IFOF :
                    if (! GET_OF(state.flags)) {
                        state.skiping = true;
                        state.wait_cycles++;
                    }
                    break;
                
                case OPCODE_BRANCH::IFCF :
                    if (! GET_CF(state.flags)) {
                        state.skiping = true;
                        state.wait_cycles++;
                    }
                    break;
                
                default:
                    // Unknow OpCode -> Acts like a NOP (this could change)
                    state.wait_cycles -= 2;
            }
        
        } else {    // Misc no parameter instruction
            switch (opcode) {
                case NOPAR_OPCODE::SLEEP :
                    state.wait_cycles = 1;
                    state.sleeping = true;
                    break;
                
                case NOPAR_OPCODE::NOP :
                default:
                    // Unknow OpCode -> Acts like a NOP 
                    state.wait_cycles = 1;

            }
        }
    }
    
    // If step-mode is enable, throw the adequate exception
    if (state.step_mode)
        throwInterrupt(1);

    processInterrupt(); // Here we check if a interrupt happens
    

    return state.wait_cycles;
    
}

/**
 * Check if there is an interrupt to be procesed
 */
void RC3200::processInterrupt()
{
    if (GET_EI(state.flags) && state.interrupt && !state.iacq) {
        state.iacq = true;

        // push %r0
        ram.wb(--state.r[SP], state.r[0] >> 24);
        ram.wb(--state.r[SP], state.r[0] >> 16);
        ram.wb(--state.r[SP], state.r[0] >> 8);
        ram.wb(--state.r[SP], state.r[0]); // Little Endian

        // push PC
        ram.wb(--state.r[SP], state.pc >> 24);
        ram.wb(--state.r[SP], state.pc >> 16);
        ram.wb(--state.r[SP], state.pc >> 8);
        ram.wb(--state.r[SP], state.pc); // Little Endian

        state.r[0] = state.int_msg;
        state.pc = state.ia;
        SET_ON_IF(state.flags);
        state.sleeping = false; // WakeUp! 
    }
}

} // End of namespace cpu
} // End of namespace vm
