#include "types.hpp"
#include "vcomputer.hpp"
#include "dis_rc3200.hpp"

#include "CDA.hpp"

#include <iostream>
#include <fstream>
#include <ios>
#include <iomanip> 
#include <cstdio>
#include <algorithm>
#include <memory>
#include <string>
#include <cwctype>
#include <clocale>

#include <chrono>


void print_regs(const vm::cpu::CpuState& state);
void print_pc(const vm::cpu::CpuState& state, const vm::ram::Mem& ram);
void print_stack(const vm::cpu::CpuState& state, const vm::ram::Mem& ram);

int main(int argc, char* argv[])
{
    using namespace vm;
    using namespace vm::cpu;

    byte_t* rom = NULL;
    size_t rom_size = 0;

    if (argc < 2) {
        std::printf("Usage: %s binary_file\n", argv[0]);
        return -1;

    } else {
        rom = new byte_t[64*1024];

        std::printf("Opening file %s\n", argv[1]);
        std::fstream f(argv[1], std::ios::in | std::ios::binary);
        unsigned count = 0;
#if (BYTE_ORDER != LITTLE_ENDIAN)
        while (f.good() && count < 64*1024) {
            f.read(reinterpret_cast<char*>(rom + count++), 1); // Read byte at byte, so the file must be in Little Endian
        }
#else
        size_t size;
        auto begin = f.tellg();
        f.seekg (0, std::ios::end);
        auto end = f.tellg();
        f.seekg (0, std::ios::beg);

        size = end - begin;
        size = size > (64*1024) ? (64*1024) : size;
        
        f.read(reinterpret_cast<char*>(rom), size);
        count = size;
#endif
        std::printf("Read %u bytes and stored in ROM\n", count);
        rom_size = count;
    }
  
    VirtualComputer vm;
    vm.WriteROM(rom, rom_size);
    delete[] rom;

    // Add devices
    cda::CDA gcard;
    vm.AddDevice(0, gcard);

    vm.Reset();
    
    std::printf("Size of CPU state : %zu bytes \n", sizeof(vm.CPUState()) );
    
  std::cout << "Run program (r) or Step Mode (s) ?\n";
    char mode;
    std::cin >> mode;
    std::getchar();


    bool debug = false;
    if (mode == 's' || mode == 'S') {
        debug = true;
    }
   

    std::cout << "Running!\n";
    unsigned ticks = 2000;
    unsigned long ticks_count = 0;

    using namespace std::chrono;
    auto clock = high_resolution_clock::now();
    double delta;

    int c = ' ';
    while ( 1) {
        
        if (debug) {
            print_pc(vm.CPUState(), vm.RAM());
            if (vm.CPUState().skiping)
                std::printf("Skiping!\n");
            if (vm.CPUState().sleeping)
                std::printf("ZZZZzzzz...\n");
        }

        if (!debug) {
            // cpu.Tick(ticks);
            vm.Tick(ticks);
            ticks_count += ticks;
            

            auto oldClock = clock;
            clock = high_resolution_clock::now();
            if (ticks <= 0) // Compensates if is too quick
                delta += duration_cast<nanoseconds>(clock - oldClock).count();
            else
                delta = duration_cast<nanoseconds>(clock - oldClock).count();

            ticks = delta/100.0f + 0.5f; // Rounding bug correction
        } else
            ticks = vm.Step(); // cpu.Step();


        // Speed info
        if (!debug && ticks_count > 5000000) {
            std::cout << "Running " << ticks << " cycles in " << delta << " nS"
                      << " Speed of " 
                      << 100.0f * (((ticks * 1000000000.0) / vm.Clock()) / delta)
                      << "% \n";
            std::cout << std::endl;
            ticks_count -= 5000000;
        }


        if (debug) {
            std::printf("Takes %u cycles\n", vm.CPUState().wait_cycles);
            print_regs(vm.CPUState());
            print_stack(vm.CPUState(), vm.RAM());
            c = std::getchar();
            if (c == 'q' || c == 'Q')
                break;

        }

    }

    return 0;
}


void print_regs(const vm::cpu::CpuState& state)
{
    // Print registers
    for (int i=0; i < 27; i++) {
        std::printf("%%r%2d= 0x%08x ", i, state.r[i]);
        if (i == 3 || i == 7 || i == 11 || i == 15 || i == 19 || i == 23 || i == 27 || i == 31)
            std::printf("\n");
    }
    std::printf("%%y= 0x%08x\n", Y);
    
    std::printf("%%ia= 0x%08x ", IA);
    std::printf("%%flags= 0x%08x ", FLAGS);
    std::printf("%%bp= 0x%08x ", state.r[BP]);
    std::printf("%%sp= 0x%08x\n", state.r[SP]);

    std::printf("%%pc= 0x%08x \n", state.pc);
    std::printf("EDE: %d EOE: %d ESS: %d EI: %d \t IF: %d DE %d OF: %d CF: %d\n",
            GET_EDE(FLAGS), GET_EOE(FLAGS), GET_ESS(FLAGS), GET_EI(FLAGS),
            GET_IF(FLAGS) , GET_DE(FLAGS) , GET_OF(FLAGS) , GET_CF(FLAGS));
    std::printf("\n");

}

void print_pc(const vm::cpu::CpuState& state, const vm::ram::Mem&  ram)
{
    vm::dword_t val = ram.RD(state.pc);
    
    std::printf("\tPC : 0x%08X > 0x%08X ", state.pc, val); 
    std::cout << vm::cpu::Disassembly(ram,  state.pc) << std::endl;  
}

void print_stack(const vm::cpu::CpuState& state, const vm::ram::Mem& ram)
{
    std::printf("STACK:\n");

    for (size_t i =0; i <5*4; i +=4) {
        auto val = ram.RD(state.r[SP]+ i);

        std::printf("0x%08X\n", val);
        if (((size_t)(state.r[SP]) + i) >= 0xFFFFFFFF)
            break;
    }
}

