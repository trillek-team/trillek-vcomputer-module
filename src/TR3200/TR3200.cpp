/**
 * Trillek Virtual Computer - TR3200.cpp
 * Implementation of the TR3200 CPU v0.9b
 * @see https://github.com/trillek-team/trillek-computer/blob/master/TR3200.md
 */

#include "TR3200/TR3200.hpp"
#include "TR3200/TR3200_opcodes.hpp"
#include "TR3200/TR3200_macros.hpp"
#include "VSFix.hpp"

#include <cstdio>
#include <algorithm>

#include <cassert>

namespace vm {
  namespace cpu {

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
      skiping = false;
      sleeping = false;
    }

    unsigned TR3200::Step() {
      assert (vcomp != nullptr);

      if (!sleeping) {
        unsigned cyc = RealStep();
        return cyc;
      } else {
        ProcessInterrupt();
        return 1;
      }
    }

    void TR3200::Tick(unsigned n) {
      assert (vcomp != nullptr);

      unsigned i = 0;

      while (i < n) {
        if (!sleeping) {
          if (wait_cycles <= 0 ) {
            wait_cycles = RealStep();
          }

          wait_cycles--;
        } else {
          ProcessInterrupt();
        }

        i++;
      }
    }

    bool TR3200::SendInterrupt (word_t msg) {
      if (GET_EI(FLAGS)) {
        // The CPU accepts a new interrupt
        interrupt = true;
        int_msg = msg;
        return true;
      }

      return false;
    }

