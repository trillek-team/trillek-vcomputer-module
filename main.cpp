#include "cpu.hpp"

#include <iostream>
#include <ios>
#include <iomanip> 
#include <cstdio>
#include <algorithm>

size_t prg_size = 24*2;
CPU::word_t prg[] = {
    0x6210,  // 000h SET 1, r0
    0x6211,  // 002h SET 1, r1
    0x6222,  // 004h SET 2, r2 ..
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
    0x62EE,  // 01Ch SET 14, BP
    0x62FF,  // 01Eh SET 15, SP
    0x000F,  // 020h literal
    0x620F,  // 022h SET 0 , SP
    0x2001,  // 024h NOT r1
    0x2012,  // 026h NEG r2
    0x2023,  // 028h XCHG r3
    0x62F4,  // 02Ah SET 0x00FF, r4
    0x00FF,  // 02Ch literal
    0x2034,  // 02Eh SXTBD r4 (r4= 0xFFFF)
    0
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
        cpu.step();
        std::printf("Takes %u cycles\n", cpu.getState().wait_cycles);
        print_regs(cpu.getState());
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

}

void print_cspc(const CPU::CpuState& state, const CPU::byte_t* ram)
{
    CPU::dword_t epc = ((state.cs&0x0F) << 16) | state.pc;
    std::printf("\t[CS:PC]= 0x%02x%02x ", ram[epc+1], ram[epc]); // Big Endian
}
