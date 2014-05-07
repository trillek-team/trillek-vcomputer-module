/**
 * Trillek Virtual Computer - DisTR3200.cpp
 * OnLine dis-assembler of TR3200 machine code
 */

#include "VComputer.hpp"
#include "TR3200/DisTR3200.hpp"
#include "TR3200/TR3200_opcodes.hpp"
#include "TR3200/TR3200_macros.hpp"
#include "VSFix.hpp"

#include <cstdio>

namespace vm {
namespace cpu {

std::string DisassemblyTR3200 (const VComputer& vc, dword_t pc) {
#define BUF_SIZE (32)
    char buf[BUF_SIZE] = {
        0
    };

    dword_t inst = vc.ReadDW(pc); // Fetch
    pc = pc +4;

    dword_t opcode, rd, rn, rs;
    rd = GRD(inst);
    rs = GRS(inst);
    // Here beging the Decoding
    bool literal = HAVE_LITERAL(inst);
    opcode = GET_OP_CODE(inst);

    if ( IS_P3(inst) ) {
        // 3 parameter instruction ********************************************
        if (literal) {
            rn = LIT15(inst);
            if ( IS_BIG_LITERAL_L15(rn) ) {
                // Next dword is literal value
                rn  = vc.ReadDW(pc);
                pc += 4;
            }
            else if ( RN_SIGN_BIT(inst) ) {
                // Negative Literal -> Extend sign
                rn |= 0xFFFF8000;
            }
        }
        else {
            rn = GRN(inst);
        }

        switch (opcode) {
        case P3_OPCODE::AND:
            if (literal) {
                snprintf(buf, BUF_SIZE, "AND %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "AND %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::OR:
            if (literal) {
                snprintf(buf, BUF_SIZE, "OR %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "OR %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::XOR:
            if (literal) {
                snprintf(buf, BUF_SIZE, "XOR %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "XOR %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::BITC:
            if (literal) {
                snprintf(buf, BUF_SIZE, "BITC %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "BITC %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::ADD:
            if (literal) {
                snprintf(buf, BUF_SIZE, "ADD %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "ADD %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::ADDC:
            if (literal) {
                snprintf(buf, BUF_SIZE, "ADDC %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "ADDC %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::SUB:
            if (literal) {
                snprintf(buf, BUF_SIZE, "SUB %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "SUB %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::SUBB:
            if (literal) {
                snprintf(buf, BUF_SIZE, "SUBB %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "SUBB %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::RSB:
            if (literal) {
                snprintf(buf, BUF_SIZE, "RSB %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "RSB %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::RSBB:
            if (literal) {
                snprintf(buf, BUF_SIZE, "RSBB %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "RSBB %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::LLS:
            if (literal) {
                snprintf(buf, BUF_SIZE, "LLS %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "LLS %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::RLS:
            if (literal) {
                snprintf(buf, BUF_SIZE, "RLS %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "RLS %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::ARS:
            if (literal) {
                snprintf(buf, BUF_SIZE, "ARS %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "ARS %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::ROTL:
            if (literal) {
                snprintf(buf, BUF_SIZE, "ROTL %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "ROTL %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::ROTR:
            if (literal) {
                snprintf(buf, BUF_SIZE, "ROTR %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "ROTR %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;


        case P3_OPCODE::MUL:
            if (literal) {
                snprintf(buf, BUF_SIZE, "MUL %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "MUL %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::SMUL:
            if (literal) {
                snprintf(buf, BUF_SIZE, "SMUL %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "SMUL %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::DIV:
            if (literal) {
                snprintf(buf, BUF_SIZE, "DIV %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "DIV %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;

        case P3_OPCODE::SDIV:
            if (literal) {
                snprintf(buf, BUF_SIZE, "SDIV %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "SDIV %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;


        case P3_OPCODE::LOAD:
            if (literal) {
                snprintf(buf, BUF_SIZE, "LOAD %%r%u, [%%r%u + 0x%08X]",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "LOAD %%r%u, [%%r%u + %%r%u]", rd, rs, rn);
            }
            break;

        case P3_OPCODE::LOADW:
            if (literal) {
                snprintf(buf, BUF_SIZE, "LOADW %%r%u, [%%r%u + 0x%08X]",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "LOADW %%r%u, [%%r%u + %%r%u]", rd, rs, rn);
            }
            break;

        case P3_OPCODE::LOADB:
            if (literal) {
                snprintf(buf, BUF_SIZE, "LOADB %%r%u, [%%r%u + 0x%08X]",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "LOADB %%r%u, [%%r%u + %%r%u]", rd, rs, rn);
            }
            break;

        case P3_OPCODE::STORE:
            if (literal) {
                snprintf(buf, BUF_SIZE, "STORE [%%r%u + 0x%08X], %%r%u", rs, rn, rd);
            }
            else {
                snprintf(buf, BUF_SIZE, "STORE [%%r%u + %%r%u], %%r%u", rs, rn, rd);
            }
            break;

        case P3_OPCODE::STOREW:
            if (literal) {
                snprintf(buf, BUF_SIZE, "STOREW [%%r%u + 0x%08X], %%r%u", rs, rn, rd);
            }
            else {
                snprintf(buf, BUF_SIZE, "STOREW [%%r%u + %%r%u], %%r%u", rs, rn, rd);
            }
            break;

        case P3_OPCODE::STOREB:
            if (literal) {
                snprintf(buf, BUF_SIZE, "STOREB [%%r%u + 0x%08X], %%r%u", rs, rn, rd);
            }
            else {
                snprintf(buf, BUF_SIZE, "STOREB [%%r%u + %%r%u], %%r%u", rs, rn, rd);
            }
            break;


        default:
            if (literal) {
                snprintf(buf, BUF_SIZE, "???? %%r%u, %%r%u, 0x%08X",  rd, rs, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "???? %%r%u, %%r%u, %%r%u", rd, rs, rn);
            }
            break;
        } // switch
    }
    else if ( IS_P2(inst) ) {
        // Fetch Rn operand
        if (literal) {
            rn = LIT19(inst);
            if ( IS_BIG_LITERAL_L19(rn) ) {
                // Next dword is literal value
                rn  = vc.ReadDW(pc);
                pc += 4;
            }
            else if ( RN_SIGN_BIT(inst) ) {
                // Negative Literal -> Extend sign
                rn |= 0xFFF80000;
            }
        }
        else {
            rn = GRS(inst);
        }

        switch (opcode) {
        case P2_OPCODE::MOV:
            if (literal) {
                snprintf(buf, BUF_SIZE, "MOV %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "MOV %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::SWP:
            if (literal) {
                snprintf(buf, BUF_SIZE, "SWP %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "SWP %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::SIGXB:
            if (literal) {
                snprintf(buf, BUF_SIZE, "SIGXB %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "SIGXB %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::SIGXW:
            if (literal) {
                snprintf(buf, BUF_SIZE, "SIGXW %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "SIGXW %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::NOT:
            if (literal) {
                snprintf(buf, BUF_SIZE, "NOT %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "NOT %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::LOAD2:
            if (literal) {
                snprintf(buf, BUF_SIZE, "LOAD %%r%u, [0x%08X]",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "LOAD %%r%u, [%%r%u]", rd,  rn);
            }
            break;

        case P2_OPCODE::LOADW2:
            if (literal) {
                snprintf(buf, BUF_SIZE, "LOADW %%r%u, [0x%08X]",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "LOADW %%r%u, [%%r%u]", rd, rn);
            }
            break;

        case P2_OPCODE::LOADB2:
            if (literal) {
                snprintf(buf, BUF_SIZE, "LOADB %%r%u, [0x%08X]",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "LOADB %%r%u, [%%r%u]", rd, rn);
            }
            break;

        case P2_OPCODE::STORE2:
            if (literal) {
                snprintf(buf, BUF_SIZE, "STORE [0x%08X], %%r%u",  rn, rd);
            }
            else {
                snprintf(buf, BUF_SIZE, "STORE [%%r%u], %%r%u", rn, rd);
            }
            break;

        case P2_OPCODE::STOREW2:
            if (literal) {
                snprintf(buf, BUF_SIZE, "STOREW [0x%08X], %%r%u", rn, rd);
            }
            else {
                snprintf(buf, BUF_SIZE, "STOREW [%%r%u], %%r%u", rn, rd);
            }
            break;

        case P2_OPCODE::STOREB2:
            if (literal) {
                snprintf(buf, BUF_SIZE, "STOREB [0x%08X], %%r%u", rn, rd);
            }
            else {
                snprintf(buf, BUF_SIZE, "STOREB [%%r%u], %%r%u", rn, rd);
            }
            break;


        case P2_OPCODE::IFEQ:
            if (literal) {
                snprintf(buf, BUF_SIZE, "IFEQ %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "IFEQ %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::IFNEQ:
            if (literal) {
                snprintf(buf, BUF_SIZE, "IFNEQ %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "IFNEQ %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::IFL:
            if (literal) {
                snprintf(buf, BUF_SIZE, "IFL %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "IFL %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::IFSL:
            if (literal) {
                snprintf(buf, BUF_SIZE, "IFSL %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "IFSL %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::IFLE:
            if (literal) {
                snprintf(buf, BUF_SIZE, "IFLE %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "IFLE %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::IFSLE:
            if (literal) {
                snprintf(buf, BUF_SIZE, "IFSLE %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "IFSLE %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::IFBITS:
            if (literal) {
                snprintf(buf, BUF_SIZE, "IFBITS %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "IFBITS %%r%u, %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::IFCLEAR:
            if (literal) {
                snprintf(buf, BUF_SIZE, "IFCLEAR %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "IFCLEAR %%r%u, %%r%u", rd, rn);
            }
            break;


        case P2_OPCODE::JMP2:
            if (literal) {
                snprintf(buf, BUF_SIZE, "JMP %%r%u + 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "JMP %%r%u + %%r%u", rd, rn);
            }
            break;

        case P2_OPCODE::CALL2:
            if (literal) {
                snprintf(buf, BUF_SIZE, "CALL %%r%u + 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "CALL %%r%u + %%r%u", rd, rn);
            }
            break;


        default:
            if (literal) {
                snprintf(buf, BUF_SIZE, "???? %%r%u, 0x%08X",  rd, rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "???? %%r%u, %%r%u", rd, rn);
            }
            break;
        } // switch
    }
    else if ( IS_P1(inst) ) {
        // 1 parameter instrucction *******************************************
        // Fetch Rn operand
        if (literal) {
            rn = LIT23(inst);
            if ( IS_BIG_LITERAL_L23(rn) ) {
                // Next dword is literal value
                rn  = vc.ReadDW(pc);
                pc += 4;
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
            if (literal) {
                snprintf(buf, BUF_SIZE, "XCHGB? 0x%08X",  rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "XCHGB %%r%u", rn);
            }
            break;

        case P1_OPCODE::XCHGW:
            if (literal) {
                snprintf(buf, BUF_SIZE, "XCHGW? 0x%08X",  rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "XCHGW %%r%u", rn);
            }
            break;

        case P1_OPCODE::GETPC:
            if (literal) {
                snprintf(buf, BUF_SIZE, "GETPC? 0x%08X",  rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "GETPC %%r%u", rn);
            }
            break;


        case P1_OPCODE::POP:
            if (literal) {
                snprintf(buf, BUF_SIZE, "POP? 0x%08X",  rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "POP %%r%u", rn);
            }
            break;

        case P1_OPCODE::PUSH:
            if (literal) {
                snprintf(buf, BUF_SIZE, "PUSH 0x%08X",  rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "PUSH %%r%u", rn);
            }
            break;


        case P1_OPCODE::JMP:
            if (literal) {
                snprintf(buf, BUF_SIZE, "JMP 0x%08X",  rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "JMP %%r%u", rn);
            }
            break;

        case P1_OPCODE::CALL:
            if (literal) {
                snprintf(buf, BUF_SIZE, "CALL 0x%08X",  rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "CALL %%r%u", rn);
            }
            break;

        case P1_OPCODE::RJMP:
            if (literal) {
                snprintf(buf, BUF_SIZE, "JMP %%pc +0x%08X",  rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "JMP %%pc +%%r%u", rn);
            }
            break;

        case P1_OPCODE::RCALL:
            if (literal) {
                snprintf(buf, BUF_SIZE, "CALL %%pc +0x%08X",  rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "CALL %%pc +%%r%u", rn);
            }
            break;

        case P1_OPCODE::INT:
            if (literal) {
                snprintf(buf, BUF_SIZE, "INT %08Xh", rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "INT %%r%u", rn);
            }
            break;


        default:
            if (literal) {
                snprintf(buf, BUF_SIZE, "???? 0x%08X",  rn);
            }
            else {
                snprintf(buf, BUF_SIZE, "???? %%r%u", rn);
            }
            break;
        } // switch
    }
    else {
        // 0 parameter instrucction *******************************************

        switch (opcode) {
        case NP_OPCODE::SLEEP:
            snprintf(buf, BUF_SIZE, "SLEEP");
            break;

        case NP_OPCODE::RET:
            snprintf(buf, BUF_SIZE, "RET");
            break;

        case NP_OPCODE::RFI:
            snprintf(buf, BUF_SIZE, "RFI");
            break;

        default:
            snprintf(buf, BUF_SIZE, "????");
        } // switch
    }


    std::string out(buf);
    return out;

    #undef BUF_SIZE
} // DisassemblyTR3200
} // End of namespace cpu
} // End of namespace vm
