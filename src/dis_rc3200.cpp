#include "dis_rc3200.hpp"
#include "rc3200_opcodes.hpp"

#include <cstdio>

namespace vm {
namespace cpu {

std::string disassembly(const ram::Mem& ram, dword_t pc) {
#define BUF_SIZE (32)
	char buf[BUF_SIZE] = {0};
    
    dword_t inst = ram.rb(pc++); // Fetch
    inst |= (ram.rb(pc++) << 8);
    inst |= (ram.rb(pc++) << 16);
    inst |= (ram.rb(pc++) << 24); // Little Endian

    dword_t opcode, r1, r2, r3;
    // Here beging the Decoding
    bool literal = HAVE_LITERAL(inst); 

    r3 = OP3(inst);
    r2 = OP2(inst);
    r1 = OP1(inst);
    opcode = (inst >> 16) & 0x0FFF;

    // Check the type of instruction
    if (IS_PAR3(inst)) {
        // 3 parameter instruction ********************************************
        
        if (literal) {
            if (IS_BIG_LITERAL_P3(r2)) { // Next dword is literal value 
                r2 = ram.rb(pc++);
                r2 |= (ram.rb(pc++) << 8);
                r2 |= (ram.rb(pc++) << 16);
                r2 |= (ram.rb(pc++) << 24); // Little Endian
            } else if (O5_SIGN_BIT(r2)) { // Negative Literal -> Extend sign
                r2 |= 0xFFFFFFE0;
            }
        } 
        
        if (IS_RAM(inst)) { // LOAD/STORE instruction
            switch (opcode) {
                case OPCODE_RAM::LOAD :
                    if (literal)
				        snprintf(buf, BUF_SIZE, "LOAD %%r%u + 0x%08x, %%r%u", r1, r2, r3);
                    else
				        snprintf(buf, BUF_SIZE, "LOAD %%r%u + %%r%u, %%r%u", r1, r2, r3);
                    break;

                case OPCODE_RAM::LOADW :
                    if (literal)
				        snprintf(buf, BUF_SIZE, "LOAD.W %%r%u + 0x%08x, %%r%u", r1, r2, r3);
                    else
				        snprintf(buf, BUF_SIZE, "LOAD.W %%r%u + %%r%u, %%r%u", r1, r2, r3);
                    break;

                case OPCODE_RAM::LOADB :
                    if (literal)
				        snprintf(buf, BUF_SIZE, "LOAD.B %%r%u + 0x%08x, %%r%u", r1, r2, r3);
                    else
				        snprintf(buf, BUF_SIZE, "LOAD.B %%r%u + %%r%u, %%r%u", r1, r2, r3);
                    break;

                case OPCODE_RAM::STORE :
                    if (literal)
				        snprintf(buf, BUF_SIZE, "STORE %%r%u + 0x%08x, %%r%u", r1, r2, r3);
                    else
				        snprintf(buf, BUF_SIZE, "STORE %%r%u + %%r%u, %%r%u", r1, r2, r3);
                    break;

                case OPCODE_RAM::STOREW :
                    if (literal)
				        snprintf(buf, BUF_SIZE, "STORE.W %%r%u + 0x%08x, %%r%u", r1, r2, r3);
                    else
				        snprintf(buf, BUF_SIZE, "STORE.W %%r%u + %%r%u, %%r%u", r1, r2, r3);
                    break;

                case OPCODE_RAM::STOREB :
                    if (literal)
				        snprintf(buf, BUF_SIZE, "STORE.B %%r%u + 0x%08x, %%r%u", r1, r2, r3);
                    else
				        snprintf(buf, BUF_SIZE, "STORE.B %%r%u + %%r%u, %%r%u", r1, r2, r3);
                    break;

                default:
                    if (literal)
                        snprintf(buf, BUF_SIZE, "???? %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "???? %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
            }
        } else { // ALU operations of type r3 = r1 op r2 
            switch (opcode) {
                case PAR3_OPCODE_ALU::ADD :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "ADD %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "ADD %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;

                case PAR3_OPCODE_ALU::SUB :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "SUB %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "SUB %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::ADDC :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "ADDC %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "ADDC %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
                
                case PAR3_OPCODE_ALU::SUBB :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "SUBB %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "SUBB %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::AND :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "AND %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "AND %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::OR :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "OR %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "OR %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::XOR :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "XOR %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "XOR %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::NAND :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "NAND %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "NAND %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::SRL :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "SRL %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "SRL %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::SLL :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "SLL %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "SLL %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::SRA :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "SRA %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "SRA %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::ROTL :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "ROTL %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "ROTL %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::ROTR :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "ROTR %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "ROTR %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
                
                case PAR3_OPCODE_ALU::UMUL :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "UMUL %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "UMUL %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::UDIV :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "UDIV %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "UDIV %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::MUL :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "MUL %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "MUL %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                case PAR3_OPCODE_ALU::DIV :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "DIV %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "DIV %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
                    break;
            
                default:
                    if (literal)
                        snprintf(buf, BUF_SIZE, "???? %%r%u , %u, %%r%u", r1, r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "???? %%r%u , %%r%u, %%r%u", r1, r2 ,r3);
            
            }
        }
    } else if (IS_PAR2(inst)) {
        // 2 parameter instrucction *******************************************
       
        if (literal) {
            r2 = (inst >> 5) & 0x3FF;
            if (IS_BIG_LITERAL_P2(r2)) { // Next dword is literal value 
                r2 = ram.rb(pc++);
                r2 |= (ram.rb(pc++) << 8);
                r2 |= (ram.rb(pc++) << 16);
                r2 |= (ram.rb(pc++) << 24); // Little Endian
            } else if (O10_SIGN_BIT(r2)) { // Negative Literal -> Extend sign
                r2 |= 0xFFFFFC00;
            }
        }
        
        if (IS_RAM(inst) && literal) { // LOAD/STORE instruction
            switch (opcode) {
                case OPCODE_RAM::LOAD :
				    snprintf(buf, BUF_SIZE, "LOAD 0x%08x, %%r%u", r2, r3);
                    break;
                
                case OPCODE_RAM::LOADW :
				    snprintf(buf, BUF_SIZE, "LOAD.W 0x%08x, %%r%u", r2, r3);
                    break;

                case OPCODE_RAM::LOADB :
				    snprintf(buf, BUF_SIZE, "LOAD.B 0x%08x, %%r%u", r2, r3);
                    break;

                case OPCODE_RAM::STORE :
				    snprintf(buf, BUF_SIZE, "STORE 0x%08x, %%r%u", r2, r3);
                    break;
                
                case OPCODE_RAM::STOREW :
				    snprintf(buf, BUF_SIZE, "STORE.W 0x%08x, %%r%u", r2, r3);
                    break;

                case OPCODE_RAM::STOREB :
				    snprintf(buf, BUF_SIZE, "STORE.B 0x%08x, %%r%u", r2, r3);
                    break;

                default:
                    snprintf(buf, BUF_SIZE, "???? 0x%08x, %%r%u", r2 ,r3);

            }
        } else if (IS_BRANCH(inst)) { // BRANCHing instruction
            switch (opcode) {
                case OPCODE_BRANCH::IFEQ :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFEQ %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFEQ %%r%u, %%r%u", r2 ,r3);
                    break;
                
                case OPCODE_BRANCH::IFNEQ :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFNEQ %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFNEQ %%r%u, %%r%u", r2 ,r3);
                    break;
                
                case OPCODE_BRANCH::IFUG :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFUG %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFUG %%r%u, %%r%u", r2 ,r3);
                    break;
            
                case OPCODE_BRANCH::IFG :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFG %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFG %%r%u, %%r%u", r2 ,r3);
                    break;
            
                case OPCODE_BRANCH::IFUGE :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFUGE %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFUGE %%r%u, %%r%u", r2 ,r3);
                    break;
            
                case OPCODE_BRANCH::IFGE :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFGE %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFGE %%r%u, %%r%u", r2 ,r3);
                    break;
            
                
                case OPCODE_BRANCH::IFUL :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFUL %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFUL %%r%u, %%r%u", r2 ,r3);
                    break;
            
                case OPCODE_BRANCH::IFL :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFL %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFL %%r%u, %%r%u", r2 ,r3);
                    break;
            
                case OPCODE_BRANCH::IFULE :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFULE %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFULE %%r%u, %%r%u", r2 ,r3);
                    break;
            
                case OPCODE_BRANCH::IFLE :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFLE %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFLE %%r%u, %%r%u", r2 ,r3);
                    break;
            
            
                case OPCODE_BRANCH::IFBITS :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFBITS %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFBITS %%r%u, %%r%u", r2 ,r3);
                    break;
                
                case OPCODE_BRANCH::IFCLEAR :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IFCLEAR %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IFCLEAR %%r%u, %%r%u", r2 ,r3);
                    break;
            
                default:
                    if (literal)
                        snprintf(buf, BUF_SIZE, "IF???? %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "IF???? %%r%u, %%r%u", r2 ,r3);
            }
        } else if (IS_JUMP(inst)) { // JUMP/CALL instruction
            switch (opcode) {
                case OPCODE_JUMP::JMP : // Absolute jump
                    if (literal)
                        snprintf(buf, BUF_SIZE, "JMP 0x%08x, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "JMP %%r%u, %%r%u", r2 ,r3);
                    break;
                
                case OPCODE_JUMP::CALL : // Absolute jump
                    if (literal)
                        snprintf(buf, BUF_SIZE, "CALL 0x%08x, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "CALL %%r%u, %%r%u", r2 ,r3);
                    break;

                default:
                    if (literal)
                        snprintf(buf, BUF_SIZE, "J???? %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "J???? %%r%u, %%r%u", r2 ,r3);
            }
        } else { // Register manipulation instruction
            switch (opcode) {
                case PAR2_OPCODE::CPY_SET :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "SET 0x%08x, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "CPY %%r%u, %%r%u", r2 ,r3);
                    break;
                
                case PAR2_OPCODE::SWP :
                    if (literal)
                        snprintf(buf, BUF_SIZE, "R???? %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "SWP %%r%u, %%r%u", r2 ,r3);
                    break;


                default:
                    if (literal)
                        snprintf(buf, BUF_SIZE, "R???? %u, %%r%u", r2 ,r3);
                    else
                        snprintf(buf, BUF_SIZE, "R???? %%r%u, %%r%u", r2 ,r3);
            }
        }

    } else if (IS_PAR1(inst)) {
        // 1 parameter instrucction *******************************************
        
        if (literal) {
            r3 = inst & 0x7FFF;
            if (IS_BIG_LITERAL_P1(r3)) { // Next dword is literal value 
                r3 = ram.rb(pc++);
                r3 |= (ram.rb(pc++) << 8);
                r3 |= (ram.rb(pc++) << 16);
                r3 |= (ram.rb(pc++) << 24); // Little Endian
            } else if (O15_SIGN_BIT(r3)) { // Negative Literal -> Extend sign
                r3 |= 0xFFFF8000;
            }
        }

        if (IS_RAM(inst)) { // Stack instructions
            if (opcode == OPCODE_RAM::PUSH) {
                    if (literal)
                        snprintf(buf, BUF_SIZE, "PUSH 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "PUSH %%r%u", r3);
            } else if (opcode == OPCODE_RAM::POP) {
                    if (literal)
                        snprintf(buf, BUF_SIZE, "Pop?? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "POP %%r%u", r3);
            } else {
                if (literal)
                    snprintf(buf, BUF_SIZE, "RAM??? 0x%08x", r3);
                else
                    snprintf(buf, BUF_SIZE, "RAM??? \r%u", r3);
            }
        } else if (IS_JUMP(inst)) { // Jump/Call instructions
            switch (opcode) {
                case OPCODE_JUMP::INT : // Software interrupt
                    if (literal)
                        snprintf(buf, BUF_SIZE, "INT %08Xh", r3);
                    else
                        snprintf(buf, BUF_SIZE, "INT %%r%u", r3);
                    break;
                
                case OPCODE_JUMP::JMP : // Relative 
                    if (literal)
                        snprintf(buf, BUF_SIZE, "JMP %%pc + 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "JMP %%pc +%%r%u", r3);
                    break;

                case OPCODE_JUMP::CALL : 
                    if (literal)
                        snprintf(buf, BUF_SIZE, "CALL %%pc + 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "CALL %%pc + %%r%u", r3);
                    break;

                default:
                    if (literal)
                        snprintf(buf, BUF_SIZE, "J???? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "J???? %%r%u", r3);
            }

        } else { // ALU / REGISTER instructions
            switch (opcode) {
                case PAR1_OPCODE::NOT :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "NOT? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "NOT %%r%u", r3);
                    break;
                
                case PAR1_OPCODE::NEG :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "NEG? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "NEG %%r%u", r3);
                    break;
                
                case PAR1_OPCODE::XCHG :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "XCHG? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "XCHG %%r%u", r3);
                    break;
                
                case PAR1_OPCODE::XCHGW :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "XCHGW? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "XCHGW %%r%u", r3);
                    break;
                
                case PAR1_OPCODE::SXTBD :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "SXTBD? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "SXTBD %%r%u", r3);
                    break;
                
                case PAR1_OPCODE::SXTWD :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "SXTWD? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "SXTWD %%r%u", r3);
                    break;
               

                case PAR1_OPCODE::GETPC :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "GETPC? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "GETPC %%r%u", r3);
                    break;
                
                case PAR1_OPCODE::SETFLAGS :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "SETFLAGS 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "SETFLAGS %%r%u", r3);
                    break;
                
                case PAR1_OPCODE::GETFLAGS :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "GETFLAGS? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "GETFLAGS %%r%u", r3);
                    break;
                
                case PAR1_OPCODE::SETY :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "SETY 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "SETY %%r%u", r3);
                    break;
                
                case PAR1_OPCODE::GETY :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "GETY? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "GETY %%r%u", r3);
                    break;
                
                case PAR1_OPCODE::SETIA :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "SETIA 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "SETIA %%r%u", r3);
                    break;
                
                case PAR1_OPCODE::GETIA :
                    if(literal)
                        snprintf(buf, BUF_SIZE, "GETIA? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "GETIA %%r%u", r3);
                    break;
                
                default:
                    if (literal)
                        snprintf(buf, BUF_SIZE, "J???? 0x%08x", r3);
                    else
                        snprintf(buf, BUF_SIZE, "J???? %%r%u", r3);
            }
        }

    } else if (IS_NOPAR(inst)) {
        // Instructions without parameters ************************************
        
        opcode = inst & 0x0FFFFFFF; // OpCode uses the 16 LSB
        if (IS_JUMP(inst)) { // Return instruction
            switch (opcode) {
                case OPCODE_JUMP::RFI :
                    snprintf(buf, BUF_SIZE, "RFI");
                    break;

                case OPCODE_JUMP::RET :
                    snprintf(buf, BUF_SIZE, "RET");
                    break;
                
                default:
                    snprintf(buf, BUF_SIZE, "J????");

            }
        } else if (IS_BRANCH(inst)) { // Branch if Flag X
            switch (opcode) {
                case OPCODE_BRANCH::IFOF :
                    snprintf(buf, BUF_SIZE, "IFOF");
                    break;
                
                case OPCODE_BRANCH::IFCF :
                    snprintf(buf, BUF_SIZE, "IFCF");
                    break;
                
                default:
                    snprintf(buf, BUF_SIZE, "IF????");
            }
        } else {    // Misc no parameter instruction
            switch (opcode) {
                case NOPAR_OPCODE::SLEEP :
                    snprintf(buf, BUF_SIZE, "SLEEP");
                    break;
                
                case NOPAR_OPCODE::NOP :
                    snprintf(buf, BUF_SIZE, "NOP");
                    break;
                
                default:
                    snprintf(buf, BUF_SIZE, "NOP????");
            }
        }
    }
	
    std::string out(buf);
	return out;

#undef BUF_SIZE
}

} // End of namespace cpu
} // End of namespace vm
