/**
 * TR3200 VM - cpu.cpp
 * CPU of the virtual machine
 */

#include "TR3200.hpp"
#include "VSFix.hpp"

#include <iostream>
#include <cstdio>
#include <algorithm>

#include <cassert>

namespace vm {
	namespace cpu {

		TR3200::TR3200(size_t ram_size, unsigned clock) : ICpu(ram_size, clock) { 
		}

		TR3200::~TR3200() {
		}

		void TR3200::Reset() {
			std::fill_n(state.r, TR3200_NGPRS, 0);
			state.pc = 0;
			state.step_mode = false;

			ICpu::Reset();
		}

		/**
		 * Executes a TR3200 instruction
		 * @return Number of cycles that takes to do it
		 */
		unsigned TR3200::RealStep() {
			wait_cycles = 0;

			dword_t inst = ram.RD(state.pc);
			state.pc +=4;

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
							rn = ram.RD(state.pc);
							state.pc +=4;
							wait_cycles++;
						} else if (O13_SIGN_BIT(rn)) { // Negative Literal -> Extend sign
							rn |= 0xFFFFF000;
						}
					} else {
						rn = state.r[GRN(inst)];
					}
					rs = state.r[rs];

					switch (opcode) {
						case P3_OPCODE::AND :
							state.r[rd] = rs & rn;
							SET_OFF_CF(FLAGS);
							SET_OFF_OF(FLAGS);
							break;

						case P3_OPCODE::OR :
							state.r[rd] = rs | rn;
							SET_OFF_CF(FLAGS);
							SET_OFF_OF(FLAGS);
							break;

						case P3_OPCODE::XOR :
							state.r[rd] = rs ^ rn;
							SET_OFF_CF(FLAGS);
							SET_OFF_OF(FLAGS);
							break;

						case P3_OPCODE::BITC :
							state.r[rd] = rs & (~rn);
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
									//if (GET_EOE(FLAGS)) {
									//    ThrowInterrupt(4);
									//}
								} else {
									SET_OFF_OF(FLAGS);
								}
							}
							state.r[rd] = (dword_t)ltmp;
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
									//if (GET_EOE(FLAGS)) {
									//    ThrowInterrupt(4);
									//}
								} else {
									SET_OFF_OF(FLAGS);
								}
							}
							state.r[rd] = (dword_t)ltmp;
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
									//    ThrowInterrupt(4);
									//}
								} else {
									SET_OFF_OF(FLAGS);
								}
							}
							state.r[rd] = (dword_t)ltmp;
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
									//    ThrowInterrupt(4);
									//}
								} else {
									SET_OFF_OF(FLAGS);
								}
							}
							state.r[rd] = (dword_t)ltmp;
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
									//    ThrowInterrupt(4);
									//}
								} else {
									SET_OFF_OF(FLAGS);
								}
							}
							state.r[rd] = (dword_t)ltmp;
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
									//if (GET_EOE(FLAGS)) {
									//    ThrowInterrupt(4);
									//}
								} else {
									SET_OFF_OF(FLAGS);
								}
							}
							state.r[rd] = (dword_t)ltmp;
							break;

						case P3_OPCODE::LLS :
							ltmp = ((qword_t)rs) << rn;
							if (CARRY_BIT(ltmp)) // We grab output bit
								SET_ON_CF(FLAGS);
							else
								SET_OFF_CF(FLAGS);
							SET_OFF_OF(FLAGS);
							state.r[rd] = (dword_t)ltmp;
							break;

						case P3_OPCODE::RLS :
							ltmp = ((qword_t)rs << 1) >> rn;
							if (ltmp & 1) // We grab output bit
								SET_ON_CF(FLAGS);
							else
								SET_OFF_CF(FLAGS);
							SET_OFF_OF(FLAGS);
							state.r[rd] = (dword_t)(ltmp >> 1);
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
																		state.r[rd] = (dword_t)(result >> 1);
																		break;
																	}

						case P3_OPCODE::ROTL :
																	state.r[rd] = rs << (rn%32);
																	state.r[rd] |= rs >> (32 - (rn)%32);
																	SET_OFF_OF(FLAGS);
																	SET_OFF_CF(FLAGS);
																	break;

						case P3_OPCODE::ROTR :
																	state.r[rd] = rs >> (rn%32);
																	state.r[rd] |= rs << (32 - (rn)%32);
																	SET_OFF_OF(FLAGS);
																	SET_OFF_CF(FLAGS);
																	break;

						case P3_OPCODE::MUL :
																	wait_cycles += 17;
																	ltmp = ((qword_t)rs) * rn;
																	RY = (dword_t)(ltmp >> 32);      // 32bit MSB of the 64 bit result
																	state.r[rd] = (dword_t)ltmp;     // 32bit LSB of the 64 bit result
																	SET_OFF_OF(FLAGS);
																	SET_OFF_CF(FLAGS);
																	break;

						case P3_OPCODE::SMUL : {
																		 wait_cycles += 27;
																		 sqword_t lword = (sqword_t)rs;
																		 lword *= rn;
																		 RY = (dword_t)(lword >> 32);     // 32bit MSB of the 64 bit result
																		 state.r[rd] = (dword_t)lword;    // 32bit LSB of the 64 bit result
																		 SET_OFF_OF(FLAGS);
																		 SET_OFF_CF(FLAGS);
																		 break;
																	 }

						case P3_OPCODE::DIV :
																	 wait_cycles += 27;
																	 if (rn != 0) {
																		 state.r[rd] = rs / rn;
																		 RY = rs % rn; // Compiler should optimize this and use a single instruction
																	 } else { // Division by 0
																		 SET_ON_DE(FLAGS);
																		 //if ( GET_EDE(FLAGS)) {
																		 //    ThrowInterrupt(0);
																		 //}
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
																			 state.r[rd] = result;
																			 result = srs % srn;
																			 RY = result;
																		 } else { // Division by 0
																			 SET_ON_DE(FLAGS);
																			 //if ( GET_EDE(FLAGS)) {
																			 //    ThrowInterrupt(0);
																			 //}
																		 }
																		 SET_OFF_OF(FLAGS);
																		 SET_OFF_CF(FLAGS);

																		 break;
																	 }


						case P3_OPCODE::LOAD :
																	 state.r[rd] = ram.RD(rs+rn);
																	 break;

						case P3_OPCODE::LOADW :
																	 state.r[rd] = ram.RW(rs+rn);
																	 break;

						case P3_OPCODE::LOADB :
																	 state.r[rd] = ram.RB(rs+rn);
																	 break;

						case P3_OPCODE::STORE :
																	 ram.WB(rs+rn   , state.r[rd]);
																	 ram.WB(rs+rn +1, state.r[rd] >> 8);
																	 ram.WB(rs+rn +2, state.r[rd] >> 16);
																	 ram.WB(rs+rn +3, state.r[rd] >> 24);
																	 break;

						case P3_OPCODE::STOREW :
																	 ram.WB(rs+rn   , state.r[rd]);
																	 ram.WB(rs+rn +1, state.r[rd] >> 8);
																	 break;

						case P3_OPCODE::STOREB :
																	 ram.WB(rs+rn   , state.r[rd]);
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
							rn = ram.RD(state.pc);
							state.pc +=4;
							wait_cycles++;
						} else if (O18_SIGN_BIT(rn)) { // Negative Literal -> Extend sign
							rn |= 0xFFFC0000;
						}
					} else {
						rn = state.r[GRS(inst)];
					}

					switch (opcode) {
						case P2_OPCODE::MOV :
							state.r[rd] = rn;
							break;

						case P2_OPCODE::SWP :
							if (!literal) {
								auto tmp = rn;
								rn = state.r[rd];
								state.r[rd] = tmp;
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

						case P2_OPCODE::NOT :
							state.r[rd] = ~ rn;
							break;


						case P2_OPCODE::LOAD2 :
							state.r[rd] = ram.RD(rn);
							break;

						case P2_OPCODE::LOADW2 :
							state.r[rd] = ram.RW(rn);
							break;

						case P2_OPCODE::LOADB2 :
							state.r[rd] = ram.RB(rn);
							break;

						case P2_OPCODE::STORE2 :
							ram.WB(rn   , state.r[rd]);
							ram.WB(rn +1, state.r[rd] >> 8);
							ram.WB(rn +2, state.r[rd] >> 16);
							ram.WB(rn +3, state.r[rd] >> 24);
							break;

						case P2_OPCODE::STOREW2 :
							ram.WB(rn   , state.r[rd]);
							ram.WB(rn +1, state.r[rd] >> 8);
							break;

						case P2_OPCODE::STOREB2 :
							ram.WB(rn   , state.r[rd]);
							break;


						case P2_OPCODE::IFEQ :
							if (!(state.r[rd] == rn)) {
								skiping = true;
								wait_cycles++;
							}
							break;

						case P2_OPCODE::IFNEQ :
							if (!(state.r[rd] != rn)) {
								skiping = true;
								wait_cycles++;
							}
							break;

						case P2_OPCODE::IFL :
							if (!(state.r[rd] < rn)) {
								skiping = true;
								wait_cycles++;
							}
							break;

						case P2_OPCODE::IFSL : {
																		 sdword_t srd = state.r[rd];
																		 sdword_t srn = rn;
																		 if (!(srd < srn)) {
																			 skiping = true;
																			 wait_cycles++;
																		 }
																		 break;
																	 }

						case P2_OPCODE::IFLE :
																	 if (!(state.r[rd] <= rn)) {
																		 skiping = true;
																		 wait_cycles++;
																	 }
																	 break;

						case P2_OPCODE::IFSLE : {
																			sdword_t srd = state.r[rd];
																			sdword_t srn = rn;
																			if (!(srd <= srn)) {
																				skiping = true;
																				wait_cycles++;
																			}
																			break;
																		}

						case P2_OPCODE::IFBITS :
																		if (! ((state.r[rd] & rn) != 0)) {
																			skiping = true;
																			wait_cycles++;
																		}
																		break;

						case P2_OPCODE::IFCLEAR :
																		if (! ((state.r[rd] & rn) == 0)) {
																			skiping = true;
																			wait_cycles++;
																		}
																		break;

						case P2_OPCODE::JMP2 : // Absolute jump
																		state.pc = (state.r[rd] + rn) & 0xFFFFFFFC;
																		break;

						case P2_OPCODE::CALL2 : // Absolute call
																		wait_cycles++;
																		// push to the stack register pc value
																		ram.WB(--state.r[SP], state.pc >> 24);
																		ram.WB(--state.r[SP], state.pc >> 16);
																		ram.WB(--state.r[SP], state.pc >> 8);
																		ram.WB(--state.r[SP], state.pc); // Little Endian
																		state.pc = (state.r[rd] + rn) & 0xFFFFFFFC;
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
							rn = ram.RD(state.pc);
							state.pc +=4;
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
								word_t lob = (state.r[rn]  & 0xFF) << 8;
								word_t hib = (state.r[rn]  >> 8) & 0xFF;
								state.r[rn] = (state.r[rn]  & 0xFFFF0000) | lob | hib;
							}
							break;

						case P1_OPCODE::XCHGW :
							if (!literal) {
								dword_t low = state.r[rn]  << 16;
								dword_t hiw = state.r[rn]  >> 16;
								state.r[rn] = low | hiw;
							}
							break;

						case P1_OPCODE::GETPC :
							if (!literal) {
								state.r[rn] = state.pc; // PC is alredy pointing to the next instruction
							}
							break;


						case P1_OPCODE::POP :
							if (!literal) {
								// SP always points to the last pushed element
								state.r[rn]  = ram.RD(state.r[SP]);
								state.r[SP] += 4;
							}
							break;

						case P1_OPCODE::PUSH :
							// SP always points to the last pushed element
							if (!literal)
								rn = state.r[rn]; 
							ram.WB(--state.r[SP] , rn >> 24);
							ram.WB(--state.r[SP] , rn >> 16);
							ram.WB(--state.r[SP] , rn >> 8 );
							ram.WB(--state.r[SP] , rn      );
							break;


						case P1_OPCODE::JMP :   // Absolute jump
							if (!literal)
								rn = state.r[rn]; 
							state.pc = rn & 0xFFFFFFFC;
							break;

						case P1_OPCODE::CALL :  // Absolute call
							wait_cycles++;
							// push to the stack register pc value
							ram.WB(--state.r[SP], state.pc >> 24);
							ram.WB(--state.r[SP], state.pc >> 16);
							ram.WB(--state.r[SP], state.pc >> 8);
							ram.WB(--state.r[SP], state.pc); // Little Endian
							if (!literal)
								rn = state.r[rn]; 
							state.pc = rn & 0xFFFFFFFC;
							break;

						case P1_OPCODE::RJMP :  // Relative jump
							if (!literal)
								rn = state.r[rn]; 
							state.pc = (state.pc + rn) & 0xFFFFFFFC;
							break;

						case P1_OPCODE::RCALL : // Relative call
							wait_cycles++;
							// push to the stack register pc value
							ram.WB(--state.r[SP], state.pc >> 24);
							ram.WB(--state.r[SP], state.pc >> 16);
							ram.WB(--state.r[SP], state.pc >> 8);
							ram.WB(--state.r[SP], state.pc); // Little Endian
							if (!literal)
								rn = state.r[rn]; 
							state.pc = (state.pc + rn) & 0xFFFFFFFC;
							break;


						case P1_OPCODE::INT : // Software Interrupt
							wait_cycles += 3;
							if (!literal)
								rn = state.r[rn]; 
							ThrowInterrupt(rn);
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
							state.pc = ram.RB(state.r[SP]++);
							state.pc |= ram.RB(state.r[SP]++) << 8;
							state.pc |= ram.RB(state.r[SP]++) << 16;
							state.pc |= ram.RB(state.r[SP]++) << 24;
							state.pc &= 0xFFFFFFFC;
							break;

						case NP_OPCODE::RFI :
							wait_cycles = 6;

							// Pop PC
							state.pc = ram.RB(state.r[SP]++);
							state.pc |= ram.RB(state.r[SP]++) << 8;
							state.pc |= ram.RB(state.r[SP]++) << 16;
							state.pc |= ram.RB(state.r[SP]++) << 24;
							state.pc &= 0xFFFFFFFC;

							// Pop %r0
							state.r[0] = ram.RB(state.r[SP]++);
							state.r[0] |= ram.RB(state.r[SP]++) << 8;
							state.r[0] |= ram.RB(state.r[SP]++) << 16;
							state.r[0] |= ram.RB(state.r[SP]++) << 24;

							SET_OFF_IF(FLAGS);
							interrupt = false; // We now not have a interrupt
							break;

						default:
							// Unknow OpCode -> Acts like a NOP 
							wait_cycles = 1;

					}
				}

				// If step-mode is enable, Throw the adequate exception
				if (state.step_mode && ! GET_IF(FLAGS))
					ThrowInterrupt(0);

				ProcessInterrupt(); // Here we check if a interrupt happens


				return wait_cycles;

			} else {
				wait_cycles += 1;
				skiping = false;

				// See what kind of instruction is to know how many should
				// increment PC, and remove skiping flag if is not a branch
				if (literal) {
					if (IS_PAR3(inst)) {
						// 3 parameter instruction
						rn = LIT13(inst);
						if (IS_BIG_LITERAL_L13(rn) )
							state.pc +=4;
					} else if (IS_PAR2(inst)) {
						// 2 parameter instruction
						skiping = IS_BRANCH(inst); // Chain IFxx
						rn = LIT18(inst);
						if (literal && IS_BIG_LITERAL_L18(rn) )
							state.pc +=4;
					} else if (IS_PAR1(inst)) { 
						// 1 parameter instruction
						rn = LIT22(inst);
						if (literal && IS_BIG_LITERAL_L22(rn) )
							state.pc +=4;
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
				dword_t addr = ram.RD(IA + index*4); // Get the address to jump from the Vector Table
				interrupt = false;
				if (addr == 0) { // Null entry, does nothing
					return;
				}

				// push %r0
				ram.WB(--state.r[SP], state.r[0] >> 24);
				ram.WB(--state.r[SP], state.r[0] >> 16);
				ram.WB(--state.r[SP], state.r[0] >> 8);
				ram.WB(--state.r[SP], state.r[0]); // Little Endian

				// push PC
				ram.WB(--state.r[SP], state.pc >> 24);
				ram.WB(--state.r[SP], state.pc >> 16);
				ram.WB(--state.r[SP], state.pc >> 8);
				ram.WB(--state.r[SP], state.pc); // Little Endian

				state.r[0] = int_msg;
				state.pc = addr;
				SET_ON_IF(FLAGS);
				sleeping = false; // WakeUp! 
			}
		}

	} // End of namespace cpu
} // End of namespace vm
