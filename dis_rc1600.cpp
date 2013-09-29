#include "dis_rc1600.hpp"
#include "rc1600_opcodes.hpp"

#include <cstdio>

namespace CPU {

std::string disassembly(const byte_t* data) {
	char buf[32] = {0};
    
	dword_t epc = 0;
    word_t inst = (data[epc+1] << 8) | data[epc]; // Big Endian
	epc +=2;
    word_t opcode, reg1, reg2, reg3;

    reg3 = inst & 0xF;
    reg2 = (inst >> 4) & 0xF;
    reg1 = (inst >> 8) & 0xF;

    if (IS_PAR3(inst)) {
        // 3 parameter instrucction
        opcode = (inst >> 12) & 7;
		switch (opcode) {
			case PAR3_OPCODE::LOAD_LIT :
				if (reg2 == 0xF) {
					reg2 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "LOAD [r%u + 0x%04x], r%u", reg1, reg2, reg3);
				break;

			case PAR3_OPCODE::LOAD :
				snprintf(buf, 32, "LOAD [r%u + r%u], r%u", reg1, reg2, reg3);
				break;

			case PAR3_OPCODE::LOADB_LIT :
				if (reg2 == 0xF) {
					reg2 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "LOAD.B [r%u + 0x%04x], r%u", reg1, reg2, 
						 reg3);
				break;

			case PAR3_OPCODE::LOADB :
				snprintf(buf, 32, "LOAD.B [r%u + r%u], r%u", reg1, reg2, reg3);
				break;

            // Write ----------------------------------------------------------
            case PAR3_OPCODE::STORE_LIT :
				if (reg2 == 0xF) {
					reg2 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "STORE [r%u + 0x%04x], r%u", reg1, reg2, 
						 reg3);
				break;

            case PAR3_OPCODE::STORE :
				snprintf(buf, 32, "STORE [r%u + r%u], r%u", reg1, reg2, reg3);
				break;

            case PAR3_OPCODE::STOREB_LIT :
				if (reg2 == 0xF) {
					reg2 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "STORE.B [r%u + 0x%04x], r%u", reg1, reg2, 
						 reg3);
				break;

            case PAR3_OPCODE::STOREB :
				snprintf(buf, 32, "STORE.B [r%u + r%u], r%u", reg1, reg2, reg3);
				break;

			default:
				snprintf(buf, 32, "??? r%u , r%u, r%u", reg1, reg2, reg3);
		}

    } else if (IS_PAR2(inst)) {
		// 2 parameter instrucction
        opcode = (inst >> 8) & 0x3F;
        switch (opcode) {
			case PAR2_OPCODE::ADD :
				snprintf(buf, 32, "ADD r%u, r%u", reg3, reg2);
				break;

			case PAR2_OPCODE::ADD_LIT :
				if (reg2 == 0xF) {
					reg2 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "ADD r%u, 0x%04x", reg3, reg2);
				break;

			case PAR2_OPCODE::SUB :
				snprintf(buf, 32, "SUB r%u, r%u", reg3, reg2);
				break;

			case PAR2_OPCODE::SUB_LIT :
				if (reg2 == 0xF) {
					reg2 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "SUB r%u, 0x%04x", reg3, reg2);
				break;

			case PAR2_OPCODE::ADDC :
				snprintf(buf, 32, "SUB r%u, r%u", reg3, reg2);
				break;

			case PAR2_OPCODE::SUBC :
				snprintf(buf, 32, "SUBB r%u, r%u", reg3, reg2);
				break;

			case PAR2_OPCODE::SUBC_LIT :
				if (reg2 == 0xF) {
					reg2 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "SUBB r%u, 0x%04x", reg3, reg2);
				break;

		
			case PAR2_OPCODE::AND :
				snprintf(buf, 32, "AND r%u, r%u", reg3, reg2);
				break;

			case PAR2_OPCODE::OR :
				snprintf(buf, 32, "OR r%u, r%u", reg3, reg2);
				break;

			case PAR2_OPCODE::XOR :
				snprintf(buf, 32, "XOR r%u, r%u", reg3, reg2);
				break;
		
		
		}
    
	} else if (IS_PAR1(inst)) {
        // 1 parameter instrucction
        opcode = (inst >> 4) & 0x1FF;
        switch (opcode) {
			case PAR1_OPCODE::NOT :
				snprintf(buf, 32, "NOT r%u", reg3);
				break;

			case PAR1_OPCODE::NEG :
				snprintf(buf, 32, "NEG r%u", reg3);
				break;

			case PAR1_OPCODE::XCHG :
				snprintf(buf, 32, "XCHG r%u", reg3);
				break;

			case PAR1_OPCODE::SXTBD :
				snprintf(buf, 32, "SXTBD r%u", reg3);
				break;


			case PAR1_OPCODE::PUSH :
				snprintf(buf, 32, "PUSH r%u", reg3);
				break;
			
			case PAR1_OPCODE::POP :
				snprintf(buf, 32, "POP r%u", reg3);
				break;


			case PAR1_OPCODE::GETPC :
				snprintf(buf, 32, "GETPC r%u", reg3);
				break;

			case PAR1_OPCODE::SETFLAGS :
				snprintf(buf, 32, "SETFLAGS r%u", reg3);
				break;

			case PAR1_OPCODE::GETFLAGS :
				snprintf(buf, 32, "GETFLAGS r%u", reg3);
				break;


			case PAR1_OPCODE::SETY :
				snprintf(buf, 32, "SETY r%u", reg3);
				break;

			case PAR1_OPCODE::SETY_LIT :
				if (reg3 == 0xF) {
					reg3 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "SETY 0x%04x", reg3);
				break;
			
			case PAR1_OPCODE::GETY :
				snprintf(buf, 32, "GETY r%u", reg3);
				break;


			case PAR1_OPCODE::INT :
				snprintf(buf, 32, "INT r%u", reg3);
				break;

			case PAR1_OPCODE::INT_LIT :
				if (reg3 == 0xF) {
					reg3 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "INT %xh", reg3);
				break;


			case PAR1_OPCODE::SETIA :
				snprintf(buf, 32, "SETIA r%u", reg3);
				break;

			case PAR1_OPCODE::SETIA_LIT :
				if (reg3 == 0xF) {
					reg3 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "SETIA 0x%04x", reg3);
				break;
			
			case PAR1_OPCODE::GETIA :
				snprintf(buf, 32, "GETY r%u", reg3);
				break;

			case PAR1_OPCODE::SETCS :
				snprintf(buf, 32, "SETCS r%u", reg3);
				break;

			case PAR1_OPCODE::SETCS_LIT :
				if (reg3 == 0xF) {
					reg3 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "SETCS 0x%04x", reg3);
				break;
			
			case PAR1_OPCODE::GETCS :
				snprintf(buf, 32, "GETCS r%u", reg3);
				break;

			case PAR1_OPCODE::SETDS :
				snprintf(buf, 32, "SETDS r%u", reg3);
				break;

			case PAR1_OPCODE::SETDS_LIT :
				if (reg3 == 0xF) {
					reg3 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "SETDS 0x%04x", reg3);
				break;
			
			case PAR1_OPCODE::GETDS :
				snprintf(buf, 32, "GETDS r%u", reg3);
				break;

			case PAR1_OPCODE::SETSS :
				snprintf(buf, 32, "SETSS r%u", reg3);
				break;

			case PAR1_OPCODE::SETSS_LIT :
				if (reg3 == 0xF) {
					reg3 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "SETSS 0x%04x", reg3);
				break;
			
			case PAR1_OPCODE::GETSS :
				snprintf(buf, 32, "GETSS r%u", reg3);
				break;

			case PAR1_OPCODE::SETIS :
				snprintf(buf, 32, "SETIS r%u", reg3);
				break;

			case PAR1_OPCODE::SETIS_LIT :
				if (reg3 == 0xF) {
					reg3 = (data[epc+1] << 8) | data[epc];
					epc +=2;
				}
				snprintf(buf, 32, "SETIS 0x%04x", reg3);
				break;
			
			case PAR1_OPCODE::GETIS :
				snprintf(buf, 32, "GETIS r%u", reg3);
				break;

			default:
				snprintf(buf, 32, "??? r%u", reg3);

		}
    
	} else if (IS_PAR1L(inst)) {
        // 1 parameter that is a long literal
        opcode = (inst >> 9) & 0x7;
        word_t long_lit = inst & 0x1FF;
        switch (opcode) {
            case PAR1L_OPCODE::JMP_REL :
				snprintf(buf, 32, "JUMP [%d]", (int)long_lit);
				break;
            
			case PAR1L_OPCODE::CALL_REL : 
				snprintf(buf, 32, "CALL [%d]", (int)long_lit);
				break;

			default:
				snprintf(buf, 32, "??? %d", (int)long_lit);
		}
		
    } else if (IS_NOPAR(inst)) {
        // Instrucctions without parameters
        opcode = inst & 0xFFF;
		switch (opcode) {
            case NOPAR_OPCODE::NOP :
				snprintf(buf, 32, "NOP");
				break;

            case NOPAR_OPCODE::SLEEP :
				snprintf(buf, 32, "SLEEP");
				break;

            case NOPAR_OPCODE::RET :
				snprintf(buf, 32, "RET");
				break;

            case NOPAR_OPCODE::LRET :
				snprintf(buf, 32, "LRET");
				break;
            
			case NOPAR_OPCODE::RFI :
				snprintf(buf, 32, "RFI");
				break;

            case NOPAR_OPCODE::BOVF :
				snprintf(buf, 32, "BOVF");
				break;

            case NOPAR_OPCODE::BOCF :
				snprintf(buf, 32, "BOCF");
				break;

			default:
				snprintf(buf, 32, "???");
		}

	} else {
		// Unknow
	}

	std::string out(buf);
	return out;
}

} // End of namespace CPU
