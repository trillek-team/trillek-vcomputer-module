#include "cpu.hpp"

#include <iostream>
#include <ios>
#include <iomanip> 
#include <cstdio>
#include <algorithm>

size_t prg_size = 45*2;
CPU::word_t prg[] = {
    0x6210,  // 000h SET r0, 1
    0x6211,  // 002h SET r1, 1
    0x6222,  // 004h SET r2, 2 ..
    0x6233,
    0x6244,
    0x6255,
    0x6266,
    0x6277,
    0x6288,
    0x6299,
    0x62AA,
    0x62BB,
    0x62CC,
    0x62DD,
    0x62EE,  // 01Ch SET BP, 14
    0x62FF,  // 01Eh SET SP, 15
    0x000F,  // 020h literal
    0x620F,  // 022h SET SP, 0
    0x2001,  // 024h NOT r1
    0x2012,  // 026h NEG r2
    0x2023,  // 028h XCHG r3
    0x62F4,  // 02Ah SET 0x00FF, r4
    0x00FF,  // 02Ch literal
    0x2034,  // 02Eh SXTBD r4       (r4  == 0xFFFF)
    0x4025,  // 030h ADD r5, r2     (r5  == 0x0003) CF=1
    0x4111,  // 032h ADD r1, 1      (r1  == 0xFFFF)
    0x4225,  // 034h SUB r5, r2     (r5  == 0x0005) CF=1
    0x4316,  // 036h SUB r6, 1      (r6  == 0x0005)
    0x62FA,  // 038h SET r10, 0x7FFF
    0x7FFF,  // 03Ah literal
    0x411A,  // 03Ch ADD r10, 1     (r10 == 0x8000) OF=1
    0x6061,  // 03Eh SWP r1 ,r6
    0x6116,  // 040h CPY r6, r1     (r1 == r6 == 0x0005)
    0x8001,  // 042h LOAD [r0], r1          (r1 == 0x6210)
    0x9061,  // 044h LOAD [r0 + r6], r1     (r1 == 0x3362)
    0xA012,  // 046h LOAD.B [r0+1], r2      (r2 == 0xFF62)
    0xB082,  // 048h LOAD.B [r0+r8], r2     (r2 == 0xFF44)
    0x6456,  // 04Ah BEQ r6 == 5  (true)
    0x62F6,  // 04Ch SET r6, 0xCAFE (not should happen)
    0xCAFE,  // 04Eh literal
    0x6201,  // 050h SET r1, 0
    0x6401,  // 052h BEQ r1, 0  (true)
    0x6501,  // 054h BNEQ r1, 0 (false, but chained)
    0x62F6,  // 056h SET r6, 0xCAFE (not should happen)
    0xCAFE,  // 058h literal
    0,
};

void print_regs(const CPU::CpuState& state);
void print_cspc(const CPU::CpuState& state, const CPU::byte_t* ram);

int main()
{
    using namespace CPU;
    RC1600 cpu;
    cpu.reset();
    
    std::copy_n((byte_t*)prg, prg_size, cpu.ram);

    print_regs(cpu.getState());
    
    int c = std::getchar();
    while (c != 'q' && c != 'Q') {
        print_cspc(cpu.getState(), cpu.ram);
        if (cpu.getState().skiping)
            std::printf("Skiping!\n");
        if (cpu.getState().sleeping)
            std::printf("ZZZZzzzz...\n");
        
        cpu.step();
        
        std::printf("Takes %u cycles\n", cpu.getState().wait_cycles);
        print_regs(cpu.getState());
        std::cout << std::endl;
        c = std::getchar();
    }

    return 0;
}


void print_regs(const CPU::CpuState& state)
{
    // Print registers
    for (int i=0; i < 14; i++) {
        std::printf("r%2d= 0x%04x ", i, state.r[i]);
        if (i == 5 || i == 11 || i == 13)
            std::printf("\n");
    }

    std::printf("\tSS:SP= %01X:%04Xh ", state.ss, state.r[SP]);
    std::printf("BP= 0x%04x ", state.r[BP]);
    std::printf("\tDS= 0x%04x\n", state.ds);
    std::printf("\tCS:PC= %01X:%04Xh ", state.cs, state.pc);
    std::printf("IS:IA= %01X:%04Xh \n", state.is, state.ia);
    std::printf("FLAGS= 0x%04x \n", state.flags);
    std::printf("TDE: %d TOE: %d TSS: %d \t IF: %d DE %d OF: %d CF: %d\n",
            GET_TDE(state.flags), GET_TOE(state.flags), GET_TSS(state.flags),
            GET_IF(state.flags), GET_DE(state.flags), GET_OF(state.flags),
            GET_CF(state.flags));

}

void print_cspc(const CPU::CpuState& state, const CPU::byte_t* ram)
{
    CPU::dword_t epc = ((state.cs&0x0F) << 16) | state.pc;
    std::printf("\t[CS:PC]= 0x%02x%02x \n", ram[epc+1], ram[epc]); // Big Endian
}