    /**
     * Executes a TR3200 instruction
     * @return Number of cycles that takes to do it
     */
    unsigned TR3200::RealStep() {
      unsigned wait_cycles = 0;

      dword_t inst = vcomp->ReadDW(pc);
      pc +=4;

      dword_t opcode, rd, rs, rn;
      bool literal = HAVE_LITERAL(inst);

      qword_t ltmp;

      rd = GRD(inst);
      rs = GRS(inst);

      // Check if we are skiping a instruction
      if (! skiping) {

        // Check the type of instruction
        if (IS_PAR3(inst)) {
          // 3 parameter instruction ********************************************
          wait_cycles += 3;
          opcode = (inst >> 24) & 0x3F;

          // Fetch RN and RS parameters
          if (literal) {
            rn = LIT13(inst);
            if (IS_BIG_LITERAL_L13(rn)) { // Next dword is literal value
              rn = vcomp->ReadDW(pc);
              pc +=4;
              wait_cycles++;
            } else if (O13_SIGN_BIT(rn)) { // Negative Literal -> Extend sign
              rn |= 0xFFFFF000;
            }
          } else {
            rn = r[GRN(inst)];
          }
          rs = r[rs];

          switch (opcode) {
            case P3_OPCODE::AND :
              r[rd] = rs & rn;
              SET_OFF_CF(FLAGS);
              SET_OFF_OF(FLAGS);
              break;

            case P3_OPCODE::OR :
              r[rd] = rs | rn;
              SET_OFF_CF(FLAGS);
              SET_OFF_OF(FLAGS);
              break;

            case P3_OPCODE::XOR :
              r[rd] = rs ^ rn;
              SET_OFF_CF(FLAGS);
              SET_OFF_OF(FLAGS);
              break;

            case P3_OPCODE::BITC :
              r[rd] = rs & (~rn);
              SET_OFF_CF(FLAGS);
              SET_OFF_OF(FLAGS);
              break;


            case P3_OPCODE::ADD :
              ltmp = ((qword_t)rs) + rn;
              if (CARRY_BIT(ltmp)) // We grab carry bit
                SET_ON_CF(FLAGS);
              else
                SET_OFF_CF(FLAGS);

              // If operands have same sign, check overflow
              if (DW_SIGN_BIT(rs) == DW_SIGN_BIT(rn)) {
                if (DW_SIGN_BIT(rn) != DW_SIGN_BIT(ltmp) ) {
                  // Overflow happens
                  SET_ON_OF(FLAGS);
                } else {
                  SET_OFF_OF(FLAGS);
                }
              }
              r[rd] = (dword_t)ltmp;
              break;

            case P3_OPCODE::ADDC :
              ltmp = ((qword_t)rs) + rn + GET_CF(FLAGS);
              if (CARRY_BIT(ltmp)) // We grab carry bit
                SET_ON_CF(FLAGS);
              else
                SET_OFF_CF(FLAGS);

              // If operands have same sign, check overflow
              if (DW_SIGN_BIT(rs) == DW_SIGN_BIT(rn)) {
                if (DW_SIGN_BIT(rn) != DW_SIGN_BIT(ltmp) ) {
                  // Overflow happens
                  SET_ON_OF(FLAGS);
                } else {
                  SET_OFF_OF(FLAGS);
                }
              }
              r[rd] = (dword_t)ltmp;
              break;

            case P3_OPCODE::SUB :
              ltmp = ((qword_t)rs) - rn;
              if (rs < rn) // We grab carry bit
                SET_ON_CF(FLAGS);
              else
                SET_OFF_CF(FLAGS);

              // If operands have distint sign, check overflow
              if (DW_SIGN_BIT(rs) != DW_SIGN_BIT(rn)) {
                if (DW_SIGN_BIT(rn) == DW_SIGN_BIT(ltmp) ) {
                  // Overflow happens
                  SET_ON_OF(FLAGS);
                  //if (GET_EOE(FLAGS)) {
                  //    SendInterrupt(4);
                  //}
                } else {
                  SET_OFF_OF(FLAGS);
                }
              }
              r[rd] = (dword_t)ltmp;
              break;

            case P3_OPCODE::SUBB :
              ltmp = ((qword_t)rs) - (rn + GET_CF(FLAGS));
              if (rs < (rn + GET_CF(FLAGS)) ) // We grab carry bit
                SET_ON_CF(FLAGS);
              else
                SET_OFF_CF(FLAGS);

              // If operands have distint sign, check overflow
              if (DW_SIGN_BIT(rs) != DW_SIGN_BIT(rn)) {
                if (DW_SIGN_BIT(rn) == DW_SIGN_BIT(ltmp) ) {
                  // Overflow happens
                  SET_ON_OF(FLAGS);
                  //if (GET_EOE(FLAGS)) {
                  //    SendInterrupt(4);
                  //}
                } else {
                  SET_OFF_OF(FLAGS);
                }
              }
              r[rd] = (dword_t)ltmp;
              break;

            case P3_OPCODE::RSB :
              ltmp = ((qword_t)rn) - rs;
              if (rn < rs) // We grab carry bit
                SET_ON_CF(FLAGS);
              else
                SET_OFF_CF(FLAGS);

              // If operands have same sign, check overflow
              if (DW_SIGN_BIT(rs) == DW_SIGN_BIT(rn)) {
                if (DW_SIGN_BIT(rn) != DW_SIGN_BIT(ltmp) ) {
                  // Overflow happens
                  SET_ON_OF(FLAGS);
                  //if (GET_EOE(FLAGS)) {
                  //    SendInterrupt(4);
                  //}
                } else {
                  SET_OFF_OF(FLAGS);
                }
              }
              r[rd] = (dword_t)ltmp;
              break;

            case P3_OPCODE::RSBB :
              ltmp = ((qword_t)rn) - (rs + GET_CF(FLAGS));
              if (rn < (rs + GET_CF(FLAGS)) ) // We grab carry bit
                SET_ON_CF(FLAGS);
              else
                SET_OFF_CF(FLAGS);

              // If operands have same sign, check overflow
              if (DW_SIGN_BIT(rs) == DW_SIGN_BIT(rn)) {
                if (DW_SIGN_BIT(rn) != DW_SIGN_BIT(ltmp) ) {
                  // Overflow happens
                  SET_ON_OF(FLAGS);
                } else {
                  SET_OFF_OF(FLAGS);
                }
              }
              r[rd] = (dword_t)ltmp;
              break;

            case P3_OPCODE::LLS :
              ltmp = ((qword_t)rs) << rn;
              if (CARRY_BIT(ltmp)) // We grab output bit
                SET_ON_CF(FLAGS);
              else
                SET_OFF_CF(FLAGS);
              SET_OFF_OF(FLAGS);
              r[rd] = (dword_t)ltmp;
              break;

            case P3_OPCODE::RLS :
              ltmp = ((qword_t)rs << 1) >> rn;
              if (ltmp & 1) // We grab output bit
                SET_ON_CF(FLAGS);
              else
                SET_OFF_CF(FLAGS);
              SET_OFF_OF(FLAGS);
              r[rd] = (dword_t)(ltmp >> 1);
              break;

            case P3_OPCODE::ARS : {
              sdword_t srs = rs;
              sdword_t srn = rn;

              sqword_t result = (((sqword_t)srs) << 1) >> srn; // Enforce to do arithmetic shift

              if (result & 1) // We grab output bit
                SET_ON_CF(FLAGS);
              else
                SET_OFF_CF(FLAGS);
              SET_OFF_OF(FLAGS);
              r[rd] = (dword_t)(result >> 1);
              break;
            }

            case P3_OPCODE::ROTL :
              r[rd] = rs << (rn%32);
              r[rd] |= rs >> (32 - (rn)%32);
              SET_OFF_OF(FLAGS);
              SET_OFF_CF(FLAGS);
              break;

            case P3_OPCODE::ROTR :
              r[rd] = rs >> (rn%32);
              r[rd] |= rs << (32 - (rn)%32);
              SET_OFF_OF(FLAGS);
              SET_OFF_CF(FLAGS);
              break;

            case P3_OPCODE::MUL :
              wait_cycles += 17;
              ltmp = ((qword_t)rs) * rn;
              RY = (dword_t)(ltmp >> 32);      // 32bit MSB of the 64 bit result
              r[rd] = (dword_t)ltmp;     // 32bit LSB of the 64 bit result
              SET_OFF_OF(FLAGS);
              SET_OFF_CF(FLAGS);
              break;

            case P3_OPCODE::SMUL : {
              wait_cycles += 27;
              sqword_t lword = (sqword_t)rs;
              lword *= rn;
              RY = (dword_t)(lword >> 32);     // 32bit MSB of the 64 bit result
              r[rd] = (dword_t)lword;    // 32bit LSB of the 64 bit result
              SET_OFF_OF(FLAGS);
              SET_OFF_CF(FLAGS);
              break;
            }

            case P3_OPCODE::DIV :
              wait_cycles += 27;
              if (rn != 0) {
               r[rd] = rs / rn;
               RY = rs % rn; // Compiler should optimize this and use a single instruction
              } else { // Division by 0
               SET_ON_DE(FLAGS);
              }
              SET_OFF_OF(FLAGS);
              SET_OFF_CF(FLAGS);
              break;


            case P3_OPCODE::SDIV : {
              wait_cycles += 37;
              if (rn != 0) {
                sdword_t srs = rs;
                sdword_t srn = rn;
                sdword_t result = srs / srn;
                r[rd] = result;
                result = srs % srn;
                RY = result;
              } else { // Division by 0
                SET_ON_DE(FLAGS);
              }
              SET_OFF_OF(FLAGS);
              SET_OFF_CF(FLAGS);

              break;
            }


            case P3_OPCODE::LOAD :
              r[rd] = vcomp->ReadDW(rs+rn);
              break;

            case P3_OPCODE::LOADW :
              r[rd] = vcomp->ReadW(rs+rn);
              break;

            case P3_OPCODE::LOADB :
              r[rd] = vcomp->ReadB(rs+rn);
              break;

            case P3_OPCODE::STORE :
              vcomp->WriteDW(rs+rn, r[rd]);
              /*
              vcomp->WriteB(rs+rn   , r[rd]);
              vcomp->WriteB(rs+rn +1, r[rd] >> 8);
              vcomp->WriteB(rs+rn +2, r[rd] >> 16);
              vcomp->WriteB(rs+rn +3, r[rd] >> 24);
              */
              break;

            case P3_OPCODE::STOREW :
              vcomp->WriteW(rs+rn, r[rd]);
              /*
              vcomp->WriteB(rs+rn   , r[rd]);
              vcomp->WriteB(rs+rn +1, r[rd] >> 8);
              */
              break;

            case P3_OPCODE::STOREB :
              vcomp->WriteB(rs+rn, r[rd]);
              break;

            default:
               break;// Unknow OpCode -> Acts like a NOP (this could change)
          }

        } else if (IS_PAR2(inst)) {
          // 2 parameter instrucction *******************************************

          wait_cycles += 3;
          opcode = (inst >> 24) & 0x7F;

          // Fetch Rn operand
          if (literal) {
            rn = LIT18(inst);
            if (IS_BIG_LITERAL_L18(rn)) { // Next dword is literal value
              rn = vcomp->ReadDW(pc);
              pc +=4;
              wait_cycles++;
            } else if (O18_SIGN_BIT(rn)) { // Negative Literal -> Extend sign
              rn |= 0xFFFC0000;
            }
          } else {
            rn = r[GRS(inst)];
          }

          switch (opcode) {
            case P2_OPCODE::MOV :
              r[rd] = rn;
              break;

            case P2_OPCODE::SWP :
              if (!literal) {
                dword_t tmp = r[rd];
                r[rd] = rn;
                r[GRS(inst)] = tmp;
              } // If M != acts like a NOP
              break;

            case P2_OPCODE::SIGXB :
              if ((rn & 0x00000080) != 0) {
                rd |= 0xFFFFFF00;   // Negative
              } else {
                rd &= 0x000000FF;   // Positive
              }
              break;

            case P2_OPCODE::SIGXW :
              if ((rn & 0x00008000) != 0) {
                rd |= 0xFFFF0000;   // Negative
              } else {
                rd &= 0x0000FFFF;   // Positive
              }
              break;

            case P2_OPCODE::LOAD2 :
              r[rd] = vcomp->ReadDW(rn);
              break;

            case P2_OPCODE::LOADW2 :
              r[rd] = vcomp->ReadW(rn);
              break;

            case P2_OPCODE::LOADB2 :
              r[rd] = vcomp->ReadB(rn);
              break;

            case P2_OPCODE::STORE2 :
              vcomp->WriteDW(rn, r[rd]);
              /*
              vcomp->WriteB(rn   , r[rd]);
              vcomp->WriteB(rn +1, r[rd] >> 8);
              vcomp->WriteB(rn +2, r[rd] >> 16);
              vcomp->WriteB(rn +3, r[rd] >> 24);
              */
              break;

            case P2_OPCODE::STOREW2 :
              vcomp->WriteW(rn, r[rd]);
              /*
              vcomp->WriteB(rn   , r[rd]);
              vcomp->WriteB(rn +1, r[rd] >> 8);
              */
              break;

            case P2_OPCODE::STOREB2 :
              vcomp->WriteB(rn, r[rd]);
              break;


            case P2_OPCODE::IFEQ :
              if (!(r[rd] == rn)) {
                skiping = true;
                wait_cycles++;
              }
              break;

            case P2_OPCODE::IFNEQ :
              if (!(r[rd] != rn)) {
                skiping = true;
                wait_cycles++;
              }
              break;

            case P2_OPCODE::IFL :
              if (!(r[rd] < rn)) {
                skiping = true;
                wait_cycles++;
              }
              break;

            case P2_OPCODE::IFSL : {
              sdword_t srd = r[rd];
              sdword_t srn = rn;
              if (!(srd < srn)) {
                skiping = true;
                wait_cycles++;
              }
              break;
            }

            case P2_OPCODE::IFLE :
              if (!(r[rd] <= rn)) {
                skiping = true;
                wait_cycles++;
              }
              break;

            case P2_OPCODE::IFSLE : {
              sdword_t srd = r[rd];
              sdword_t srn = rn;
              if (!(srd <= srn)) {
                skiping = true;
                wait_cycles++;
              }
              break;
            }

            case P2_OPCODE::IFBITS :
              if (! ((r[rd] & rn) != 0)) {
                skiping = true;
                wait_cycles++;
              }
              break;

            case P2_OPCODE::IFCLEAR :
              if (! ((r[rd] & rn) == 0)) {
                skiping = true;
                wait_cycles++;
              }
              break;

            case P2_OPCODE::JMP2 : // Absolute jump
              pc = (r[rd] + rn) & 0xFFFFFFFC;
              break;

            case P2_OPCODE::CALL2 : // Absolute call
              wait_cycles++;
              // push to the stack register pc value
              vcomp->WriteB(--r[SP], pc >> 24);
              vcomp->WriteB(--r[SP], pc >> 16);
              vcomp->WriteB(--r[SP], pc >> 8);
              vcomp->WriteB(--r[SP], pc); // Little Endian
              pc = (r[rd] + rn) & 0xFFFFFFFC;
              break;


            default:
              break; // Unknow OpCode -> Acts like a NOP (this could change)
          }

        } else if (IS_PAR1(inst)) {
          // 1 parameter instrucction *******************************************

          wait_cycles += 3;
          opcode = (inst >> 24) & 0x1F;

          // Fetch Rn operand
          if (literal) {
            rn = LIT22(inst);
            if (IS_BIG_LITERAL_L22(rn)) { // Next dword is literal value
              rn = vcomp->ReadDW(pc);
              pc +=4;
              wait_cycles++;
            } else if (O22_SIGN_BIT(rn)) { // Negative Literal -> Extend sign
              rn |= 0xFF800000;
            }
          } else {
            rn = GRD(inst);
          }

          switch (opcode) {
            case P1_OPCODE::XCHGB :
              if (!literal) {
                word_t lob = (r[rn]  & 0xFF) << 8;
                word_t hib = (r[rn]  >> 8) & 0xFF;
                r[rn] = (r[rn]  & 0xFFFF0000) | lob | hib;
              }
              break;

            case P1_OPCODE::XCHGW :
              if (!literal) {
                dword_t low = r[rn]  << 16;
                dword_t hiw = r[rn]  >> 16;
                r[rn] = low | hiw;
              }
              break;

            case P1_OPCODE::GETPC :
              if (!literal) {
                r[rn] = pc; // PC is alredy pointing to the next instruction
              }
              break;


            case P1_OPCODE::POP :
              if (!literal) {
                // SP always points to the last pushed element
                r[rn]  = vcomp->ReadDW(r[SP]);
                r[SP] += 4;
              }
              break;

            case P1_OPCODE::PUSH :
              // SP always points to the last pushed element
              if (!literal)
                rn = r[rn];
              vcomp->WriteB(--r[SP] , rn >> 24);
              vcomp->WriteB(--r[SP] , rn >> 16);
              vcomp->WriteB(--r[SP] , rn >> 8 );
              vcomp->WriteB(--r[SP] , rn      );
              break;


            case P1_OPCODE::JMP :   // Absolute jump
              if (!literal)
                rn = r[rn];
              pc = rn & 0xFFFFFFFC;
              break;

            case P1_OPCODE::CALL :  // Absolute call
              wait_cycles++;
              // push to the stack register pc value
              vcomp->WriteB(--r[SP], pc >> 24);
              vcomp->WriteB(--r[SP], pc >> 16);
              vcomp->WriteB(--r[SP], pc >> 8);
              vcomp->WriteB(--r[SP], pc); // Little Endian
              if (!literal)
                rn = r[rn];
              pc = rn & 0xFFFFFFFC;
              break;

            case P1_OPCODE::RJMP :  // Relative jump
              if (!literal)
                rn = r[rn];
              pc = (pc + rn) & 0xFFFFFFFC;
              break;

            case P1_OPCODE::RCALL : // Relative call
              wait_cycles++;
              // push to the stack register pc value
              vcomp->WriteB(--r[SP], pc >> 24);
              vcomp->WriteB(--r[SP], pc >> 16);
              vcomp->WriteB(--r[SP], pc >> 8);
              vcomp->WriteB(--r[SP], pc); // Little Endian
              if (!literal)
                rn = r[rn];
              pc = (pc + rn) & 0xFFFFFFFC;
              break;


            case P1_OPCODE::INT : // Software Interrupt
              wait_cycles += 3;
              if (!literal)
                rn = r[rn];
              SendInterrupt(rn);
              break;

            default:
              break; // Unknow OpCode -> Acts like a NOP (this could change)
          }

        } else if (IS_NP(inst)) {
          // Instructions without parameters ************************************
          opcode = inst & 0x0FFFFFFF; // OpCode uses the 16 LSB

          switch (opcode) {
            case NP_OPCODE::SLEEP :
              wait_cycles = 1;
              sleeping = true;
              break;

            case NP_OPCODE::RET:
              wait_cycles = 4;
              // Pop PC
              pc = vcomp->ReadDW(r[SP]);
              r[SP] += 4;
              /*
              pc = vcomp->ReadB(r[SP]++);
              pc |= vcomp->ReadB(r[SP]++) << 8;
              pc |= vcomp->ReadB(r[SP]++) << 16;
              pc |= vcomp->ReadB(r[SP]++) << 24;
              */
              pc &= 0xFFFFFFFC;
              break;

            case NP_OPCODE::RFI :
              wait_cycles = 6;

              // Pop PC
              pc = vcomp->ReadDW(r[SP]);
              r[SP] += 4;
              /*
              pc = vcomp->ReadB(r[SP]++);
              pc |= vcomp->ReadB(r[SP]++) << 8;
              pc |= vcomp->ReadB(r[SP]++) << 16;
              pc |= vcomp->ReadB(r[SP]++) << 24;
              */
              pc &= 0xFFFFFFFC;

              // Pop %r0
              r[0] = vcomp->ReadDW(r[SP]);
              r[SP] += 4;
              /*
              r[0] = vcomp->ReadB(r[SP]++);
              r[0] |= vcomp->ReadB(r[SP]++) << 8;
              r[0] |= vcomp->ReadB(r[SP]++) << 16;
              r[0] |= vcomp->ReadB(r[SP]++) << 24;
              */

              SET_OFF_IF(FLAGS);
              interrupt = false; // We now not have a interrupt
              break;

            default:
              // Unknow OpCode -> Acts like a NOP
              wait_cycles = 1;

          }
        }

        // If step-mode is enable, Throw the adequate exception
        if (step_mode && ! GET_IF(FLAGS)) {
          SendInterrupt(0);
        }

        ProcessInterrupt(); // Here we check if a interrupt happens

        return wait_cycles;

      } else { // Skiping an instruction
        wait_cycles += 1;
        skiping = false;

        // See what kind of instruction is to know how many should
        // increment PC, and remove skiping flag if is not an IFxxx instruction
        if (literal) {
          if (IS_PAR3(inst)) {
            // 3 parameter instruction
            rn = LIT13(inst);
            if (IS_BIG_LITERAL_L13(rn) )
              pc +=4;

          } else if (IS_PAR2(inst)) {
            // 2 parameter instruction
            skiping = IS_BRANCH(inst); // Chain IFxx
            rn = LIT18(inst);
            if (literal && IS_BIG_LITERAL_L18(rn) )
              pc +=4;

          } else if (IS_PAR1(inst)) {
            // 1 parameter instruction
            rn = LIT22(inst);
            if (literal && IS_BIG_LITERAL_L22(rn) )
              pc +=4;

          }

        } else if (IS_PAR2(inst) && IS_BRANCH(inst)) {
          skiping = true; // Chain IFxx
        }

        return wait_cycles;
      }

    }

