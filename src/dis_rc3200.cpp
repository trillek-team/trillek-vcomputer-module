#include "dis_rc3200.hpp"
#include "rc3200_opcodes.hpp"

#include <cstdio>

namespace vm {
namespace cpu {

std::string Disassembly (const ram::Mem& ram, dword_t pc) {
#define BUF_SIZE (32)
	char buf[BUF_SIZE] = {0};
    
    dword_t inst = ram.RB(pc++); // Fetch
    inst |= (ram.RB(pc++) << 8);
    inst |= (ram.RB(pc++) << 16);
    inst |= (ram.RB(pc++) << 24); // Little Endian

    dword_t opcode, rd, rn, rs;
    rd = GRD(inst);
    rs = GRS(inst);
    // Here beging the Decoding
    bool literal = HAVE_LITERAL(inst);


    if (IS_PAR3(inst)) {
        // 3 parameter instruction ******************************************** 
        opcode = (inst >> 24) & 0x3F;
       
        if (literal) {
            rn = LIT13(inst);
            if (IS_BIG_LITERAL_L13(rn)) { // Next dword is literal value 
                rn = ram.RD(pc);
                pc +=4;
            } else if (O13_SIGN_BIT(rn)) { // Negative Literal -> Extend sign
                rn |= 0xFFFFF000;
            }
        } else {
            rn = GRN(inst);
        }
            
        switch (opcode) {
            case P3_OPCODE::AND :
                if (literal)
                    snprintf(buf, BUF_SIZE, "AND %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "AND %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::OR :
                if (literal)
                    snprintf(buf, BUF_SIZE, "OR %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "OR %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::XOR :
                if (literal)
                    snprintf(buf, BUF_SIZE, "XOR %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "XOR %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::BITC :
                if (literal)
                    snprintf(buf, BUF_SIZE, "BITC %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "BITC %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::ADD :
                if (literal)
                    snprintf(buf, BUF_SIZE, "ADD %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "ADD %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::ADDC :
                if (literal)
                    snprintf(buf, BUF_SIZE, "ADDC %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "ADDC %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::SUB :
                if (literal)
                    snprintf(buf, BUF_SIZE, "SUB %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "SUB %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::SUBB :
                if (literal)
                    snprintf(buf, BUF_SIZE, "SUBB %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "SUBB %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::RSB :
                if (literal)
                    snprintf(buf, BUF_SIZE, "RSB %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "RSB %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::RSBB :
                if (literal)
                    snprintf(buf, BUF_SIZE, "RSBB %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "RSBB %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::LLS :
                if (literal)
                    snprintf(buf, BUF_SIZE, "LLS %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "LLS %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::RLS :
                if (literal)
                    snprintf(buf, BUF_SIZE, "RLS %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "RLS %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::ARS :
                if (literal)
                    snprintf(buf, BUF_SIZE, "ARS %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "ARS %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::ROTL :
                if (literal)
                    snprintf(buf, BUF_SIZE, "ROTL %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "ROTL %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::ROTR :
                if (literal)
                    snprintf(buf, BUF_SIZE, "ROTR %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "ROTR %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;


            case P3_OPCODE::MUL :
                if (literal)
                    snprintf(buf, BUF_SIZE, "MUL %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "MUL %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::SMUL :
                if (literal)
                    snprintf(buf, BUF_SIZE, "SMUL %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "SMUL %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::DIV :
                if (literal)
                    snprintf(buf, BUF_SIZE, "DIV %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "DIV %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;

            case P3_OPCODE::SDIV :
                if (literal)
                    snprintf(buf, BUF_SIZE, "SDIV %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "SDIV %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;


            case P3_OPCODE::LOAD :
                if (literal)
                    snprintf(buf, BUF_SIZE, "LOAD %%r%02u, [%%r%02u + 0x%08x]",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "LOAD %%r%02u, [%%r%02u + %%r%02u]", rd, rs, rn);
                break;

            case P3_OPCODE::LOADW :
                if (literal)
                    snprintf(buf, BUF_SIZE, "LOAD.W %%r%02u, [%%r%02u + 0x%08x]",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "LOAD.W %%r%02u, [%%r%02u + %%r%02u]", rd, rs, rn);
                break;

            case P3_OPCODE::LOADB :
                if (literal)
                    snprintf(buf, BUF_SIZE, "LOAD.B %%r%02u, [%%r%02u + 0x%08x]",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "LOAD.B %%r%02u, [%%r%02u + %%r%02u]", rd, rs, rn);
                break;

            case P3_OPCODE::STORE :
                if (literal)
                    snprintf(buf, BUF_SIZE, "STORE [%%r%02u + 0x%08x], %%r%02u", rs, rn, rd);
                else                                                           
                    snprintf(buf, BUF_SIZE, "STORE [%%r%02u + %%r%02u], %%r%02u",rs, rn, rd);
                break;

            case P3_OPCODE::STOREW :
                if (literal)
                    snprintf(buf, BUF_SIZE, "STOREW [%%r%02u + 0x%08x], %%r%02u", rs, rn, rd);
                else                                                            
                    snprintf(buf, BUF_SIZE, "STOREW [%%r%02u + %%r%02u], %%r%02u", rs, rn, rd);
                break;                                                          
                                                                                
            case P3_OPCODE::STOREB :                                            
                if (literal)                                                    
                    snprintf(buf, BUF_SIZE, "STOREB [%%r%02u + 0x%08x], %%r%02u", rs, rn, rd);
                else                                                            
                    snprintf(buf, BUF_SIZE, "STOREB [%%r%02u + %%r%02u], %%r%02u", rs, rn, rd);
                break;


            default:
                if (literal)
                    snprintf(buf, BUF_SIZE, "???? %%r%02u, %%r%02u, 0x%08x",  rd, rs, rn);
                else
                    snprintf(buf, BUF_SIZE, "???? %%r%02u, %%r%02u, %%r%02u", rd, rs, rn);
                break;
        }
    } else if (IS_PAR2(inst)) {
        opcode = (inst >> 24) & 0x7F;
        
        // Fetch Rn operand
        if (literal) {
            rn = LIT18(inst);
            if (IS_BIG_LITERAL_L18(rn)) { // Next dword is literal value 
                rn = ram.RD(pc);
                pc +=4;
            } else if (O18_SIGN_BIT(rn)) { // Negative Literal -> Extend sign
                rn |= 0xFFFC0000;
            }
        } else {
            rn = GRS(inst);
        }

        switch (opcode) {
            case P2_OPCODE::MOV :
                if (literal)
                    snprintf(buf, BUF_SIZE, "MOV %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "MOV %%r%02u, %%r%02u", rd, rn);
                break;
            
            case P2_OPCODE::SWP :
                if (literal)
                    snprintf(buf, BUF_SIZE, "SWP %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "SWP %%r%02u, %%r%02u", rd, rn);
                break;
           
            case P2_OPCODE::SIGXB :
                if (literal)
                    snprintf(buf, BUF_SIZE, "SIGXB %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "SIGXB %%r%02u, %%r%02u", rd, rn);
                break;
           
            case P2_OPCODE::SIGXW :
                if (literal)
                    snprintf(buf, BUF_SIZE, "SIGXW %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "SIGXW %%r%02u, %%r%02u", rd, rn);
                break;
           
            case P2_OPCODE::NOT :
                if (literal)
                    snprintf(buf, BUF_SIZE, "NOT %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "NOT %%r%02u, %%r%02u", rd, rn);
                break;
           

            case P2_OPCODE::LOAD2 :
                if (literal)
                    snprintf(buf, BUF_SIZE, "LOAD %%r%02u, [0x%08x]",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "LOAD %%r%02u, [%%r%02u]", rd,  rn);
                break;

            case P2_OPCODE::LOADW2 :
                if (literal)
                    snprintf(buf, BUF_SIZE, "LOAD.W %%r%02u, [0x%08x]",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "LOAD.W %%r%02u, [%%r%02u]", rd, rn);
                break;

            case P2_OPCODE::LOADB2 :
                if (literal)
                    snprintf(buf, BUF_SIZE, "LOAD.B %%r%02u, [0x%08x]",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "LOAD.B %%r%02u, [%%r%02u]", rd, rn);
                break;

            case P2_OPCODE::STORE2 :
                if (literal)
                    snprintf(buf, BUF_SIZE, "STORE [0x%08x], %%r%02u",  rn, rd);
                else                                                 
                    snprintf(buf, BUF_SIZE, "STORE [%%r%02u], %%r%02u", rn, rd);
                break;

            case P2_OPCODE::STOREW2 :
                if (literal)
                    snprintf(buf, BUF_SIZE, "STOREW [0x%08x], %%r%02u", rn, rd);
                else                                                  
                    snprintf(buf, BUF_SIZE, "STOREW [%%r%02u], %%r%02u", rn, rd);
                break;                                                
                                                                      
            case P2_OPCODE::STOREB2 :                                  
                if (literal)                                          
                    snprintf(buf, BUF_SIZE, "STOREB [0x%08x], %%r%02u", rn, rd);
                else                                                  
                    snprintf(buf, BUF_SIZE, "STOREB [%%r%02u], %%r%02u", rn, rd);
                break;


            case P2_OPCODE::IFEQ :
                if (literal)                                          
                    snprintf(buf, BUF_SIZE, "IFEQ %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "IFEQ %%r%02u, %%r%02u", rd, rn);
                break;

            case P2_OPCODE::IFNEQ :
                if (literal)                                          
                    snprintf(buf, BUF_SIZE, "IFNEQ %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "IFNEQ %%r%02u, %%r%02u", rd, rn);
                break;

            case P2_OPCODE::IFL :
                if (literal)                                          
                    snprintf(buf, BUF_SIZE, "IFL %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "IFL %%r%02u, %%r%02u", rd, rn);
                break;

            case P2_OPCODE::IFSL :
                if (literal)                                          
                    snprintf(buf, BUF_SIZE, "IFSL %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "IFSL %%r%02u, %%r%02u", rd, rn);
                break;

            case P2_OPCODE::IFLE :
                if (literal)                                          
                    snprintf(buf, BUF_SIZE, "IFLE %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "IFLE %%r%02u, %%r%02u", rd, rn);
                break;

            case P2_OPCODE::IFSLE :
                if (literal)                                          
                    snprintf(buf, BUF_SIZE, "IFSLE %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "IFSLE %%r%02u, %%r%02u", rd, rn);
                break;

            case P2_OPCODE::IFBITS :
                if (literal)                                          
                    snprintf(buf, BUF_SIZE, "IFBITS %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "IFBITS %%r%02u, %%r%02u", rd, rn);
                break;

            case P2_OPCODE::IFCLEAR :
                if (literal)                                          
                    snprintf(buf, BUF_SIZE, "IFCLEAR %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "IFCLEAR %%r%02u, %%r%02u", rd, rn);
                break;


            case P2_OPCODE::JMP2 :
                if (literal)                                          
                    snprintf(buf, BUF_SIZE, "JMP %%r%02u + 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "JMP %%r%02u + %%r%02u", rd, rn);
                break;

            case P2_OPCODE::CALL2 :
                if (literal)                                          
                    snprintf(buf, BUF_SIZE, "CALL %%r%02u + 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "CALL %%r%02u + %%r%02u", rd, rn);
                break;


            default:
                if (literal)
                    snprintf(buf, BUF_SIZE, "???? %%r%02u, 0x%08x",  rd, rn);
                else
                    snprintf(buf, BUF_SIZE, "???? %%r%02u, %%r%02u", rd, rn);
                break;
        }

    } else if (IS_PAR1(inst)) {
        // 1 parameter instrucction *******************************************
        
        opcode = (inst >> 24) & 0x1F;
        
        // Fetch Rn operand
        if (literal) {
            rn = LIT22(inst);
            if (IS_BIG_LITERAL_L22(rn)) { // Next dword is literal value 
                rn = ram.RD(pc);
                pc +=4;
            } else if (O22_SIGN_BIT(rn)) { // Negative Literal -> Extend sign
                rn |= 0xFF800000;
            }
        } else {
            rn = GRD(inst);
        }

        switch (opcode) {
            case P1_OPCODE::XCHGB :
                if (literal)
                    snprintf(buf, BUF_SIZE, "XCHGB? 0x%08x",  rn);
                else
                    snprintf(buf, BUF_SIZE, "XCHGB %%r%02u", rn);
                break;

            case P1_OPCODE::XCHGW :
                if (literal)
                    snprintf(buf, BUF_SIZE, "XCHGW? 0x%08x",  rn);
                else
                    snprintf(buf, BUF_SIZE, "XCHGW %%r%02u", rn);
                break;

            case P1_OPCODE::GETPC :
                if (literal)
                    snprintf(buf, BUF_SIZE, "GETPC? 0x%08x",  rn);
                else
                    snprintf(buf, BUF_SIZE, "GETPC %%r%02u", rn);
                break;


            case P1_OPCODE::POP :
                if (literal)
                    snprintf(buf, BUF_SIZE, "POP? 0x%08x",  rn);
                else
                    snprintf(buf, BUF_SIZE, "POP %%r%02u", rn);
                break;

            case P1_OPCODE::PUSH :
                if (literal)
                    snprintf(buf, BUF_SIZE, "PUSH 0x%08x",  rn);
                else
                    snprintf(buf, BUF_SIZE, "PUSH %%r%02u", rn);
                break;


            case P1_OPCODE::JMP :
                if (literal)
                    snprintf(buf, BUF_SIZE, "JMP 0x%08x",  rn);
                else
                    snprintf(buf, BUF_SIZE, "JMP %%r%02u", rn);
                break;
            
            case P1_OPCODE::CALL :
                if (literal)
                    snprintf(buf, BUF_SIZE, "CALL 0x%08x",  rn);
                else
                    snprintf(buf, BUF_SIZE, "CALL %%r%02u", rn);
                break;

            case P1_OPCODE::RJMP :
                if (literal)
                    snprintf(buf, BUF_SIZE, "JMP %%pc +0x%08x",  rn);
                else
                    snprintf(buf, BUF_SIZE, "JMP %%pc +%%r%02u", rn);
                break;
            
            case P1_OPCODE::RCALL :
                if (literal)
                    snprintf(buf, BUF_SIZE, "CALL %%pc +0x%08x",  rn);
                else
                    snprintf(buf, BUF_SIZE, "CALL %%pc +%%r%02u", rn);
                break;

            case P1_OPCODE::INT :
                if (literal)
                    snprintf(buf, BUF_SIZE, "INT %08xh", rn);
                else
                    snprintf(buf, BUF_SIZE, "INT %%r%02u", rn);
                break;



            default:
                if (literal)
                    snprintf(buf, BUF_SIZE, "???? 0x%08x",  rn);
                else
                    snprintf(buf, BUF_SIZE, "???? %%r%02u", rn);
                break;
        }
    } else {
        
        opcode = inst & 0x0FFFFFFF; // OpCode uses the 16 LSB
        switch (opcode) {
            case NP_OPCODE::SLEEP :
                snprintf(buf, BUF_SIZE, "SLEEP");
                break;

            case NP_OPCODE::RET :
                snprintf(buf, BUF_SIZE, "RET");
                break;

            case NP_OPCODE::RFI :
                snprintf(buf, BUF_SIZE, "RFI");
                break;


            default:
                snprintf(buf, BUF_SIZE, "????");
        }
    }

	
    std::string out(buf);
	return out;

#undef BUF_SIZE
}

} // End of namespace cpu
} // End of namespace vm
