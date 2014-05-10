/**
 * Trillek Virtual Computer - DisDCPU16N.cpp
 * OnLine dis-assembly of DCPU-16N machine code
 */

#include "dcpu16n/dis_dcpu16n.hpp"
#include "vs_fix.hpp"

#include <cstdio>

namespace trillek {
namespace computer {

static const char *DCPU16N_OPCN[] = {
    "---", "SET", "ADD", "SUB", "MUL", "MLI", "DIV", "DVI",
    "MOD", "MDI", "AND", "BOR", "XOR", "SHR", "ASR", "SHL",
    "IFB", "IFC", "IFE", "IFN", "IFG", "IFA", "IFL", "IFU",
    "???", "???", "ADX", "SBX", "HWW", "HWR", "STI", "STD"
};

static const char *DCPU16N_OPCS[] = {
    "---", "JSR", "BSR", "???", "???", "NEG", "???", "HCF",
    "INT", "IAG", "IAS", "RFI", "IAQ", "???", "???", "???",
    "MMW", "MMR", "???", "???", "SXB", "SWP", "???", "???",
    "???", "???", "???", "???", "???", "???", "???", "???"
};

static const char *DCPU16N_OPCI[] = {
    "HLT", "SLP", "???", "???", "BYT", "???", "???", "???",
    "???", "???", "???", "???", "???", "???", "???", "???",
    "SKP", "???", "???", "???", "???", "???", "???", "???",
    "???", "???", "???", "???", "???", "???", "???", "???"
};

struct DCPU16N_DisInfo {
    const char *pre;
    const char *suf;
    bool fetch;
};

const DCPU16N_DisInfo DCPU16N_Operand[] = {
    { "A", "", false }, { "B", "", false }, { "C", "", false },
    { "X", "", false }, { "Y", "", false }, { "Z", "", false },
    { "I", "", false }, { "J", "", false },
    { "[A]", "", false }, { "[B]", "", false }, { "[C]", "", false },
    { "[X]", "", false }, { "[Y]", "", false }, { "[Z]", "", false },
    { "[I]", "", false }, { "[J]", "", false },
    { "[A+", "]", true }, { "[B+", "]", true }, { "[C+", "]", true },
    { "[X+", "]", true }, { "[Y+", "]", true }, { "[Z+", "]", true },
    { "[I+", "]", true }, { "[J+", "]", true },
    { "[SP++]", "[--SP]", false }, { "[SP]", "", false },
    { "[SP+", "]", true }, { "SP", "", false },
    { "PC", "", false }, { "EX", "", false },
    { "[", "]", true }, { "", "", true }
};

std::string DisassemblyDCPU16N(const VComputer& vc, DWord pc) {
    Word opc = ( ((Word)vc.ReadB(pc + 1)) << 8 )
                 |  (Word)vc.ReadB(pc);
    Word opta, optb;
    Word opa, opb;
#define BUF_SIZE (32)
    char buf[BUF_SIZE] = { 0, };
    std::string obja, objb;
    if((opc & 0x001f) != 0 || (opc & 0x03e0) != 0) {
        opta = opc >> 10;
        if(opta & 0x20) {
            snprintf(buf, BUF_SIZE, "%d", ((int)opta) - 33);
            obja.assign(buf);
        }
        else if(opta == 0x18) {
            obja.assign(DCPU16N_Operand[0x18].suf);
        }
        else {
            obja.assign(DCPU16N_Operand[opta & 0x1f].pre);
            if(DCPU16N_Operand[opta & 0x1f].fetch) {
                pc += 2;
                opa = ( ((Word)vc.ReadB(pc + 1)) << 8 )
                      |  (Word)vc.ReadB(pc);
                snprintf(buf, BUF_SIZE, "0x%04X", opa);
                obja.append(buf);
            }
            obja.append(DCPU16N_Operand[opta & 0x1f].suf);
        }
    }
    if((opc & 0x001f) != 0) {
        optb = (opc >> 5) & 0x01f;
        if(optb == 0x18) {
            objb.assign(DCPU16N_Operand[0x18].pre);
        }
        else {
            objb.assign(DCPU16N_Operand[optb & 0x1f].pre);
            if(DCPU16N_Operand[optb & 0x1f].fetch) {
                pc += 2;
                opb = ( ((Word)vc.ReadB(pc + 1)) << 8 )
                      |  (Word)vc.ReadB(pc);
                snprintf(buf, BUF_SIZE, "0x%04X", opb);
                objb.append(buf);
            }
            objb.append(DCPU16N_Operand[optb & 0x1f].suf);
        }
    }
    std::string opname;
    if((opc & 0x001f) != 0) {
        opname.assign(DCPU16N_OPCN[opc & 0x1f]);
        opname.append(" ");
        opname.append(objb);
        opname.append(",");
        opname.append(obja);
    }
    else if((opc & 0x03e0) != 0) {
        opname.assign(DCPU16N_OPCS[(opc >> 5) & 0x1f]);
        opname.append(" ");
        opname.append(obja);
    }
    else {
        opname.assign(DCPU16N_OPCI[(opc >> 10) & 0x1f]);
        if(opc & 0x8000) {
            opname.append(" 1");
        }
        else {
            opname.append(" 0");
        }
    }
    return opname;
}

} // namespace computer
} // namespace trillek