    /**
     * Check if there is an interrupt to be procesed
     */
    void TR3200::ProcessInterrupt() {
      if (GET_EI(FLAGS) && interrupt) {
        byte_t index = int_msg;
        dword_t addr = vcomp->ReadDW(IA + (index << 2)); // Get the address to jump from the Vector Table
        interrupt = false;
        if (addr == 0) { // Null entry, does nothing
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
        pc = addr;
        SET_ON_IF(FLAGS);
        sleeping = false; // WakeUp!
      }
    }

    void TR3200::GetState (void* ptr, std::size_t& size) const {
      if (ptr != nullptr && size >= sizeof(TR3200State)) {
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
      } else {
        size = 0;
      }
    }

    bool TR3200::SetState (const void* ptr, std::size_t size) {
      if (ptr != nullptr && size >= sizeof(TR3200State)) {
        const TR3200State* state = (const TR3200State*)ptr;
        std::copy_n(state->r, TR3200_NGPRS, this->r);
        this->pc          = state->pc;

        this->wait_cycles = state->wait_cycles;

        this->int_msg     = state->int_msg;

        this->interrupt   = state->interrupt;
        this->step_mode   = state->step_mode;
        this->skiping     = state->skiping;
        this->sleeping    = state->sleeping;

        return true;
      }

      return false;
    }


  } // End of namespace cpu
} // End of namespace vm
