#include "cpu.hpp"
#include "dis_rc1600.hpp"

#include <iostream>
#include <ios>
#include <iomanip> 
#include <cstdio>
#include <algorithm>
#include <string>
#include <cwctype>
#include <clocale>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#define FRAMERATE (60)

size_t prg_size = 68*2;
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
    0x62F2,  // 070h SET r2, 80*2
    0x00A0,  // 072h
    0x4111,  // 074h ADD r1, 1 
    0xC201,  // 076h STORE [r2], r1  (type B)
    0x62F2,  // 078h SET r2, 80*4
    0x0140,  // 07Ah
    0x4111,  // 07Ch ADD r1, 1 
    0xC201,  // 07Eh STORE [r2], r1  (type C)
    0x2150,  // 080h SETDS 0
    0xC001,  // 082h STORE [0], r1
    0x8001,  // 084h LOAD [0], r1 (shoud be 0x6210)
    0x0000,
    0x0000,
    0x0000,
};

size_t isr_size = 2*2;
CPU::word_t isr[] = {
	0x0000,  // 000h NOP
    0x0004,  // 002h RFI
};

void print_regs(const CPU::CpuState& state);
void print_cspc(const CPU::CpuState& state, const CPU::Mem& ram);
void print_stack(const CPU::CpuState& state, const CPU::Mem& ram);

int main()
{
    using namespace CPU;
    RC1600 cpu;

    cpu.reset();
	
	auto prg_blq = cpu.ram.addBlock(0, 0x8000, true); // ROM
	cpu.ram.addBlock(0x8000, 0x8000); // Free ram
	auto isr_blq = cpu.ram.addBlock(0x10000, 0x10000);
	
    auto mda_blq = cpu.ram.addBlock(0xB0000, 0x10000);
   
    std::cout << "Allocted memory: " << cpu.ram.allocateBlocks()/1024 
              << "KiB" << std::endl;

    {
        auto prg_sptr = prg_blq.lock();
        auto isr_sptr = isr_blq.lock();

        std::copy_n((byte_t*)prg, prg_size, prg_sptr->getPtr()); // Copy program
        std::copy_n((byte_t*)isr, isr_size, isr_sptr->getPtr()); // Copy ISR
    }
    
	std::cout << "Run program (r) or Step Mode (s) ?\n";
    char mode;
    std::cin >> mode;

    sf::RenderWindow mda_window;
    mda_window.create(sf::VideoMode(640, 406), "RC1600 prototype");
    mda_window.setFramerateLimit(FRAMERATE);

    bool debug = false;
    if (mode == 's' || mode == 'S') {
        debug = true;
    }
   
    sf::Font font;
    if (!font.loadFromFile("./assets/font/cour.ttf")) {
        std::cerr << "Error loading font\n";
        exit(0);
    }
    sf::Text text;
    text.setFont(font);
    text.setColor(sf::Color::Green);
    text.setCharacterSize(14); // 14 pixels

    std::cout << "Running!\n";
    sf::Clock clock; 
    unsigned ticks;
    unsigned long ticks_count = 0; 
    mda_window.clear(sf::Color::Black);
    mda_window.display();

    int c = ' ';
    while ( mda_window.isOpen()) {

        // T period of a 1MHz signal = 1 microseconds
        const auto delta=clock.getElapsedTime().asMicroseconds(); 
        clock.restart();
        double tmp = cpu.getClock()/ (double)(FRAMERATE);
        ticks= tmp+0.5;
        
        if (debug) {
            print_cspc(cpu.getState(), cpu.ram);
            if (cpu.getState().skiping)
                std::printf("Skiping!\n");
            if (cpu.getState().sleeping)
                std::printf("ZZZZzzzz...\n");
        }

        if (!debug)
            cpu.tick(ticks);
        else
            ticks = cpu.step();

        ticks_count += ticks;

        // Print to the stdout a 80x25 MDA like screen at 80000h
        auto mda_ram = mda_blq.lock();
        if (mda_ram) {
            std::wstring txt = L"";
            unsigned col = 0, row = 0;
            dword_t i = 0;
            for(row =0; row < 25 && i < mda_ram->size(); row++) {
                for(col =0; col < 80 && i < mda_ram->size(); col++) {
                    wchar_t c = (wchar_t)(mda_ram->getPtr()[i]);
                    i +=2;
                    if (std::iswgraph(c))
                        txt.push_back(c);
                    else
                        txt.push_back(L' ');
                }
                txt.push_back(L'\n');
            }
            text.setString(txt);
        }

        // Speed info
        if (ticks_count > 100000 && !debug) {
            std::cout << "Running " << ticks << " cycles in " << delta << " uS"
                      << " Speed of " 
                      << 100 * (((ticks * 1000000.0) / cpu.getClock()) / delta)
                      << "% \n";
            ticks_count -= 100000;
        }

        // Poll events
        sf::Event ev;
        while (mda_window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                mda_window.close();
                break;
            }
        }


        mda_window.clear(sf::Color(0,0,0, 0xAA));
        // Update here
        mda_window.draw(text);
        mda_window.display();

        if (debug) {
            std::printf("Takes %u cycles\n", cpu.getState().wait_cycles);
            print_regs(cpu.getState());
            print_stack(cpu.getState(), cpu.ram);
            c = std::getchar();
            if (c == 'q' || c == 'Q')
                mda_window.close();
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

void print_cspc(const CPU::CpuState& state, const CPU::Mem&  ram)
{
    CPU::dword_t epc = ((state.cs&0x0F) << 16) | state.pc;
    std::printf("\t[CS:PC]= 0x%02x%02x ", ram.rb(epc+1), ram.rb(epc)); // Big Endian
    std::cout << CPU::disassembly(ram,  epc) << std::endl;  
}

void print_stack(const CPU::CpuState& state, const CPU::Mem& ram)
{
    std::printf("STACK:\n");

    CPU::dword_t ptr = ((state.ss&0x0F) << 16) | state.r[SP];
    for (size_t i =0; i <6; i++) {
        std::printf("%02Xh ", ram.rb(ptr));
        ptr++;
        if (((size_t)(state.r[SP]) + i) >= 0xFFFF)
            break;
    }
    std::printf("\n");
}

