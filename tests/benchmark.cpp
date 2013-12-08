#include <vm.hpp>

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

#define N_CPUS 1000
    
    VirtualComputer vm[N_CPUS];
    for (auto i=0; i< N_CPUS; i++)
        vm[i].WriteROM(rom, rom_size);

    delete[] rom;

    // Add devices
    cda::CDA gcard[N_CPUS];
    
    for (auto i=0; i< N_CPUS; i++) {
        vm[i].AddDevice(0, gcard[i]);
        vm[i].Reset();
    }
    
    std::cout << "Running " << N_CPUS << " CPUs !\n";
    unsigned ticks = 2000;
    unsigned long ticks_count = 0;

    using namespace std::chrono;
    auto clock = high_resolution_clock::now();
    double delta;

    while ( 1) {
        
        for (auto i=0; i< N_CPUS; i++)
            vm[i].Tick(ticks);
        
        ticks_count += ticks;

        auto oldClock = clock;
        clock = high_resolution_clock::now();
        if (ticks <= 0) // Compensates if is too quick
            delta += duration_cast<nanoseconds>(clock - oldClock).count();
        else
            delta = duration_cast<nanoseconds>(clock - oldClock).count();

        //ticks = delta/100.0f + 0.5f; // Rounding bug correction


        // Speed info
        if (ticks_count > 500000) {
            std::cout << "Running " << ticks << " cycles in " << delta << " nS"
                      << " Speed of " 
                      << 100.0f * (((ticks * 1000000000.0) / vm[0].Clock()) / delta)
                      << "% \n";
            std::cout << std::endl;
            ticks_count -= 500000;
        }

    }

    return 0;
}

