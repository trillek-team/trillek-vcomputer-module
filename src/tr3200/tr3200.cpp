/**
 * \brief       TR3200 CPU
 * \file        tr3200.cpp
 * \copyright   The MIT License (MIT)
 *
 * Implementation of the TR3200 CPU v0.10
 * @see https://github.com/trillek-team/trillek-computer/blob/master/TR3200.md
 */

#include "tr3200/tr3200.hpp"
#include "tr3200/tr3200_opcodes.hpp"
#include "tr3200/tr3200_macros.hpp"
#include "vs_fix.hpp"
#include "config.hpp"

#include <cstdio>
#include <algorithm>

#include <cassert>

namespace vm {
namespace cpu {

static const byte_t cycle_table[] = { /// Lookup table for cycle count
#include "tr3200/tr3200_cycles.inc"
};

TR3200::TR3200(unsigned clock) : ICPU(), cpu_clock(clock) {
    this->Reset();
}

TR3200::~TR3200() {
}

void TR3200::Reset() {
    std::fill_n(r, TR3200_NGPRS, 0);
    pc = 0x100000; //0;

    wait_cycles = 0;

    int_msg = 0;

    interrupt = false;
    step_mode = false;
    skiping   = false;
    sleeping  = false;
} // Reset

unsigned TR3200::Step() {
    assert (vcomp != nullptr);

    if (!sleeping) {
        unsigned cyc = RealStep();
        return cyc;
    }
    else {
        ProcessInterrupt();
        return 1;
    }
} // Step

void TR3200::Tick(unsigned n) {
    assert (vcomp != nullptr);

    unsigned i = 0;

    while (i < n) {
        if (!sleeping) {
            if (wait_cycles <= 0 ) {
                RealStep();
            }
#ifdef BRKPOINTS
            if ( vcomp->isHalted() ) {
                return;
            }
#endif
            wait_cycles--;
        }
        else {
            ProcessInterrupt();
        }

        i++;
    }
} // Tick

bool TR3200::SendInterrupt (word_t msg) {
    if ( GET_EI(REG_FLAGS) ) {
        // The CPU accepts a new interrupt
        interrupt = true;
        int_msg   = msg;
        return true;
    }

    return false;
}

/**
 * Executes a TR3200 instruction
 * @return Number of cycles that takes to do it
 */
unsigned TR3200::RealStep() {
    //unsigned wait_cycles;

#ifdef BRKPOINTS
    if ( vcomp->isBreakPoint(pc) ) {
        // Breakpoint !
        return 0;
    }
#endif

    dword_t inst = vcomp->ReadDW(pc);
    pc += 4;

    dword_t opcode, rd, rs, rn;
    bool literal = HAVE_LITERAL(inst);

    qword_t ltmp;

    rd          = GRD(inst);
    rs          = GRS(inst);
    opcode      = GET_OP_CODE(inst);
    wait_cycles = cycle_table[opcode];

    // Check if we are skiping a instruction
    if (!skiping) {
        if ( IS_P3(inst) ) {
            // Processing of operands
            if (literal) {
                rn = LIT15(inst);
                if ( IS_BIG_LITERAL_L15(rn) ) {
                    // Next dword is literal value
                    rn  = vcomp->ReadDW(pc);
                    pc += 4;
                    wait_cycles++;
                }
                else if ( RN_SIGN_BIT(inst) ) {
                    // Negative Literal -> Extend sign
                    rn |= 0xFFFF8000;
                }
            }
            else {
                rn = r[GRN(inst)];
            }

            rs = r[rs];

            switch (opcode) {
            case P3_OPCODE::AND:
                r[rd] = rs & rn;
                SET_OFF_CF(REG_FLAGS);
                SET_OFF_OF(REG_FLAGS);
                break;

            case P3_OPCODE::OR:
                r[rd] = rs | rn;
                SET_OFF_CF(REG_FLAGS);
                SET_OFF_OF(REG_FLAGS);
                break;

            case P3_OPCODE::XOR:
                r[rd] = rs ^ rn;
                SET_OFF_CF(REG_FLAGS);
                SET_OFF_OF(REG_FLAGS);
                break;

            case P3_OPCODE::BITC:
                r[rd] = rs & (~rn);
                SET_OFF_CF(REG_FLAGS);
                SET_OFF_OF(REG_FLAGS);
                break;


            case P3_OPCODE::ADD:
                ltmp = ( (qword_t)rs ) + rn;
                if ( CARRY_BIT(ltmp) ) {
                    // We grab carry bit
                    SET_ON_CF(REG_FLAGS);
                }
                else {
                    SET_OFF_CF(REG_FLAGS);
                }

                // If operands have same sign, check overflow
                if ( DW_SIGN_BIT(rs) == DW_SIGN_BIT(rn) ) {
                    if ( DW_SIGN_BIT(rn) != DW_SIGN_BIT(ltmp) ) {
                        // Overflow happens
                        SET_ON_OF(REG_FLAGS);
                    }
                    else {
                        SET_OFF_OF(REG_FLAGS);
                    }
                }
                r[rd] = (dword_t)ltmp;
                break;

            case P3_OPCODE::ADDC:
                ltmp = ( (qword_t)rs ) + rn + GET_CF(REG_FLAGS);
                if ( CARRY_BIT(ltmp) ) {
                    // We grab carry bit
                    SET_ON_CF(REG_FLAGS);
                }
                else {
                    SET_OFF_CF(REG_FLAGS);
                }

                // If operands have same sign, check overflow
                if ( DW_SIGN_BIT(rs) == DW_SIGN_BIT(rn) ) {
                    if ( DW_SIGN_BIT(rn) != DW_SIGN_BIT(ltmp) ) {
                        // Overflow happens
                        SET_ON_OF(REG_FLAGS);
                    }
                    else {
                        SET_OFF_OF(REG_FLAGS);
                    }
                }
                r[rd] = (dword_t)ltmp;
                break;

            case P3_OPCODE::SUB:
                ltmp = ( (qword_t)rs ) - rn;
                if (rs < rn) {
                    // We grab carry bit
                    SET_ON_CF(REG_FLAGS);
                }
                else {
                    SET_OFF_CF(REG_FLAGS);
                }

                // If operands have distint sign, check overflow
                if ( DW_SIGN_BIT(rs) != DW_SIGN_BIT(rn) ) {
                    if ( DW_SIGN_BIT(rn) == DW_SIGN_BIT(ltmp) ) {
                        // Overflow happens
                        SET_ON_OF(REG_FLAGS);
                    }
                    else {
                        SET_OFF_OF(REG_FLAGS);
                    }
                }
                r[rd] = (dword_t)ltmp;
                break;

            case P3_OPCODE::SUBB:
                ltmp = ( (qword_t)rs ) - ( rn + GET_CF(REG_FLAGS) );
                if ( rs < ( rn + GET_CF(REG_FLAGS) ) ) {
                    // We grab carry bit
                    SET_ON_CF(REG_FLAGS);
                }
                else {
                    SET_OFF_CF(REG_FLAGS);
                }

                // If operands have distint sign, check overflow
                if ( DW_SIGN_BIT(rs) != DW_SIGN_BIT(rn) ) {
                    if ( DW_SIGN_BIT(rn) == DW_SIGN_BIT(ltmp) ) {
                        // Overflow happens
                        SET_ON_OF(REG_FLAGS);
                    }
                    else {
                        SET_OFF_OF(REG_FLAGS);
                    }
                }
                r[rd] = (dword_t)ltmp;
                break;

            case P3_OPCODE::RSB:
                ltmp = ( (qword_t)rn ) - rs;
                if (rn < rs) {
                    // We grab carry bit
                    SET_ON_CF(REG_FLAGS);
                }
                else {
                    SET_OFF_CF(REG_FLAGS);
                }

                // If operands have same sign, check overflow
                if ( DW_SIGN_BIT(rs) == DW_SIGN_BIT(rn) ) {
                    if ( DW_SIGN_BIT(rn) != DW_SIGN_BIT(ltmp) ) {
                        // Overflow happens
                        SET_ON_OF(REG_FLAGS);
                    }
                    else {
                        SET_OFF_OF(REG_FLAGS);
                    }
                }
                r[rd] = (dword_t)ltmp;
                break;

            case P3_OPCODE::RSBB:
                ltmp = ( (qword_t)rn ) - ( rs + GET_CF(REG_FLAGS) );
                if ( rn < ( rs + GET_CF(REG_FLAGS) ) ) {
                    // We grab carry bit
                    SET_ON_CF(REG_FLAGS);
                }
                else {
                    SET_OFF_CF(REG_FLAGS);
                }

                // If operands have same sign, check overflow
                if ( DW_SIGN_BIT(rs) == DW_SIGN_BIT(rn) ) {
                    if ( DW_SIGN_BIT(rn) != DW_SIGN_BIT(ltmp) ) {
                        // Overflow happens
                        SET_ON_OF(REG_FLAGS);
                    }
                    else {
                        SET_OFF_OF(REG_FLAGS);
                    }
                }
                r[rd] = (dword_t)ltmp;
                break;

            case P3_OPCODE::LLS:
                ltmp = ( (qword_t)rs ) << rn;
                if ( CARRY_BIT(ltmp) ) {
                    // We grab output bit
                    SET_ON_CF(REG_FLAGS);
                }
                else {
                    SET_OFF_CF(REG_FLAGS);
                }
                SET_OFF_OF(REG_FLAGS);
                r[rd] = (dword_t)ltmp;
                break;

            case P3_OPCODE::RLS:
                ltmp = ( (qword_t)rs << 1 ) >> rn;
                if (ltmp & 1) {
                    // We grab output bit
                    SET_ON_CF(REG_FLAGS);
                }
                else {
                    SET_OFF_CF(REG_FLAGS);
                }
                SET_OFF_OF(REG_FLAGS);
                r[rd] = (dword_t)(ltmp >> 1);
                break;

            case P3_OPCODE::ARS:
            {
                sdword_t srs = rs;
                sdword_t srn = rn;

                sqword_t result = ( ( (sqword_t)srs ) << 1 ) >> srn; // Enforce
                                                                     // to do
                                                                     //
                                                                     // arithmetic
                                                                     // shift

                if (result & 1) {
                    // We grab output bit
                    SET_ON_CF(REG_FLAGS);
                }
                else {
                    SET_OFF_CF(REG_FLAGS);
                }
                SET_OFF_OF(REG_FLAGS);
                r[rd] = (dword_t)(result >> 1);
                break;
            }

            case P3_OPCODE::ROTL:
                r[rd]  = rs << (rn%32);
                r[rd] |= rs >> (32 - (rn)%32);
                SET_OFF_OF(REG_FLAGS);
                SET_OFF_CF(REG_FLAGS);
                break;

            case P3_OPCODE::ROTR:
                r[rd]  = rs >> (rn%32);
                r[rd] |= rs << (32 - (rn)%32);
                SET_OFF_OF(REG_FLAGS);
                SET_OFF_CF(REG_FLAGS);
                break;

            case P3_OPCODE::MUL:
                ltmp  = ( (qword_t)rs ) * rn;
                REG_Y = (dword_t)(ltmp >> 32); // 32bit MSB of the 64 bit result
                r[rd] = (dword_t)ltmp;         // 32bit LSB of the 64 bit result
                SET_OFF_OF(REG_FLAGS);
                SET_OFF_CF(REG_FLAGS);
                break;

            case P3_OPCODE::SMUL:
            {
                sqword_t lword = (sqword_t)rs;
                lword *= rn;
                REG_Y  = (dword_t)(lword >> 32); // 32bit MSB of the 64 bit
                                                 // result
                r[rd] = (dword_t)lword;          // 32bit LSB of the 64 bit
                                                 // result
                SET_OFF_OF(REG_FLAGS);
                SET_OFF_CF(REG_FLAGS);
                break;
            }

            case P3_OPCODE::DIV:
                if (rn != 0) {
                    r[rd] = rs / rn;
                    REG_Y = rs % rn; // Compiler should optimize this and use a
                                     // single instruction
                }
                else {
                    // Division by 0
                    SET_ON_DE(REG_FLAGS);
                }
                SET_OFF_OF(REG_FLAGS);
                SET_OFF_CF(REG_FLAGS);
                break;


            case P3_OPCODE::SDIV:
            {
                if (rn != 0) {
                    sdword_t srs    = rs;
                    sdword_t srn    = rn;
                    sdword_t result = srs / srn;
                    r[rd]  = result;
                    result = srs % srn;
                    REG_Y  = result;
                }
                else {
                    // Division by 0
                    SET_ON_DE(REG_FLAGS);
                }
                SET_OFF_OF(REG_FLAGS);
                SET_OFF_CF(REG_FLAGS);

                break;
            }


            case P3_OPCODE::LOAD:
                r[rd] = vcomp->ReadDW(rs+rn);
                break;

            case P3_OPCODE::LOADW:
                r[rd] = vcomp->ReadW(rs+rn);
                break;

            case P3_OPCODE::LOADB:
                r[rd] = vcomp->ReadB(rs+rn);
                break;

            case P3_OPCODE::STORE:
                vcomp->WriteDW(rs+rn, r[rd]);
                break;

            case P3_OPCODE::STOREW:
                vcomp->WriteW(rs+rn, r[rd]);
                break;

            case P3_OPCODE::STOREB:
                vcomp->WriteB(rs+rn, r[rd]);
                break;

            default:
                break; // Unknow OpCode -> Acts like a NOP (this could change)
            } // switch
        }
        else if ( IS_P2(inst) ) {
            // 2 parameter instrucction
            // *******************************************

            // Fetch Rn operand
            if (literal) {
                rn = LIT19(inst);
                if ( IS_BIG_LITERAL_L19(rn) ) {
                    // Next dword is literal value
                    rn  = vcomp->ReadDW(pc);
                    pc += 4;
                    wait_cycles++;
                }
                else if ( RN_SIGN_BIT(inst) ) {
                    // Negative Literal -> Extend sign
                    rn |= 0xFFF80000;
                }
            }
            else {
                rn = r[GRS(inst)];
            }

            switch (opcode) {
            case P2_OPCODE::MOV:
                r[rd] = rn;
                break;

            case P2_OPCODE::SWP:
                if (!literal) {
                    dword_t tmp = r[rd];
                    r[rd]        = rn;
                    r[GRS(inst)] = tmp;
                } // If M != acts like a NOP
                break;

            case P2_OPCODE::NOT:
                r[rd] = ~rn;
                break;

            case P2_OPCODE::SIGXB:
                if ( (rn & 0x00000080) != 0 ) {
                    rd |= 0xFFFFFF00; // Negative
                }
                else {
                    rd &= 0x000000FF; // Positive
                }
                break;

            case P2_OPCODE::SIGXW:
                if ( (rn & 0x00008000) != 0 ) {
                    rd |= 0xFFFF0000; // Negative
                }
                else {
                    rd &= 0x0000FFFF; // Positive
                }
                break;

            case P2_OPCODE::LOAD2:
                r[rd] = vcomp->ReadDW(rn);
                break;

            case P2_OPCODE::LOADW2:
                r[rd] = vcomp->ReadW(rn);
                break;

            case P2_OPCODE::LOADB2:
                r[rd] = vcomp->ReadB(rn);
                break;

            case P2_OPCODE::STORE2:
                vcomp->WriteDW(rn, r[rd]);
                break;

            case P2_OPCODE::STOREW2:
                vcomp->WriteW(rn, r[rd]);
                break;

            case P2_OPCODE::STOREB2:
                vcomp->WriteB(rn, r[rd]);
                break;


            case P2_OPCODE::IFEQ:
                if ( !(r[rd] == rn) ) {
                    skiping = true;
                    wait_cycles++;
                }
                break;

            case P2_OPCODE::IFNEQ:
                if ( !(r[rd] != rn) ) {
                    skiping = true;
                    wait_cycles++;
                }
                break;

            case P2_OPCODE::IFL:
                if ( !(r[rd] < rn) ) {
                    skiping = true;
                    wait_cycles++;
                }
                break;

            case P2_OPCODE::IFSL:
            {
                sdword_t srd = r[rd];
                sdword_t srn = rn;
                if ( !(srd < srn) ) {
                    skiping = true;
                    wait_cycles++;
                }
                break;
            }

            case P2_OPCODE::IFLE:
                if ( !(r[rd] <= rn) ) {
                    skiping = true;
                    wait_cycles++;
                }
                break;

            case P2_OPCODE::IFSLE:
            {
                sdword_t srd = r[rd];
                sdword_t srn = rn;
                if ( !(srd <= srn) ) {
                    skiping = true;
                    wait_cycles++;
                }
                break;
            }

            case P2_OPCODE::IFBITS:
                if ( !( (r[rd] & rn) != 0 ) ) {
                    skiping = true;
                    wait_cycles++;
                }
                break;

            case P2_OPCODE::IFCLEAR:
                if ( !( (r[rd] & rn) == 0 ) ) {
                    skiping = true;
                    wait_cycles++;
                }
                break;

            case P2_OPCODE::JMP2: // Absolute jump
                pc = (r[rd] + rn) & 0xFFFFFFFC;
                break;

            case P2_OPCODE::CALL2: // Absolute call
                // push to the stack register pc value
                vcomp->WriteB(--r[SP], pc >> 24);
                vcomp->WriteB(--r[SP], pc >> 16);
                vcomp->WriteB(--r[SP], pc >> 8);
                vcomp->WriteB(--r[SP], pc); // Little Endian
                pc = (r[rd] + rn) & 0xFFFFFFFC;
                break;


            default:
                break; // Unknow OpCode -> Acts like a NOP (this could change)
            } // switch
        }
        else if ( IS_P1(inst) ) {
            // 1 parameter instrucction
            // *******************************************

            // Fetch Rn operand
            if (literal) {
                rn = LIT23(inst);
                if ( IS_BIG_LITERAL_L23(rn) ) {
                    // Next dword is literal value
                    rn  = vcomp->ReadDW(pc);
                    pc += 4;
                    wait_cycles++;
                }
                else if ( RN_SIGN_BIT(inst) ) {
                    // Negative Literal -> Extend sign
                    rn |= 0xFF800000;
                }
            }
            else {
                rn = GRD(inst);
            }

            switch (opcode) {
            case P1_OPCODE::XCHGB:
                if (!literal) {
                    word_t lob = (r[rn]  & 0xFF) << 8;
                    word_t hib = (r[rn]  >> 8) & 0xFF;
                    r[rn] = (r[rn]  & 0xFFFF0000) | lob | hib;
                }
                break;

            case P1_OPCODE::XCHGW:
                if (!literal) {
                    dword_t low = r[rn] << 16;
                    dword_t hiw = r[rn]  >> 16;
                    r[rn] = low | hiw;
                }
                break;

            case P1_OPCODE::GETPC:
                if (!literal) {
                    r[rn] = pc; // PC is alredy pointing to the next instruction
                }
                break;


            case P1_OPCODE::POP:
                if (!literal) {
                    // SP always points to the last pushed element
                    r[rn]  = vcomp->ReadDW(r[SP]);
                    r[SP] += 4;
                }
                break;

            case P1_OPCODE::PUSH:
                // SP always points to the last pushed element
                if (!literal) {
                    rn = r[rn];
                }
                vcomp->WriteB(--r[SP], rn >> 24);
                vcomp->WriteB(--r[SP], rn >> 16);
                vcomp->WriteB(--r[SP], rn >> 8 );
                vcomp->WriteB(--r[SP], rn      );
                break;


            case P1_OPCODE::JMP: // Absolute jump
                if (!literal) {
                    rn = r[rn];
                }
                pc = rn & 0xFFFFFFFC;
                break;

            case P1_OPCODE::CALL: // Absolute call
                // push to the stack register pc value
                vcomp->WriteB(--r[SP], pc >> 24);
                vcomp->WriteB(--r[SP], pc >> 16);
                vcomp->WriteB(--r[SP], pc >> 8);
                vcomp->WriteB(--r[SP], pc); // Little Endian
                if (!literal) {
                    rn = r[rn];
                }
                pc = rn & 0xFFFFFFFC;
                break;

            case P1_OPCODE::RJMP: // Relative jump
                if (!literal) {
                    rn = r[rn];
                }
                pc = (pc + rn) & 0xFFFFFFFC;
                break;

            case P1_OPCODE::RCALL: // Relative call
                // push to the stack register pc value
                vcomp->WriteB(--r[SP], pc >> 24);
                vcomp->WriteB(--r[SP], pc >> 16);
                vcomp->WriteB(--r[SP], pc >> 8);
                vcomp->WriteB(--r[SP], pc); // Little Endian
                if (!literal) {
                    rn = r[rn];
                }
                pc = (pc + rn) & 0xFFFFFFFC;
                break;


            case P1_OPCODE::INT: // Software Interrupt
                if (!literal) {
                    rn = r[rn];
                }
                SendInterrupt(rn);
                break;

            default:
                break; // Unknow OpCode -> Acts like a NOP (this could change)
            } // switch
        }
        else if ( IS_NP(inst) ) {
            // Instructions without parameters
            // ************************************

            switch (opcode) {
            case NP_OPCODE::SLEEP:
                sleeping = true;
                break;

            case NP_OPCODE::RET:
                // Pop PC
                pc     = vcomp->ReadDW(r[SP]);
                r[SP] += 4;
                pc    &= 0xFFFFFFFC;
                break;

            case NP_OPCODE::RFI:
                // Pop PC
                pc     = vcomp->ReadDW(r[SP]);
                r[SP] += 4;
                pc    &= 0xFFFFFFFC;

                // Pop %r0
                r[0]   = vcomp->ReadDW(r[SP]);
                r[SP] += 4;

                SET_OFF_IF(REG_FLAGS);
                interrupt = false; // We now not have a interrupt
                break;

            default:
                break; // Unknow OpCode -> Acts like a NOP (this could change)
            } // switch
        }

        // If step-mode is enable, Throw the adequate exception outside
        // interrupt handlers
        if ( step_mode && !GET_IF(REG_FLAGS) ) {
            SendInterrupt(0);
        }

        ProcessInterrupt(); // Here we check if a interrupt happens

        return wait_cycles;
    }
    else {
        // Skiping an instruction
        wait_cycles = 1;
        skiping     = false;

        // See what kind of instruction is to know how many should
        // increment PC, and remove skiping flag if is not an IFxxx instruction
        if (literal) {
            if ( IS_P3(inst) ) {
                // 3 parameter instruction
                rn = LIT15(inst);
                if ( IS_BIG_LITERAL_L15(rn) ) {
                    pc += 4;
                }
            }
            else if ( IS_P2(inst) ) {
                // 2 parameter instruction
                skiping = IS_BRANCH(opcode); // Chain IFxx
                rn      = LIT19(inst);
                if ( literal && IS_BIG_LITERAL_L19(rn) ) {
                    pc += 4;
                }
            }
            else if ( IS_P1(inst) ) {
                // 1 parameter instruction
                rn = LIT23(inst);
                if ( literal && IS_BIG_LITERAL_L23(rn) ) {
                    pc += 4;
                }
            }
        }
        else if ( IS_P2(inst) && IS_BRANCH(opcode) ) {
            skiping = true; // Chain IFxx
        }

        return wait_cycles;
    }
} // RealStep

/**
 * Check if there is an interrupt to be procesed
 */
void TR3200::ProcessInterrupt() {
    if (GET_EI(REG_FLAGS) && interrupt) {
        byte_t index = int_msg;
        dword_t addr = vcomp->ReadDW( REG_IA + (index << 2) ); // Get the
                                                               // address to
                                                               // jump from the
                                                               // Vector Table
        interrupt = false;
        if (addr == 0) {
            // Null entry, does nothing
            return;
        }

        // push %r0
        vcomp->WriteB(--r[SP], r[0] >> 24);
        vcomp->WriteB(--r[SP], r[0] >> 16);
        vcomp->WriteB(--r[SP], r[0] >> 8);
        vcomp->WriteB(--r[SP], r[0]); // Little Endian

        // push PC
        vcomp->WriteB(--r[SP], pc >> 24);
        vcomp->WriteB(--r[SP], pc >> 16);
        vcomp->WriteB(--r[SP], pc >> 8);
        vcomp->WriteB(--r[SP], pc); // Little Endian

        r[0] = int_msg;
        pc   = addr;
        SET_ON_IF(REG_FLAGS);
        sleeping = false; // WakeUp!
    }
} // ProcessInterrupt

void TR3200::GetState (void* ptr, std::size_t& size) const {
    if ( ptr != nullptr && size >= sizeof(TR3200State) ) {
        TR3200State* state = (TR3200State*)ptr;
        std::copy_n(this->r, TR3200_NGPRS, state->r);
        state->pc = this->pc;

        state->wait_cycles = this->wait_cycles;

        state->int_msg = this->int_msg;

        state->interrupt = this->interrupt;
        state->step_mode = this->step_mode;
        state->skiping   = this->skiping;
        state->sleeping  = this->sleeping;

        size = sizeof(TR3200State);
    }
    else {
        size = 0;
    }
} // GetState

bool TR3200::SetState (const void* ptr, std::size_t size) {
    if ( ptr != nullptr && size >= sizeof(TR3200State) ) {
        const TR3200State* state = (const TR3200State*)ptr;
        std::copy_n(state->r, TR3200_NGPRS, this->r);
        this->pc = state->pc;

        this->wait_cycles = state->wait_cycles;

        this->int_msg = state->int_msg;

        this->interrupt = state->interrupt;
        this->step_mode = state->step_mode;
        this->skiping   = state->skiping;
        this->sleeping  = state->sleeping;

        return true;
    }

    return false;
} // SetState
} // End of namespace cpu
} // End of namespace vm
