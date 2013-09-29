#include "cpu.hpp"

#include <iostream>
#include <ios>
#include <iomanip> 
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <unistd.h>

size_t prg_size = 64*2;
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
    0x2042,  // 05Ah PUSH r2  (SP = FFFE and [FFFF] = FF44) 
    0x205B,  // 05Ch POP r11 (SP = 0 and r11 = FF44)
    0x21B1,  // 05Eh SETIS 1 (Interrupts in segment 1)
    0x20F2,  // 060h SETIA 2
    0x20D6,  // 062h INT 6
    0x215B,  // 064h SETDS 0xB
    0x62F1,  // 068h SET r1, 'A'
    0x0041,  // 06Ah
    0x6202,  // 06Ch SET r2, 0
    0xC201,  // 06Eh STORE [r2], r1  (type A)
    0x4122,  // 070h ADD r2, 2 
    0x4111,  // 072h ADD r1, 1 
    0xC201,  // 074h STORE [r2], r1  (type B)
    0x62F2,  // 076h SET r2, 0xA0
    0x00B0,  // 078h
    0x4111,  // 07Ah ADD r1, 1 
    0xC201,  // 07Ch STORE [r2], r1  (type C)
};

size_t isr_size = 1*2;
CPU::word_t isr[] = {
    0x0004,  // 002h RFI
};

void print_regs(const CPU::CpuState& state);
void print_cspc(const CPU::CpuState& state, const CPU::byte_t* ram);
void print_stack(const CPU::CpuState& state, const CPU::byte_t* ram);

int main()
{
    using namespace CPU;
    RC1600 cpu;
    cpu.reset();
    
    std::copy_n((byte_t*)prg, prg_size, cpu.ram); // Copy program
    std::copy_n((byte_t*)isr, isr_size, &(cpu.ram[0x10002])); // Copy ISR

    std::cout << "Run program (r) or Step Mode (s) ?\n";
    char mode;
    std::cin >> mode;

    if (mode == 's' || mode == 'S') {

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
            print_stack(cpu.getState(), cpu.ram);

            c = std::getchar();
        }

    } else {
        std::cout << "Running!\n";
        unsigned ticks = 40000;
        unsigned long ticks_count = 0; 
        auto start = std::chrono::system_clock::now();
        cpu.tick(ticks);
        ticks_count += ticks;
        auto end = std::chrono::system_clock::now();

        int64_t delta=std::chrono::duration_cast<std::chrono::microseconds>
                             (end-start).count();
        int64_t ttime = ticks*(1000000 / cpu.getClock());
        std::cout << "Delta   us: "<< delta << std::endl;
        std::cout << "T. Time us: "<< ttime << std::endl;
        std::cout << "Diff : "<< ttime - delta << std::endl;
     
        while (1) {
            end = std::chrono::system_clock::now();
            delta=std::chrono::duration_cast<std::chrono::microseconds>
                             (end-start).count();
            ttime = ticks*(1000000 / cpu.getClock());
            /*if (ticks_count > 1000) {
                std::cout << "Delta   us: "<< delta << std::endl;
                std::cout << "T. Time us: "<< ttime << std::endl;
                std::cout << "Diff : "<< ttime - delta << std::endl;
                ticks_count -= 1000;
            }*/
            cpu.tick(ticks);
            ticks_count += ticks;

            // Print to the stdout a 80x25 MDA like screen at 80000h
            unsigned col = 0;
            //std::printf("\033[2J\033[;H"); // Clear screen
            std::printf("\033[82T\033[;H"); // Clear screen
            std::printf(
"*******************************************************************************\n");
            for(size_t i= 0xB0000; i< 0xB0FA0; i+=2) {
                std::printf("%c", cpu.ram[i]);
                if (col == 79) {
                    col = 0;
                    std::printf("\n");
                } else {
                    col++;
                }
            }
            std::printf(
"*******************************************************************************\n");

            if (ttime > delta)
                usleep(ttime-delta );
            start = end;
        }

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
    std::printf("\n");

}

void print_cspc(const CPU::CpuState& state, const CPU::byte_t* ram)
{
    CPU::dword_t epc = ((state.cs&0x0F) << 16) | state.pc;
    std::printf("\t[CS:PC]= 0x%02x%02x \n", ram[epc+1], ram[epc]); // Big Endian
}

void print_stack(const CPU::CpuState& state, const CPU::byte_t* ram)
{
    std::printf("STACK:\n");

    CPU::dword_t ptr = ((state.ss&0x0F) << 16) | state.r[SP];
    for (size_t i =0; i <6; i++) {
        std::printf("%02Xh ", ram[ptr]);
        ptr++;
        if (((size_t)(state.r[SP]) + i) >= 0xFFFF)
            break;
    }
    std::printf("\n");
}

